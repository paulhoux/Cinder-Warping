/*
 Copyright (c) 2010-2013, Paul Houx - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "Warp.h"
#include "WarpBilinear.h"
#include "WarpPerspective.h"
#include "WarpPerspectiveBilinear.h"

#include "cinder/Xml.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;

namespace ph { namespace warping {

bool	Warp::sIsEditMode = false;
double	Warp::sSelectedTime = 0.0;
Vec2i	Warp::sMouse = Vec2i(0, 0);

Warp::Warp(WarpType type)
	:mType(type)
{
	// 
	mWidth = 640; 
	mHeight = 480;

	mWindowSize = Vec2f( float(mWidth), float(mHeight) );

	mBrightness = 1.0f;

	mIsDirty = true;

	mSelected = -1; // since this is an unsigned int, actual value will be 'MAX_INTEGER'
}

Warp::~Warp(void)
{
}

XmlTree	Warp::toXml() const
{
	XmlTree		xml;
	xml.setTag("warp");
	switch( mType )
	{
	case BILINEAR: xml.setAttribute("method", "bilinear"); break;
	case PERSPECTIVE: xml.setAttribute("method", "perspective"); break;
	case PERSPECTIVE_BILINEAR: xml.setAttribute("method", "perspectivebilinear"); break;
	default: xml.setAttribute("method", "unknown"); break;
	}
	xml.setAttribute("width", mControlsX);
	xml.setAttribute("height", mControlsY);
	xml.setAttribute("brightness", mBrightness);

	// add <controlpoint> tags (column-major)
	std::vector<Vec2f>::const_iterator itr;
	for(itr=mPoints.begin();itr!=mPoints.end();++itr) {
		XmlTree	cp;
		cp.setTag("controlpoint");
		cp.setAttribute("x", (*itr).x);
		cp.setAttribute("y", (*itr).y);

		xml.push_back( cp );
	}

	return xml;
}

void Warp::fromXml(const XmlTree &xml)
{
	mControlsX = xml.getAttributeValue<int>("width", 2);
	mControlsY = xml.getAttributeValue<int>("height", 2);
	mBrightness = xml.getAttributeValue<float>("brightness", 1.0f);

	// load control points
	mPoints.clear();
	for(XmlTree::ConstIter child=xml.begin("controlpoint");child!=xml.end();++child) {
		float x = child->getAttributeValue<float>("x", 0.0f);
		float y = child->getAttributeValue<float>("y", 0.0f);
		mPoints.push_back( Vec2f(x, y) );
	}

	// reconstruct warp
	mIsDirty = true;
}

void Warp::setSize(int w, int h)
{
	mWidth = w;
	mHeight = h;

	mIsDirty = true;
}

void Warp::setSize(const Vec2i &size)
{
	mWidth = size.x;
	mHeight = size.y;

	mIsDirty = true;
}

Vec2f Warp::getControlPoint(size_t index) const
{
	if(index < 0 || index >= mPoints.size()) return Vec2f::zero();
	return mPoints[index]; 
}

void Warp::setControlPoint(size_t index, const Vec2f &pos)
{
	if( index < 0 || index >= mPoints.size() ) return;
	mPoints[index] = pos;

	mIsDirty = true;
}

void Warp::moveControlPoint(size_t index, const Vec2f &shift)
{
	if( index < 0 || index >= mPoints.size() ) return;
	mPoints[index] += shift;

	mIsDirty = true;
}

void Warp::selectControlPoint(size_t index)
{
	if( index < 0 || index >= mPoints.size() ) return;
	if( index == mSelected ) return;

	mSelected = index;
	sSelectedTime = app::getElapsedSeconds();
}

void Warp::deselectControlPoint()
{
	mSelected = -1;
}

size_t Warp::findControlPoint(const Vec2f &pos, float *distance) const
{
	size_t index;

	// find closest control point
	float dist = 10.0e6f;

	for(size_t i=0;i<mPoints.size();i++) {
		float d = pos.distance( getControlPoint(i) * mWindowSize );

		if(d < dist){
			dist = d;
			index = i;
		}
	}

	*distance = dist;

	return index;
}

void Warp::selectClosestControlPoint( const WarpList &warps, const Vec2i &position ) 
{
	WarpRef	warp;
	size_t	i, index;
	float	d, distance=10.0e6f;	

	// find warp and distance to closest control point
	for(WarpConstReverseIter itr=warps.rbegin();itr!=warps.rend();++itr) {
		i = (*itr)->findControlPoint( position, &d );

		if(d < distance) {
			distance = d;
			index = i;
			warp = *itr;
		}
	}

	// select the closest control point and deselect all others
	for(WarpConstIter itr=warps.begin();itr!=warps.end();++itr) {
		if( *itr == warp ) 
			(*itr)->selectControlPoint(index);
		else 
			(*itr)->deselectControlPoint();
	}
}

WarpList Warp::readSettings( const DataSourceRef &source  )
{
	XmlTree		doc;
	WarpList	warps;

	// try to load the specified xml file
	try { doc = XmlTree( source ); }
	catch(...) { return warps; }

	// check if this is a valid file 
	bool isWarp = doc.hasChild("warpconfig");
	if(!isWarp) return warps;

	//
	if(isWarp) {
		// get first profile
		XmlTree profileXml = doc.getChild("warpconfig/profile");

		// iterate maps
		for(XmlTree::ConstIter child=profileXml.begin("map");child!=profileXml.end();++child) {
			XmlTree warpXml = child->getChild("warp");

			// create warp of the correct type
			std::string method = warpXml.getAttributeValue<std::string>("method", "unknown");
			if( method == "bilinear" ) {
				WarpBilinearRef warp( new WarpBilinear() );
				warp->fromXml(warpXml);
				warps.push_back( warp );
			}
			else if( method == "perspective" ) {
				WarpPerspectiveRef warp( new WarpPerspective() );
				warp->fromXml(warpXml);
				warps.push_back( warp );
			}
			else if( method == "perspectivebilinear" ) {
				WarpPerspectiveBilinearRef warp( new WarpPerspectiveBilinear() );
				warp->fromXml(warpXml);
				warps.push_back( warp );
			}
		}
	}

	return warps;
}

void Warp::writeSettings( const WarpList &warps, const DataTargetRef &target )
{
	// create default <profile> (profiles are not yet supported)
	XmlTree			profile;
	profile.setTag("profile");
	profile.setAttribute("name", "default");

	// 
	for(size_t i=0;i<warps.size();++i) {
		// create <map>
		XmlTree			map;
		map.setTag("map");
		map.setAttribute("id", i+1);
		map.setAttribute("display", 1);	// not supported yet

		// create <warp>
		map.push_back( warps[i]->toXml() );

		// add map to profile
		profile.push_back( map );
	}

	// create config document and root <warpconfig>
	XmlTree			doc;
	doc.setTag("warpconfig");
	doc.setAttribute("version", "1.0");
	doc.setAttribute("profile", "default");

	// add profile to root
	doc.push_back(profile);

	// write file
	doc.write(target);
}

bool Warp::handleMouseMove(WarpList &warps, MouseEvent event)
{
	// store mouse position 
	sMouse = event.getPos();

	// find and select closest control point
	selectClosestControlPoint( warps, sMouse );

	return false;
}

bool Warp::handleMouseDown(WarpList &warps, MouseEvent event)
{
	// store mouse position 
	sMouse = event.getPos();

	// find and select closest control point
	selectClosestControlPoint( warps, sMouse );

	bool handled = false;

	for(WarpReverseIter itr=warps.rbegin();itr!=warps.rend() && !handled;++itr)
		handled = (*itr)->mouseDown(event);

	return handled;
}

bool Warp::handleMouseDrag(WarpList &warps, MouseEvent event)
{
	bool handled = false;

	for(WarpReverseIter itr=warps.rbegin();itr!=warps.rend() && !handled;++itr)
		handled = (*itr)->mouseDrag(event);

	return handled;
}

bool Warp::handleMouseUp(WarpList &warps, MouseEvent event)
{
	return false;
}

bool Warp::handleKeyDown(WarpList &warps, KeyEvent event)
{
	bool handled = false;

	for(WarpReverseIter itr=warps.rbegin();itr!=warps.rend() && !handled;++itr)
		handled |= (*itr)->keyDown(event);
			
	switch( event.getCode() ) {
		case KeyEvent::KEY_UP:
		case KeyEvent::KEY_DOWN:
		case KeyEvent::KEY_LEFT:
		case KeyEvent::KEY_RIGHT:
			// do not select another control point
			break;
		default:
			// find and select closest control point
			selectClosestControlPoint( warps, sMouse );
			break;
	}

	return handled;
}

bool Warp::handleKeyUp(WarpList &warps, KeyEvent event)
{
	return false;
}

bool Warp::handleResize(WarpList &warps)
{
	for(WarpIter itr=warps.begin();itr!=warps.end();++itr)
		(*itr)->resize();

	return false;
}

bool Warp::mouseMove(cinder::app::MouseEvent event)
{
	float distance;

	mSelected = findControlPoint( event.getPos(), &distance );

	return false;
}

bool Warp::mouseDown(cinder::app::MouseEvent event)
{
	if(!sIsEditMode) return false;
	if(mSelected < 0 || mSelected >= mPoints.size()) return false;

	// calculate offset by converting control point from normalized to standard screen space
	Vec2f p = ( getControlPoint( mSelected ) * mWindowSize );
	mOffset = event.getPos() - p;

	return true;
}

bool Warp::mouseDrag(cinder::app::MouseEvent event)
{
	if(!sIsEditMode) return false;
	if(mSelected < 0 || mSelected >= mPoints.size()) return false;

	Vec2f m(event.getPos());
	Vec2f p(m.x - mOffset.x, m.y - mOffset.y);

	// set control point in normalized screen space
	setControlPoint( mSelected, p / mWindowSize );

	mIsDirty = true;

	return true;
}

bool Warp::mouseUp(cinder::app::MouseEvent event)
{
	return false;
}

bool Warp::keyDown( KeyEvent event )
{
	// disable keyboard input when not in edit mode
	if(sIsEditMode) {
		if(event.getCode() == KeyEvent::KEY_ESCAPE) {
			// gracefully exit edit mode 
			sIsEditMode = false;
			return true;
		}
	}
	else return false;

	// do not listen to key input if not selected
	if(mSelected < 0 || mSelected >= mPoints.size()) return false;

	switch( event.getCode() ) {
		case KeyEvent::KEY_UP: {
			if(mSelected < 0 || mSelected >= mPoints.size()) return false;
			float step = event.isShiftDown() ? 10.0f : 0.5f;
			mPoints[mSelected].y -= step / mWindowSize.y;
			mIsDirty = true; }
			break;
		case KeyEvent::KEY_DOWN: {
			if(mSelected < 0 || mSelected >= mPoints.size()) return false;
			float step = event.isShiftDown() ? 10.0f : 0.5f;
			mPoints[mSelected].y += step / mWindowSize.y;
			mIsDirty = true; }
			break;
		case KeyEvent::KEY_LEFT: {
			if(mSelected < 0 || mSelected >= mPoints.size()) return false;
			float step = event.isShiftDown() ? 10.0f : 0.5f;
			mPoints[mSelected].x -= step / mWindowSize.x;
			mIsDirty = true; }
			break;
		case KeyEvent::KEY_RIGHT: {
			if(mSelected < 0 || mSelected >= mPoints.size()) return false;
			float step = event.isShiftDown() ? 10.0f : 0.5f;
			mPoints[mSelected].x += step / mWindowSize.x;
			mIsDirty = true; }
			break;
		case KeyEvent::KEY_MINUS:
		case KeyEvent::KEY_KP_MINUS:
			if(mSelected < 0 || mSelected >= mPoints.size()) return false;
			mBrightness = math<float>::max(0.0f, mBrightness - 0.01f);
			break;
		case KeyEvent::KEY_PLUS:
		case KeyEvent::KEY_KP_PLUS:
			if(mSelected < 0 || mSelected >= mPoints.size()) return false;
			mBrightness = math<float>::min(1.0f, mBrightness + 0.01f);
			break;
		case KeyEvent::KEY_r:
			if(mSelected < 0 || mSelected >= mPoints.size()) return false;
			reset();
			mIsDirty = true;
			break;
		default:
			return false;
			break;
	}

	return true;
}

bool Warp::keyUp(KeyEvent event)
{
	return false;
}

bool Warp::resize()
{
	mWindowSize = Vec2f( getWindowSize() );

	mIsDirty = true; 

	return false; 
}

void Warp::drawControlPoint( const Vec2f &pt, bool selected, bool attached )
{ 
	float scale = 0.9f + 0.2f * math<float>::sin( 6.0f * float( app::getElapsedSeconds() - sSelectedTime ) );

	if(selected && attached) { drawControlPoint( pt, Color(0.0f, 0.8f, 0.0f) ); } 
	else if(selected) { drawControlPoint( pt, Color(0.9f, 0.9f, 0.9f), scale ); } 
	else if(attached) { drawControlPoint( pt, Color(0.0f, 0.4f, 0.0f) ); } 
	else { drawControlPoint( pt, Color(0.4f, 0.4f, 0.4f) ); }
}

void Warp::drawControlPoint(const Vec2f &pt, const Color &clr, float scale)
{
	// save current texture mode, drawing color, line width and blend mode
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT);

	// enable alpha blending, disable textures
	gl::enableAlphaBlending();
	glDisable( GL_TEXTURE_2D );		

	glLineWidth(1.0f);
	gl::color( ColorA( 0, 0, 0, 0.25f ) );
	gl::drawSolidCircle( pt, 15.0f * scale );

	gl::color( clr );
	gl::drawSolidCircle( pt, 3.0f * scale );	

	glLineWidth(2.0f);
	gl::color( ColorA( clr, 1.0f ) );
	gl::drawStrokedCircle( pt, 8.0f * scale );
	gl::drawStrokedCircle( pt, 12.0f * scale );

	gl::disableAlphaBlending();

	// restore drawing color, line width and blend mode
	glPopAttrib();
}

} } // namespace ph::warping
