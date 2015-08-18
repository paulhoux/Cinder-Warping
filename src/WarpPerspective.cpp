/*
 Copyright (c) 2010-2013, Paul Houx - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 This file is part of Cinder-Warping.

 Cinder-Warping is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Cinder-Warping is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Cinder-Warping.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "WarpPerspective.h"

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;

namespace ph {
namespace warping {

WarpPerspective::WarpPerspective( void )
	: Warp( PERSPECTIVE )
{
	//
	mSource[0].x = 0.0f;
	mSource[0].y = 0.0f;
	mSource[1].x = (float) mWidth;
	mSource[1].y = 0.0f;
	mSource[2].x = (float) mWidth;
	mSource[2].y = (float) mHeight;
	mSource[3].x = 0.0f;
	mSource[3].y = (float) mHeight;

	//
	reset();
}

WarpPerspective::~WarpPerspective( void )
{
}

mat4 WarpPerspective::getTransform()
{
	// calculate warp matrix using OpenCV
	if( mIsDirty ) {
		// update source size
		mSource[1].x = (float) mWidth;
		mSource[2].x = (float) mWidth;
		mSource[2].y = (float) mHeight;
		mSource[3].y = (float) mHeight;

		// convert corners to actual destination pixels
		mDestination[0].x = mPoints[0].x * mWindowSize.x;
		mDestination[0].y = mPoints[0].y * mWindowSize.y;
		mDestination[1].x = mPoints[1].x * mWindowSize.x;
		mDestination[1].y = mPoints[1].y * mWindowSize.y;
		mDestination[2].x = mPoints[2].x * mWindowSize.x;
		mDestination[2].y = mPoints[2].y * mWindowSize.y;
		mDestination[3].x = mPoints[3].x * mWindowSize.x;
		mDestination[3].y = mPoints[3].y * mWindowSize.y;

		// calculate warp matrix
		cv::Mat	warp = cv::getPerspectiveTransform( mSource, mDestination );

		// convert to OpenGL matrix
		mTransform[0][0] = (float) warp.ptr<double>( 0 )[0];
		mTransform[0][1] = (float) warp.ptr<double>( 1 )[0];
		mTransform[0][3] = (float) warp.ptr<double>( 2 )[0];

		mTransform[1][0] = (float) warp.ptr<double>( 0 )[1];
		mTransform[1][1] = (float) warp.ptr<double>( 1 )[1];
		mTransform[1][3] = (float) warp.ptr<double>( 2 )[1];

		mTransform[3][0] = (float) warp.ptr<double>( 0 )[2];
		mTransform[3][1] = (float) warp.ptr<double>( 1 )[2];
		mTransform[3][3] = (float) warp.ptr<double>( 2 )[2];

		// update the inverted matrix
		getInvertedTransform();

		mIsDirty = false;
	}

	return mTransform;
}

mat4	WarpPerspective::getInvertedTransform()
{
	if( mIsDirty )
		mInverted = glm::inverse( mTransform );

	return mInverted;
}

void WarpPerspective::reset()
{
	mPoints.clear();
	mPoints.push_back( vec2( 0.0f, 0.0f ) );
	mPoints.push_back( vec2( 1.0f, 0.0f ) );
	mPoints.push_back( vec2( 1.0f, 1.0f ) );
	mPoints.push_back( vec2( 0.0f, 1.0f ) );

	mIsDirty = true;
}

void WarpPerspective::draw( const gl::Texture2dRef &texture, const Area &srcArea, const Rectf &destRect )
{
	// clip against bounds
	Area	area = srcArea;
	Rectf	rect = destRect;
	clip( area, rect );

	// save current drawing color
	const ColorA &currentColor = gl::context()->getCurrentColor();
	gl::ScopedColor color( currentColor );

	// adjust brightness
	if( mBrightness < 1.f ) {
		ColorA drawColor = mBrightness * currentColor;
		drawColor.a = currentColor.a;

		gl::color( drawColor );
	}

	// draw texture
	gl::pushModelMatrix();
	gl::multModelMatrix( getTransform() );

	gl::draw( texture, area, rect );

	gl::popModelMatrix();

	// draw interface
	draw();
}

void WarpPerspective::begin()
{
	gl::pushModelMatrix();
	gl::multModelMatrix( getTransform() );
}

void WarpPerspective::end()
{
	// restore warp
	gl::popModelMatrix();

	// draw interface
	draw();
}

void WarpPerspective::draw( bool controls )
{
	// only draw grid while editing
	if( isEditModeEnabled() ) {
		gl::pushModelMatrix();
		gl::multModelMatrix( getTransform() );

		glLineWidth( 1.0f );
		glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

		gl::ScopedColor color( Color::white() );
		for( int i = 0; i <= 1; i++ ) {
			float s = i / 1.0f;
			gl::drawLine( vec2( s * (float) mWidth, 0.0f ), vec2( s * (float) mWidth, (float) mHeight ) );
			gl::drawLine( vec2( 0.0f, s * (float) mHeight ), vec2( (float) mWidth, s * (float) mHeight ) );
		}

		gl::drawLine( vec2( 0.0f, 0.0f ), vec2( (float) mWidth, (float) mHeight ) );
		gl::drawLine( vec2( (float) mWidth, 0.0f ), vec2( 0.0f, (float) mHeight ) );

		gl::popModelMatrix();

		if( controls ) {
			// draw control points		
			for( int i = 0; i < 4; i++ )
				drawControlPoint( toVec2f( mDestination[i] ), i == mSelected );
		}
	}
}

void WarpPerspective::keyDown( KeyEvent &event )
{
	// let base class handle keys first
	Warp::keyDown( event );
	if( event.isHandled() )
		return;

	// disable keyboard input when not in edit mode
	if( !isEditModeEnabled() ) return;

	// do not listen to key input if not selected
	if( mSelected >= mPoints.size() ) return;

	switch( event.getCode() ) {
	case KeyEvent::KEY_F9:
		// rotate content ccw
		std::swap( mPoints[1], mPoints[2] );
		std::swap( mPoints[0], mPoints[1] );
		std::swap( mPoints[3], mPoints[0] );
		mSelected = ( mSelected + 1 ) % 4;
		mIsDirty = true;
		break;
	case KeyEvent::KEY_F10:
		// rotate content cw
		std::swap( mPoints[3], mPoints[0] );
		std::swap( mPoints[0], mPoints[1] );
		std::swap( mPoints[1], mPoints[2] );
		mSelected = ( mSelected + 3 ) % 4;
		mIsDirty = true;
		break;
	case KeyEvent::KEY_F11:
		// flip content horizontally
		std::swap( mPoints[0], mPoints[1] );
		std::swap( mPoints[2], mPoints[3] );
		if( mSelected % 2 ) mSelected--;
		else mSelected++;
		mIsDirty = true;
		break;
	case KeyEvent::KEY_F12:
		// flip content vertically
		std::swap( mPoints[0], mPoints[3] );
		std::swap( mPoints[1], mPoints[2] );
		mSelected = ( mPoints.size() - 1 ) - mSelected;
		mIsDirty = true;
		break;
	default:
		return;
	}

	event.setHandled( true );
}

}
} // namespace ph::warping