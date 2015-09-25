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

#pragma once

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

	//! find homography based on source and destination quad
	ci::mat4 getPerspectiveTransform( const ci::vec2 src[4], const ci::vec2 dst[4] ) const;
	//! helper function
	void gaussianElimination( float * input, int n ) const;

protected:
	ci::vec2	mSource[4];
	ci::vec2	mDestination[4];

	ci::mat4	mTransform;
	ci::mat4	mInverted;
};

}
} // namespace ph::warping
