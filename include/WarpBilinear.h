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

#pragma once

#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"

#include "Warp.h"

#include <boost/shared_ptr.hpp>
#include <cassert>

namespace ph { namespace warping {

typedef boost::shared_ptr<class WarpBilinear>	WarpBilinearRef;

class WarpBilinear
	: public Warp, public boost::enable_shared_from_this<WarpBilinear>
{
public:
	//WarpBilinear(const ci::gl::Fbo::Format &format=ci::gl::Fbo::Format());
	WarpBilinear();
	virtual ~WarpBilinear(void);

	//!
	virtual ci::XmlTree	toXml() const; // overrides base class
	//!
	virtual void		fromXml(const ci::XmlTree &xml); // overrides base class

	//! set the frame buffer format, so you have control over its quality settings
	void				setFormat( const ci::gl::Fbo::Format &format );
	//!
	void				setLinear( bool enabled=true ) { mIsLinear=enabled; mIsDirty=true; };
	void				setCurved( bool enabled=true ) { mIsLinear=!enabled; mIsDirty=true; };

	//! reset control points to undistorted image
	virtual void		reset();
	//! setup the warp before drawing its contents
	virtual void		begin();
	//! restore the warp after drawing
	virtual void		end();

	//! draws a warped texture
	virtual void		draw(const ci::gl::Texture &texture, const ci::Area &srcArea, const ci::Rectf &destRect); 

	//! set the number of horizontal control points for this warp 
	void				setNumControlX(int n);
	//! set the number of vertical control points for this warp
	void				setNumControlY(int n);

	void				setTexCoords( float x1, float y1, float x2, float y2 );

	virtual bool		keyDown( ci::app::KeyEvent event ); 
protected:
	//! draws the warp as a mesh, allowing you to use your own texture instead of the FBO
	virtual void		draw(bool controls=true);
	//! Creates the frame buffer object and updates the vertex buffer object is necessary
	void				create();
	//! Creates the vertex buffer object
	void				createMesh( int resolutionX=36, int resolutionY=36 );
	//! Updates the vertex buffer object based on the control points
	void				updateMesh();
	//!	Returns the specified control point. Values for col and row are clamped to prevent errors.
	ci::Vec2f			getPoint(int col, int row) const;
	//! Performs fast Catmull-Rom interpolation, returns the interpolated value at t
	ci::Vec2f			cubicInterpolate( const std::vector<ci::Vec2f> &knots, float t ) const;
	//!
	ci::Rectf			getMeshBounds() const;
private:
	//! Greatest common divisor using Euclidian algorithm (from: http://en.wikipedia.org/wiki/Greatest_common_divisor)
	int				gcd(int a, int b) const { if(b==0) return a; else return gcd(b, a%b); };
protected:
	ci::gl::Fbo				mFbo;
	ci::gl::Fbo::Format		mFboFormat;
	ci::gl::VboMesh			mVboMesh;

	//! linear or curved interpolation
	bool					mIsLinear;
	//!
	bool					mIsAdaptive;

	//! texture coordinates of corners
	float					mX1, mY1, mX2, mY2;

	//! Determines the detail of the generated mesh. Multiples of 5 seem to work best.
	int						mResolution;

	//! Determines the number of horizontal and vertical quads 
	int						mResolutionX;
	int						mResolutionY;
};

} } // namespace ph::warping
