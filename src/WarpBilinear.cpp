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

#include "WarpBilinear.h"

#include "cinder/Xml.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;

namespace ph { namespace warping {

WarpBilinear::WarpBilinear()
	: Warp(BILINEAR)
{
	mIsLinear = false;
	mIsNormalized = true;
	mFlipVertical = false;
	mIsAdaptive = false;

	mResolution  = 16; // higher value is coarser mesh
	mResolutionX = 0;
	mResolutionY = 0;

	mControlsX = 2;
	mControlsY = 2;

	reset();
}

WarpBilinear::~WarpBilinear()
{
}

XmlTree WarpBilinear::toXml() const
{
	XmlTree xml = Warp::toXml();

	// add attributes specific to this type of warp
	xml.setAttribute("resolution", mResolution);
	xml.setAttribute("linear", mIsLinear);
	xml.setAttribute("adaptive", mIsAdaptive);

	return xml;
}

void WarpBilinear::fromXml(const XmlTree &xml)
{
	Warp::fromXml(xml);

	// retrieve attributes specific to this type of warp
	mResolution = xml.getAttributeValue<int>("resolution", 16);
	mIsLinear = xml.getAttributeValue<bool>("linear", false);
	mIsAdaptive = xml.getAttributeValue<bool>("adaptive", false);
}

void WarpBilinear::reset()
{
	mPoints.clear();
	for(int x=0;x<mControlsX;x++) {
		for(int y=0;y<mControlsY;y++) {
			mPoints.push_back( Vec2f(x / float(mControlsX-1), y / float(mControlsY-1)) );
		}
	}

	mIsDirty = true;
}

void WarpBilinear::draw(const gl::Texture &texture, const Area &srcArea, const Rectf &destRect)
{
	// TODO: clip against bounds

	gl::SaveTextureBindState state( texture.getTarget() );

	// TODO: adjust texture coordinates using vertex shader

	texture.enableAndBind();
	draw();
}

void WarpBilinear::draw(bool controls)
{
	create();

	if(!mVboMesh) return;

	// save current texture mode, drawing color, line width and depth buffer state
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT | GL_DEPTH_BUFFER_BIT);

	gl::disableDepthRead();
	gl::disableDepthWrite();

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// draw textured mesh
	gl::color( Color(mBrightness, mBrightness, mBrightness) );
	gl::draw( mVboMesh );	

	// draw edit interface
	if( isEditModeEnabled() ) {
		glDisable( GL_TEXTURE_2D );

		// draw wireframe
		gl::color( ColorA(1, 1, 1, 0.5f) );
		gl::enableAlphaBlending();
		gl::enableWireframe();
		gl::draw( mVboMesh );
		gl::disableAlphaBlending();
		gl::disableWireframe();	

		if(controls) {
			// draw control points
			for(size_t i=0;i<mPoints.size();i++) 
				drawControlPoint( getControlPoint(i) * mWindowSize, i == mSelected );
		}
	}

