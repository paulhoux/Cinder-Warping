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

namespace ph {
namespace warping {

typedef std::shared_ptr<class WarpPerspective> WarpPerspectiveRef;

class WarpPerspective
	: public Warp {
public:
	//
	static WarpPerspectiveRef create() { return std::make_shared<WarpPerspective>(); }

public:
	WarpPerspective( void );
	~WarpPerspective( void );

	//! returns a shared pointer to this warp
	WarpPerspectiveRef	getPtr() { return std::static_pointer_cast<WarpPerspective>( shared_from_this() ); }

	//! get the transformation matrix
	ci::mat4		getTransform();
	//! get the inverted transformation matrix
	ci::mat4		getInvertedTransform();

	//! reset control points to undistorted image
	void			reset() override;
	//! setup the warp before drawing its contents
	void			begin() override;
	//! restore the warp after drawing
	void			end() override;

	//! draws a warped texture
	void			draw( const ci::gl::Texture2dRef &texture, const ci::Area &srcArea, const ci::Rectf &destRect ) override;

	//! override keyDown method to add additional key handling
	void			keyDown( ci::app::KeyEvent &event ) override;

	//! allow WarpPerspectiveBilinear to access the protected class members
	friend class WarpPerspectiveBilinear;
protected:
	//!
	void	draw( bool controls = true ) override;
	//!
	inline	ci::vec2	toVec2f( const cv::Point2f &pt ) const { return ci::vec2( pt.x, pt.y ); };
	inline	cv::Point2f	fromVec2f( const ci::vec2 &pt ) const { return cv::Point2f( pt.x, pt.y ); };
protected:
	cv::Point2f	mSource[4];
	cv::Point2f	mDestination[4];

	ci::mat4	mTransform;
	ci::mat4	mInverted;
};

}
} // namespace ph::warping
