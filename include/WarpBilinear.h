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
	WarpBilinear(const ci::gl::Fbo::Format &format=ci::gl::Fbo::Format());
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
	virtual void		reset(); // overrides base class
	//! setup the warp before drawing its contents
	virtual void		begin(); // overrides base class
	//! restore the warp after drawing
	virtual void		end(); // overrides base class

	//! set the number of horizontal control points for this warp 
	void				setNumControlX(int n);
	//! set the number of vertical control points for this warp
	void				setNumControlY(int n);

	void				flipVertical(bool enabled=true) { mFlipVertical=enabled; mIsDirty=true; create(); };
	void				setNormalizedTexCoords(bool enabled=true) { mIsNormalized=enabled; mIsDirty=true; create(); };

	virtual bool		keyDown( ci::app::KeyEvent event ); // overrides base class
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
	ci::gl::VboMesh			mVboMesh;

	//
	ci::gl::Fbo::Format		mFboFormat;

	//! linear or curved interpolation
	bool					mIsLinear;
	//!
	bool					mIsNormalized;
	bool					mFlipVertical;
	bool					mIsAdaptive;

	//! Determines the detail of the generated mesh. Multiples of 5 seem to work best.
	int						mResolution;

	//! Determines the number of horizontal and vertical quads 
	int						mResolutionX;
	int						mResolutionY;
};

} } // namespace ph::warping
