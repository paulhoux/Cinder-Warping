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

#include "WarpPerspectiveBilinear.h"

#include "cinder/Xml.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;

namespace ph { namespace warping {

WarpPerspectiveBilinear::WarpPerspectiveBilinear()
	: WarpBilinear()
{
	// change type 
	mType = PERSPECTIVE_BILINEAR;

	// create perspective warp
	mWarp = WarpPerspectiveRef( new WarpPerspective() );
}

WarpPerspectiveBilinear::~WarpPerspectiveBilinear()
{
}

XmlTree	WarpPerspectiveBilinear::toXml() const
{
	XmlTree xml = WarpBilinear::toXml();

	// set corners
	for(unsigned i=0;i<4;++i) {
		Vec2f corner = mWarp->getControlPoint(i);

		XmlTree cp;
		cp.setTag("corner");
		cp.setAttribute("x", corner.x);
		cp.setAttribute("y", corner.y);

		xml.push_back(cp);
	}

	return xml;
}

void WarpPerspectiveBilinear::fromXml(const XmlTree &xml)
{
	WarpBilinear::fromXml(xml);

	// get corners
	unsigned i = 0;
	for(XmlTree::ConstIter child=xml.begin("corner");child!=xml.end();++child) {
		float x = child->getAttributeValue<float>("x", 0.0f);
		float y = child->getAttributeValue<float>("y", 0.0f);

		mWarp->setControlPoint(i, Vec2f(x, y));

		i++;
	}
}

void WarpPerspectiveBilinear::draw(bool controls)
{
	// apply perspective transform
	gl::pushModelView();
	gl::multModelView( mWarp->getTransform() );

	// draw bilinear warp
	WarpBilinear::draw(false);

	// restore transform
	gl::popModelView();

	// draw edit interface
	if( isEditModeEnabled() ) {
		if(controls) {
			// draw control points
			for(unsigned i=0;i<mPoints.size();++i) 
				drawControlPoint( getControlPoint(i) * mWindowSize, mSelected==i );
		}
	}
}

bool WarpPerspectiveBilinear::mouseMove( MouseEvent event )
{
	bool handled = mWarp->mouseMove( event );
	return Warp::mouseMove( event ) || handled;
}

bool WarpPerspectiveBilinear::mouseDown( MouseEvent event )
{
	if( ! isEditModeEnabled() ) return false;
	if(mSelected >= mPoints.size()) return false;

	// depending on selected control point, let perspective or bilinear warp handle it
	if( isCorner( mSelected ) ) {
		return mWarp->mouseDown( event );
	}
	else {
		return Warp::mouseDown( event );
	}
}

bool WarpPerspectiveBilinear::mouseDrag( MouseEvent event )
{
	if( !isEditModeEnabled() ) return false;
	if(mSelected >= mPoints.size()) return false;

	// depending on selected control point, let perspective or bilinear warp handle it
	if( isCorner( mSelected ) ) {
		return mWarp->mouseDrag( event );
	}
	else {
		return Warp::mouseDrag( event );
	}
}

bool WarpPerspectiveBilinear::keyDown(KeyEvent event)
{
	if( ! isEditModeEnabled() ) return false;
	if(mSelected >= mPoints.size()) return false;

	switch( event.getCode() ) {
		case KeyEvent::KEY_UP:
		case KeyEvent::KEY_DOWN:
		case KeyEvent::KEY_LEFT:
		case KeyEvent::KEY_RIGHT: {
			// make sure cursor keys are handled by 1 warp only
			if(!isCorner( mSelected ) || !mWarp->keyDown(event)) 
				return WarpBilinear::keyDown( event );

			return true;
		}
		break;
		case KeyEvent::KEY_F9:
		case KeyEvent::KEY_F10:
			// let only the Perspective warp handle rotating 
			return mWarp->keyDown(event);
		case KeyEvent::KEY_F11:
		case KeyEvent::KEY_F12:
			// let only the Bilinear warp handle flipping
			return WarpBilinear::keyDown( event );
		default: 
			{
				// let both warps handle the other keyDown events
				bool handled = mWarp->keyDown(event);
				return (WarpBilinear::keyDown( event ) || handled);
			}
		 break;
	}
}

bool WarpPerspectiveBilinear::resize()
{
	// make content size compatible with WarpBilinear's mWindowSize
	mWarp->setSize( getWindowSize() );

	// 
	bool handled = mWarp->resize();
	return (WarpBilinear::resize() || handled);
}

void WarpPerspectiveBilinear::setSize(int w, int h)
{
	// make content size compatible with WarpBilinear's mWindowSize
	mWarp->setSize( mWindowSize );

	WarpBilinear::setSize(w, h);
}

void WarpPerspectiveBilinear::setSize(const Vec2i &size)
{
	// make content size compatible with WarpBilinear's mWindowSize
	mWarp->setSize( mWindowSize );

	WarpBilinear::setSize(size);
}

Vec2f WarpPerspectiveBilinear::getControlPoint(unsigned index) const
{
	// depending on index, return perspective or bilinear control point
	if( isCorner( index ) ) {
		// perspective: simply return one of the corners
		return mWarp->getControlPoint( convertIndex( index ) );
	}
	else {
		// bilinear: transform control point from warped space to normalized screen space
		Vec2f p = Warp::getControlPoint(index) * Vec2f( mWarp->getSize() );
		Vec3f pt = mWarp->getTransform().transformPoint( Vec3f(p.x, p.y, 0.0f) );

		return Vec2f(pt.x, pt.y) / mWindowSize;
	}
}

void WarpPerspectiveBilinear::setControlPoint(unsigned index, const Vec2f &pos)
{
	// depending on index, set perspective or bilinear control point
	if( isCorner( index ) ) {
		// perspective: simply set the control point
		mWarp->setControlPoint( convertIndex( index ), pos );
	}
	else {
		// bilinear:: transform control point from normalized screen space to warped space
		Vec2f p = pos * mWindowSize;
		Vec3f pt = mWarp->getInvertedTransform().transformPoint( Vec3f(p.x, p.y, 0.0f) );

		Vec2f size( mWarp->getSize() );
		Warp::setControlPoint( index, Vec2f(pt.x, pt.y) / size );
	}
}

void WarpPerspectiveBilinear::moveControlPoint(unsigned index, const Vec2f &shift)
{
	// depending on index, move perspective or bilinear control point
	if( isCorner( index ) ) {
		// perspective: simply move the control point
		mWarp->moveControlPoint( convertIndex( index ), shift );
	}
	else {
		// bilinear: transform control point from normalized screen space to warped space
		Vec2f pt = getControlPoint(index);
		setControlPoint(index, pt + shift);
	}
}

void WarpPerspectiveBilinear::selectControlPoint(unsigned index)
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

bool WarpPerspectiveBilinear::isCorner(unsigned index) const
{
	unsigned numControls = (unsigned)(mControlsX * mControlsY);

	return (index==0 || index==(numControls-mControlsY) || index==(numControls-1) || index==(mControlsY-1));
}

unsigned WarpPerspectiveBilinear::convertIndex(unsigned index) const
{	
	unsigned numControls = (unsigned)(mControlsX * mControlsY);

	if(index == 0) return 0;
	else if(index == (numControls-mControlsY)) return 2;
	else if(index == (numControls-1)) return 3;
	else if(index == (mControlsY-1)) return 1;
	else return index;
}

} } // namespace ph::warping



