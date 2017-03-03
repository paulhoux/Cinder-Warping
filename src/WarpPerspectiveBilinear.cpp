/*
 Copyright (c) 2010-2015, Paul Houx - All rights reserved.
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

#include "Warp.h"

#include "cinder/Xml.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;

namespace ph {
namespace warping {

WarpPerspectiveBilinear::WarpPerspectiveBilinear( const ci::gl::Fbo::Format &format )
    : WarpBilinear( format )
{
	// change type
	mType = PERSPECTIVE_BILINEAR;

	// create perspective warp
	mWarp = WarpPerspectiveRef( new WarpPerspective() );
}

XmlTree WarpPerspectiveBilinear::toXml() const
{
	XmlTree xml = WarpBilinear::toXml();

	// set corners
	for( unsigned i = 0; i < 4; ++i ) {
		vec2 corner = mWarp->getControlPoint( i );

		XmlTree cp;
		cp.setTag( "corner" );
		cp.setAttribute( "x", corner.x );
		cp.setAttribute( "y", corner.y );

		xml.push_back( cp );
	}

	return xml;
}

void WarpPerspectiveBilinear::fromXml( const XmlTree &xml )
{
	WarpBilinear::fromXml( xml );

	// get corners
	unsigned i = 0;
	for( XmlTree::ConstIter child = xml.begin( "corner" ); child != xml.end(); ++child ) {
		float x = child->getAttributeValue<float>( "x", 0.0f );
		float y = child->getAttributeValue<float>( "y", 0.0f );

		mWarp->setControlPoint( i, vec2( x, y ) );

		i++;
	}
}

void WarpPerspectiveBilinear::draw( bool controls )
{
	// apply perspective transform
	gl::pushModelMatrix();
	gl::multModelMatrix( mWarp->getTransform() );

	// draw bilinear warp
	WarpBilinear::draw( false );

	// restore transform
	gl::popModelMatrix();

	// draw edit interface
	if( isEditModeEnabled() ) {
		if( controls && mSelected < mPoints.size() ) {
			// draw control points
			for( unsigned i = 0; i < mPoints.size(); ++i )
				queueControlPoint( getControlPoint( i ) * mWindowSize, mSelected == i );

			drawControlPoints();
		}
	}
}

void WarpPerspectiveBilinear::mouseMove( MouseEvent &event )
{
	mWarp->mouseMove( event );
	Warp::mouseMove( event );
}

void WarpPerspectiveBilinear::mouseDown( MouseEvent &event )
{
	if( !isEditModeEnabled() )
		return;
	if( mSelected >= mPoints.size() )
		return;

	// depending on selected control point, let perspective or bilinear warp handle it
	if( isCorner( mSelected ) ) {
		mWarp->mouseDown( event );
	}
	else {
		Warp::mouseDown( event );
	}
}

void WarpPerspectiveBilinear::mouseDrag( MouseEvent &event )
{
	if( !isEditModeEnabled() )
		return;
	if( mSelected >= mPoints.size() )
		return;

	// depending on selected control point, let perspective or bilinear warp handle it
	if( isCorner( mSelected ) ) {
		mWarp->mouseDrag( event );
	}
	else {
		Warp::mouseDrag( event );
	}
}

void WarpPerspectiveBilinear::keyDown( KeyEvent &event )
{
	if( !isEditModeEnabled() )
		return;
	if( mSelected >= mPoints.size() )
		return;

	switch( event.getCode() ) {
	case KeyEvent::KEY_UP:
	case KeyEvent::KEY_DOWN:
	case KeyEvent::KEY_LEFT:
	case KeyEvent::KEY_RIGHT:
		// make sure cursor keys are handled by 1 warp only
		if( !isCorner( mSelected ) )
			mWarp->keyDown( event );
		if( !event.isHandled() )
			WarpBilinear::keyDown( event );
		break;
	case KeyEvent::KEY_F9:
	case KeyEvent::KEY_F10:
		// let only the Perspective warp handle rotating
		mWarp->keyDown( event );
		break;
	case KeyEvent::KEY_F11:
	case KeyEvent::KEY_F12:
		// let only the Bilinear warp handle flipping
		WarpBilinear::keyDown( event );
		break;
	default:
		// let both warps handle the other keyDown events
		mWarp->keyDown( event );
		WarpBilinear::keyDown( event );
		break;
	}
}

void WarpPerspectiveBilinear::resize()
{
	// make content size compatible with WarpBilinear's mWindowSize
	mWarp->setSize( getWindowSize() );

	//
	mWarp->resize();
	WarpBilinear::resize();
}

void WarpPerspectiveBilinear::setSize( int w, int h )
{
	// make content size compatible with WarpBilinear's mWindowSize
	mWarp->setSize( mWindowSize );

	WarpBilinear::setSize( w, h );
}

vec2 WarpPerspectiveBilinear::getControlPoint( unsigned index ) const
{
	// depending on index, return perspective or bilinear control point
	if( isCorner( index ) ) {
		// perspective: simply return one of the corners
		return mWarp->getControlPoint( convertIndex( index ) );
	}
	else {
		// bilinear: transform control point from warped space to normalized screen space
		vec2 p = Warp::getControlPoint( index ) * vec2( mWarp->getSize() );
		vec4 pt = mWarp->getTransform() * vec4( p.x, p.y, 0, 1 );

		if( pt.w != 0 )
			pt.w = 1 / pt.w;
		pt *= pt.w;

		return vec2( pt.x, pt.y ) / mWindowSize;
	}
}

void WarpPerspectiveBilinear::setControlPoint( unsigned index, const vec2 &pos )
{
	// depending on index, set perspective or bilinear control point
	if( isCorner( index ) ) {
		// perspective: simply set the control point
		mWarp->setControlPoint( convertIndex( index ), pos );
	}
	else {
		// bilinear:: transform control point from normalized screen space to warped space
		vec2 p = pos * mWindowSize;
		vec4 pt = mWarp->getInvertedTransform() * vec4( p.x, p.y, 0, 1 );

		if( pt.w != 0 )
			pt.w = 1 / pt.w;
		pt *= pt.w;

		vec2 size( mWarp->getSize() );
		Warp::setControlPoint( index, vec2( pt.x, pt.y ) / size );
	}
}

void WarpPerspectiveBilinear::moveControlPoint( unsigned index, const vec2 &shift )
{
	// depending on index, move perspective or bilinear control point
	if( isCorner( index ) ) {
		// perspective: simply move the control point
		mWarp->moveControlPoint( convertIndex( index ), shift );
	}
	else {
		// bilinear: transform control point from normalized screen space to warped space
		vec2 pt = getControlPoint( index );
		setControlPoint( index, pt + shift );
	}
}

void WarpPerspectiveBilinear::selectControlPoint( unsigned index )
{
	// depending on index, select perspective or bilinear control point
	if( isCorner( index ) ) {
		mWarp->selectControlPoint( convertIndex( index ) );
	}
	else {
		mWarp->deselectControlPoint();
	}

	// always select bilinear control point, which we use to keep track of editing
	Warp::selectControlPoint( index );
}

void WarpPerspectiveBilinear::deselectControlPoint()
{
	mWarp->deselectControlPoint();
	Warp::deselectControlPoint();
}

bool WarpPerspectiveBilinear::isCorner( unsigned index ) const
{
	unsigned numControls = (unsigned)( mControlsX * mControlsY );

	return ( index == 0 || index == ( numControls - mControlsY ) || index == ( numControls - 1 ) || index == ( mControlsY - 1 ) );
}

unsigned WarpPerspectiveBilinear::convertIndex( unsigned index ) const
{
	unsigned numControls = (unsigned)( mControlsX * mControlsY );

	if( index == 0 )
		return 0;
	else if( index == ( numControls - mControlsY ) )
		return 2;
	else if( index == ( numControls - 1 ) )
		return 3;
	else if( index == ( mControlsY - 1 ) )
		return 1;
	else
		return index;
}
}
} // namespace ph::warping