	// restore states
	glPopAttrib();
}

bool WarpBilinear::keyDown(KeyEvent event)
{
	// let base class handle keys first
	if( Warp::keyDown( event ) )
		return true;

	// disable keyboard input when not in edit mode
	if( ! isEditModeEnabled() ) return false;

	// do not listen to key input if not selected
	if(mSelected < 0 || mSelected >= mPoints.size()) return false;

	switch( event.getCode() ) {
		case KeyEvent::KEY_F1:
			// reduce the number of horizontal control points
			if( !event.isShiftDown() ) 
				setNumControlX((mControlsX+1)/2);
			else setNumControlX(mControlsX-1);
			break;
		case KeyEvent::KEY_F2:
			// increase the number of horizontal control points
			if( !event.isShiftDown() ) 
				setNumControlX(2*mControlsX-1);
			else setNumControlX(mControlsX+1);
			break;
		case KeyEvent::KEY_F3:
			// reduce the number of vertical control points
			if( !event.isShiftDown() ) 
				setNumControlY((mControlsY+1)/2);
			else setNumControlY(mControlsY-1);
			break;
		case KeyEvent::KEY_F4:
			// increase the number of vertical control points
			if( !event.isShiftDown() ) 
				setNumControlY(2*mControlsY-1);
			else setNumControlY(mControlsY+1);
			break;
		case KeyEvent::KEY_TAB:
			// select the next of previous (+SHIFT) control point
			if( event.isShiftDown() ) {
				--mSelected;
				if(mSelected < 0) mSelected = (int) mPoints.size() - 1;
			}
			else { 
				++mSelected;
				if(mSelected >= (int) mPoints.size()) mSelected = 0;
			}
			break;
		case KeyEvent::KEY_m:
			// toggle between linear and curved mapping
			mIsLinear = !mIsLinear;
			mIsDirty = true;
			break;
		case KeyEvent::KEY_F5:
			// decrease the mesh resolution
			if(mResolution < 64) {
				mResolution+=4;
				mIsDirty = true;
			}
			break;
		case KeyEvent::KEY_F6:
			// increase the mesh resolution
			if(mResolution > 4) {
				mResolution-=4;
				mIsDirty = true;
			}
			break;
		case KeyEvent::KEY_F7:
			// toggle adaptive mesh resolution
			mIsAdaptive = !mIsAdaptive;
			mIsDirty = true;
			break;
		case KeyEvent::KEY_F9:
			// rotate content ccw
			break;
		case KeyEvent::KEY_F10: 
			// rotate content cw
			break;
		case KeyEvent::KEY_F11: 
			{
				// flip control points horizontally
				std::vector<Vec2f> points;
				for(int x=mControlsX-1;x>=0;--x) {
					for(int y=0;y<mControlsY;++y) {
						int i = (x * mControlsY + y);
						points.push_back( mPoints[i] );
					}
				}
				mPoints = points;
				mIsDirty = true;
			}
			break;
		case KeyEvent::KEY_F12: 
			{
				// flip control points vertically
				std::vector<Vec2f> points;
				for(int x=0;x<mControlsX;++x) {
					for(int y=mControlsY-1;y>=0;--y) {
						int i = (x * mControlsY + y);
						points.push_back( mPoints[i] );
					}
				}
				mPoints = points;
				mIsDirty = true;
			}
			break;
		default:
			return false;
			break;
	}

	return true;
}

void WarpBilinear::create()
{
	if(mIsDirty) {
		if(mIsAdaptive) {
			// determine a suitable mesh resolution based on width/height of the window
			// and the size of the mesh in pixels
			Rectf rect = getMeshBounds();
			createMesh( (int) (rect.getWidth() / mResolution), (int) (rect.getHeight() / mResolution) );
		}
		else {
			// use a fixed mesh resolution
			createMesh( mWidth / mResolution, mHeight / mResolution );
		}
		updateMesh();
	}
}

void WarpBilinear::createMesh(int resolutionX, int resolutionY)
{
	// convert from number of quads to number of vertices
	++resolutionX;	++resolutionY;

	// find a value for resolutionX and resolutionY that can be
	// evenly divided by mControlsX and mControlsY
	if(mControlsX < resolutionX) {
		int dx = (resolutionX-1) % (mControlsX-1);
		if(dx >= (mControlsX/2)) dx -= (mControlsX-1);
		resolutionX -= dx;
	} else {
		resolutionX = mControlsX;
	}

	if(mControlsY < resolutionY) {
		int dy = (resolutionY-1) % (mControlsY-1);
		if(dy >= (mControlsY/2)) dy -= (mControlsY-1);
		resolutionY -= dy;
	} else {
		resolutionY = mControlsY;
	}

	// only create a new mesh if necessary
	if(mVboMesh && (mResolutionX == resolutionX) && (mResolutionY == resolutionY))
		return;

	//
	mResolutionX = resolutionX;
	mResolutionY = resolutionY;

	//
	int numVertices = (resolutionX * resolutionY);
	int numQuads = (resolutionX - 1) * (resolutionY - 1);
	int numIndices = numQuads * 4; 

	//
	gl::VboMesh::Layout	layout;
	layout.setStaticIndices();
	layout.setStaticTexCoords2d();
	layout.setDynamicPositions();

	//
	mVboMesh = gl::VboMesh( numVertices, numIndices, layout, GL_QUADS );
	if(!mVboMesh) return;

	//
	Vec2f size = (mIsNormalized) ? Vec2f::one() : Vec2f((float)mWidth, (float)mHeight);

	// buffer static data
	int i = 0;
	int j = 0;
	std::vector<uint32_t>	indices( numIndices );
	std::vector<Vec2f>		texCoords( numVertices );
	for(int x=0; x<resolutionX; ++x) {
		for(int y=0; y<resolutionY; ++y) {
			// index
			if( ((x+1)<resolutionX) && ((y+1)<resolutionY) ) {
				indices[i++] = (x+0) * resolutionY + (y+0);
				indices[i++] = (x+1) * resolutionY + (y+0);
				indices[i++] = (x+1) * resolutionY + (y+1);
				indices[i++] = (x+0) * resolutionY + (y+1);
			}
			// texCoord
			if(mFlipVertical) {
				texCoords[j++] = Vec2f( x / (float)(resolutionX-1) * size.x, 
					(1.0f - y / (float)(resolutionY-1)) * size.y );
			}
			else {
				texCoords[j++] = Vec2f( x / (float)(resolutionX-1) * size.x, 
					y / (float)(resolutionY-1) * size.y );
			}
		}
	}
	mVboMesh.bufferIndices( indices );
	mVboMesh.bufferTexCoords2d( 0, texCoords );

	//
	mIsDirty = true;
}

void WarpBilinear::updateMesh()
{
	if(!mVboMesh) return;
	if(!mIsDirty) return;

	Vec2f			p;
	float			u, v;
	int				col, row;

	std::vector<Vec2f>	cols, rows;

	gl::VboMesh::VertexIter iter = mVboMesh.mapVertexBuffer();
	for(int x=0; x<mResolutionX; ++x) {
		for(int y=0; y<mResolutionY; ++y) {
			// transform coordinates to [0..numControls]
			u = x * (mControlsX - 1) / (float)(mResolutionX - 1);
			v = y * (mControlsY - 1) / (float)(mResolutionY - 1);

			// determine col and row
			col = (int)(u);
			row = (int)(v);

			// normalize coordinates to [0..1]
			u -= col;
			v -= row;

			if(mIsLinear) {
				// perform linear interpolation
				Vec2f p1 = (1.0f - u) * getPoint(col, row) + u * getPoint(col+1, row);
				Vec2f p2 = (1.0f - u) * getPoint(col, row+1) + u * getPoint(col+1, row+1);
				p = ((1.0f - v) * p1 + v * p2) * mWindowSize;
			}
			else {
				// perform bicubic interpolation
				rows.clear();
				for(int i=-1;i<3;++i) {
					cols.clear();
					for(int j=-1;j<3;++j) {
						cols.push_back( getPoint(col + i, row + j) );
					}
					rows.push_back( cubicInterpolate( cols, v ) );
				}
				p = cubicInterpolate( rows, u ) * mWindowSize;
			}

			iter.setPosition( p.x, p.y, 0.0f );
			++iter;
		}
	}

	mIsDirty = false;
}

Vec2f WarpBilinear::getPoint(int col, int row) const
{
	int maxCol = mControlsX-1;
	int maxRow = mControlsY-1;

	// here's the magic: extrapolate points beyond the edges
	if(col < 0) return 2.0f * getPoint(0, row) - getPoint(0-col, row);
	if(row < 0) return 2.0f * getPoint(col, 0) - getPoint(col, 0-row);
	if(col > maxCol) return 2.0f * getPoint(maxCol,row) - getPoint(2*maxCol-col,row);
	if(row > maxRow) return 2.0f * getPoint(col,maxRow) - getPoint(col,2*maxRow-row);

	// points on the edges or within the mesh can simply be looked up
	return mPoints[(col * mControlsY) + row];
}

// from http://www.paulinternet.nl/?page=bicubic : fast catmull-rom calculation
Vec2f WarpBilinear::cubicInterpolate( const std::vector<Vec2f> &knots, float t ) const
{
	assert( knots.size() >= 4 );

	return knots[1] + 0.5f * t*(knots[2] - knots[0] + 
		t*(2.0f*knots[0] - 5.0f*knots[1] +
		4.0f*knots[2] - knots[3] +
		t*(3.0f*(knots[1] - knots[2]) +
		knots[3] - knots[0])));
}

void WarpBilinear::setNumControlX(int n)
{
	// there should be a minimum of 2 control points
	n = math<int>::max(2, n);

	// create a list of new points
	std::vector<Vec2f> temp(n * mControlsY);

	// perform spline fitting
	for(int row=0;row<mControlsY;++row) {
		std::vector<Vec2f> points;
		if(mIsLinear) {
			// construct piece-wise linear spline
			for(int col=0;col<mControlsX;++col) {
				points.push_back( getPoint(col, row) );
			}

			BSpline2f s( points, 1, false, true );

			// calculate position of new control points
			float length = s.getLength(0.0f, 1.0f);
			float step = 1.0f / (n-1);
			for(int col=0;col<n;++col) {
				temp[(col * mControlsY) + row] = s.getPosition( s.getTime( length * col * step ) );
			}
		}
		else {
			// construct piece-wise catmull-rom spline
			for(int col=0;col<mControlsX;++col) {
				Vec2f p0 = getPoint(col-1, row);
				Vec2f p1 = getPoint(col, row);
				Vec2f p2 = getPoint(col+1, row);
				Vec2f p3 = getPoint(col+2, row);

				// control points according to an optimized Catmull-Rom implementation
				Vec2f b1 = p1 + (p2 - p0) / 6.0f;
				Vec2f b2 = p2 - (p3 - p1) / 6.0f;

				points.push_back(p1);

				if(col < (mControlsX-1)) {
					points.push_back(b1);
					points.push_back(b2);
				}
			}

			BSpline2f s( points, 3, false, true );

			// calculate position of new control points
			float length = s.getLength(0.0f, 1.0f);
			float step = 1.0f / (n-1);
			for(int col=0;col<n;++col) {
				temp[(col * mControlsY) + row] = s.getPosition( s.getTime( length * col * step ) );
			}
		}
	}

	// copy new control points 
	mPoints = temp;
	mControlsX = n;

	mIsDirty = true;
}

void WarpBilinear::setNumControlY(int n)
{
	// there should be a minimum of 2 control points
	n = math<int>::max(2, n);

	// create a list of new points
	std::vector<Vec2f> temp(mControlsX * n);

	// perform spline fitting
	for(int col=0;col<mControlsX;++col) {
		std::vector<Vec2f> points;
		if(mIsLinear) {
			// construct piece-wise linear spline
			for(int row=0;row<mControlsY;++row) 
				points.push_back( getPoint(col, row) );
			
			BSpline2f s( points, 1, false, true );

			// calculate position of new control points
			float length = s.getLength(0.0f, 1.0f);
			float step = 1.0f / (n-1);
			for(int row=0;row<n;++row) {
				temp[(col * n) + row] = s.getPosition( s.getTime( length * row * step ) );
			}
		}
		else {
			// construct piece-wise catmull-rom spline
			for(int row=0;row<mControlsY;++row) {
				Vec2f p0 = getPoint(col, row-1);
				Vec2f p1 = getPoint(col, row);
				Vec2f p2 = getPoint(col, row+1);
				Vec2f p3 = getPoint(col, row+2);

				// control points according to an optimized Catmull-Rom implementation
				Vec2f b1 = p1 + (p2 - p0) / 6.0f;
				Vec2f b2 = p2 - (p3 - p1) / 6.0f;

				points.push_back(p1);

				if(row < (mControlsY-1)) {
					points.push_back(b1);
					points.push_back(b2);
				}
			}

			BSpline2f s( points, 3, false, true );

			// calculate position of new control points
			float length = s.getLength(0.0f, 1.0f);
			float step = 1.0f / (n-1);
			for(int row=0;row<n;++row) {
				temp[(col * n) + row] = s.getPosition( s.getTime( length * row * step ) );
			}
		}
	}

	// copy new control points 
	mPoints = temp;
	mControlsY = n;

	mIsDirty = true;
}

Rectf WarpBilinear::getMeshBounds() const
{
	Vec2f min = Vec2f::one();
	Vec2f max = Vec2f::zero();

	for(size_t i=0;i<mPoints.size();++i) {
		min.x = math<float>::min( mPoints[i].x, min.x );
		min.y = math<float>::min( mPoints[i].y, min.y );
		max.x = math<float>::max( mPoints[i].x, max.x );
		max.y = math<float>::max( mPoints[i].y, min.y );
	}

	return Rectf(min * mWindowSize, max * mWindowSize);
}

} } // namespace ph::warping


