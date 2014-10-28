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

#include "WarpBilinear.h"
#include "WarpPerspective.h"

namespace ph {
namespace warping {

typedef std::shared_ptr<class WarpPerspectiveBilinear>	WarpPerspectiveBilinearRef;

class WarpPerspectiveBilinear
	: public WarpBilinear {
public:
	//
	static WarpPerspectiveBilinearRef create( const ci::gl::Fbo::Format &format = ci::gl::Fbo::Format() ) { return std::make_shared<WarpPerspectiveBilinear>( format ); }

public:
	WarpPerspectiveBilinear( const ci::gl::Fbo::Format &format = ci::gl::Fbo::Format() );
	~WarpPerspectiveBilinear( void );

	//! returns a shared pointer to this warp
	WarpPerspectiveBilinearRef	getPtr() { return std::static_pointer_cast<WarpPerspectiveBilinear>( shared_from_this() ); }

	//!
	ci::XmlTree	toXml() const override;
	//!
	void		fromXml( const ci::XmlTree &xml ) override;

	void		mouseMove( ci::app::MouseEvent &event ) override;
	void		mouseDown( ci::app::MouseEvent &event ) override;
	void		mouseDrag( ci::app::MouseEvent &event ) override;

	void		keyDown( ci::app::KeyEvent &event ) override;

	void		resize() override;

	//! set the width and height of the content in pixels
	void		setSize( int w, int h ) override;
	//! set the width and height of the content in pixels
	void		setSize( const ci::ivec2 &size ) override;

	//! returns the coordinates of the specified control point
	ci::vec2	getControlPoint( unsigned index ) const override;
	//! sets the coordinates of the specified control point
	void		setControlPoint( unsigned index, const ci::vec2 &pos ) override;
	//! moves the specified control point 
	void		moveControlPoint( unsigned index, const ci::vec2 &shift ) override;
	//! select one of the control points
	void		selectControlPoint( unsigned index ) override;
	//! deselect the selected control point
	void		deselectControlPoint() override;
protected:
	//! 
	void		draw( bool controls = true ) override;

	//! returns whether or not the control point is one of the 4 corners and should be treated as a perspective control point
	bool		isCorner( unsigned index ) const;
	//! converts the control point index to the appropriate perspective warp index
	unsigned	convertIndex( unsigned index ) const;

protected:
	WarpPerspectiveRef	mWarp;
};

}
} // namespace ph::warping
