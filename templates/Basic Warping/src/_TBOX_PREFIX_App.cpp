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

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Rand.h"

#include "Warp.h"

using namespace ci;
using namespace ci::app;
using namespace ph::warping;
using namespace std;

class _TBOX_PREFIX_App : public App {
public:
	static void prepare( Settings *settings );

	void setup() override;
	void cleanup() override;
	void update() override;
	void draw() override;

	void resize() override;

	void mouseMove( MouseEvent event ) override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void mouseUp( MouseEvent event ) override;

	void keyDown( KeyEvent event ) override;
	void keyUp( KeyEvent event ) override;

	void updateWindowTitle();
private:
	bool			mUseBeginEnd;

	fs::path		mSettings;

	gl::TextureRef	mImage;
	WarpList		mWarps;

	Area			mSrcArea;
};

void _TBOX_PREFIX_App::prepare( Settings *settings )
{
	settings->setWindowSize( 1440, 900 );
}

void _TBOX_PREFIX_App::setup()
{
	mUseBeginEnd = false;
	updateWindowTitle();
	disableFrameRate();

	// initialize warps
	mSettings = getAssetPath( "" ) / "warps.xml";
	if( fs::exists( mSettings ) ) {
		// load warp settings from file if one exists
		mWarps = Warp::readSettings( loadFile( mSettings ) );
	}
	else {
		// otherwise create a warp from scratch
		mWarps.push_back( WarpBilinear::create() );
		mWarps.push_back( WarpPerspective::create() );
		mWarps.push_back( WarpPerspectiveBilinear::create() );
	}

	// load test image
	try {
		mImage = gl::Texture::create( loadImage( loadAsset( "help.png" ) ), 
									  gl::Texture2d::Format().loadTopDown().mipmap( true ).minFilter( GL_LINEAR_MIPMAP_LINEAR ) );

		mSrcArea = mImage->getBounds();

		// adjust the content size of the warps
		Warp::setSize( mWarps, mImage->getSize() );
	}
	catch( const std::exception &e ) {
		console() << e.what() << std::endl;
	}
}

void _TBOX_PREFIX_App::cleanup()
{
	// save warp settings
	Warp::writeSettings( mWarps, writeFile( mSettings ) );
}

void _TBOX_PREFIX_App::update()
{
	// there is nothing to update
}

void _TBOX_PREFIX_App::draw()
{
	// clear the window and set the drawing color to white
	gl::clear();
	gl::color( Color::white() );

	if( mImage ) {
		// iterate over the warps and draw their content
		for( auto &warp : mWarps ) {
			// there are two ways you can use the warps:
			if( mUseBeginEnd ) {
				// a) issue your draw commands between begin() and end() statements
				warp->begin();

				// in this demo, we want to draw a specific area of our image,
				// but if you want to draw the whole image, you can simply use: gl::draw( mImage );
				gl::draw( mImage, mSrcArea, warp->getBounds() );

				warp->end();
			}
			else {
				// b) simply draw a texture on them (ideal for video)

				// in this demo, we want to draw a specific area of our image,
				// but if you want to draw the whole image, you can simply use: warp->draw( mImage );
				warp->draw( mImage, mSrcArea );
			}
		}
	}
}

void _TBOX_PREFIX_App::resize()
{
	// tell the warps our window has been resized, so they properly scale up or down
	Warp::handleResize( mWarps );
}

void _TBOX_PREFIX_App::mouseMove( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseMove( mWarps, event ) ) {
		// let your application perform its mouseMove handling here
	}
}

void _TBOX_PREFIX_App::mouseDown( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseDown( mWarps, event ) ) {
		// let your application perform its mouseDown handling here
	}
}

void _TBOX_PREFIX_App::mouseDrag( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseDrag( mWarps, event ) ) {
		// let your application perform its mouseDrag handling here
	}
}

void _TBOX_PREFIX_App::mouseUp( MouseEvent event )
{
	// pass this mouse event to the warp editor first
	if( !Warp::handleMouseUp( mWarps, event ) ) {
		// let your application perform its mouseUp handling here
	}
}

void _TBOX_PREFIX_App::keyDown( KeyEvent event )
{
	// pass this key event to the warp editor first
	if( !Warp::handleKeyDown( mWarps, event ) ) {
		// warp editor did not handle the key, so handle it here
		switch( event.getCode() ) {
			case KeyEvent::KEY_ESCAPE:
				// quit the application
				quit();
				break;
			case KeyEvent::KEY_f:
				// toggle full screen
				setFullScreen( !isFullScreen() );
				break;
			case KeyEvent::KEY_v:
				// toggle vertical sync
				gl::enableVerticalSync( !gl::isVerticalSyncEnabled() );
				break;
			case KeyEvent::KEY_w:
				// toggle warp edit mode
				Warp::enableEditMode( !Warp::isEditModeEnabled() );
				break;
			case KeyEvent::KEY_a:
				// toggle drawing a random region of the image
				if( mSrcArea.getWidth() != mImage->getWidth() || mSrcArea.getHeight() != mImage->getHeight() )
					mSrcArea = mImage->getBounds();
				else {
					int x1 = Rand::randInt( 0, mImage->getWidth() - 150 );
					int y1 = Rand::randInt( 0, mImage->getHeight() - 150 );
					int x2 = Rand::randInt( x1 + 150, mImage->getWidth() );
					int y2 = Rand::randInt( y1 + 150, mImage->getHeight() );
					mSrcArea = Area( x1, y1, x2, y2 );
				}
				break;
			case KeyEvent::KEY_SPACE:
				// toggle drawing mode
				mUseBeginEnd = !mUseBeginEnd;
				updateWindowTitle();
				break;
		}
	}
}

void _TBOX_PREFIX_App::keyUp( KeyEvent event )
{
	// pass this key event to the warp editor first
	if( !Warp::handleKeyUp( mWarps, event ) ) {
		// let your application perform its keyUp handling here
	}
}

void _TBOX_PREFIX_App::updateWindowTitle()
{
	if( mUseBeginEnd )
		getWindow()->setTitle( "Warping Sample - Using begin() and end()" );
	else
		getWindow()->setTitle( "Warping Sample - Using draw()" );
}

CINDER_APP( _TBOX_PREFIX_App, RendererGl( RendererGl::Options().msaa( 8 ) ), &_TBOX_PREFIX_App::prepare )
