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

// requires Cinder's OpenCV block
#include "CinderOpenCV.h"

#include "Warp.h"

namespace ph { namespace warping {

typedef boost::shared_ptr<class WarpPerspective> WarpPerspectiveRef;

class WarpPerspective
	: public Warp, public boost::enable_shared_from_this<WarpPerspective>
{
public:
	WarpPerspective(void);
	~WarpPerspective(void);

	//! get the transformation matrix
	ci::Matrix44f	getTransform();
	//! get the inverted transformation matrix (custom code, faster and more accurate than Cinder's)
	ci::Matrix44f	getInvertedTransform();

	//! reset control points to undistorted image
	void			reset();
	//! setup the warp before drawing its contents
	void			begin();
	//! restore the warp after drawing
	void			end();

	//! override keyDown method to add additional key handling
	bool			keyDown( ci::app::KeyEvent event );

	//! allow WarpPerspectiveBilinear to access the protected class members
	friend class WarpPerspectiveBilinear;
protected:
	//!
	void	draw(bool controls=true);
	//!
	inline	ci::Vec2f		toVec2f( const cv::Point2f &pt ) const { return ci::Vec2f( pt.x, pt.y ); };
	inline	cv::Point2f		fromVec2f( const ci::Vec2f &pt ) const { return cv::Point2f( pt.x, pt.y ); };
protected:
	cv::Point2f		mSource[4];
	cv::Point2f		mDestination[4];

	ci::Matrix44d	mTransform;
	ci::Matrix44d	mInverted;
};

} } // namespace ph::warping
