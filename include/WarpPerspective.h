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

	//! draws a warped texture
	void			draw(const ci::gl::Texture &texture, const ci::Area &srcArea, const ci::Rectf &destRect);

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
