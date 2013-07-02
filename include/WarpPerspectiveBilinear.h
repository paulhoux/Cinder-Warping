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

namespace ph { namespace warping {

typedef boost::shared_ptr<class WarpPerspectiveBilinear>	WarpPerspectiveBilinearRef;

class WarpPerspectiveBilinear
	: public WarpBilinear, public boost::enable_shared_from_this<WarpPerspectiveBilinear>
{
public:
	WarpPerspectiveBilinear();
	~WarpPerspectiveBilinear(void);

	//!
	ci::XmlTree	toXml() const;
	//!
	void		fromXml(const ci::XmlTree &xml);

	bool		mouseMove( ci::app::MouseEvent event );
	bool		mouseDown( ci::app::MouseEvent event );
	bool		mouseDrag( ci::app::MouseEvent event );

	bool		keyDown( ci::app::KeyEvent event );

	bool		resize();

	//! set the width and height of the content in pixels
	void		setSize(int w, int h);
	//! set the width and height of the content in pixels
	void		setSize(const ci::Vec2i &size);

	//! returns the coordinates of the specified control point
	ci::Vec2f	getControlPoint(unsigned index) const;
	//! sets the coordinates of the specified control point
	void		setControlPoint(unsigned index, const ci::Vec2f &pos);
	//! moves the specified control point 
	void		moveControlPoint(unsigned index, const ci::Vec2f &shift);
	//! select one of the control points
	void		selectControlPoint(unsigned index);
	//! deselect the selected control point
	void		deselectControlPoint();
protected:
	//! 
	void		draw(bool controls=true);

	//! returns whether or not the control point is one of the 4 corners and should be treated as a perspective control point
	bool		isCorner(unsigned index) const;
	//! converts the control point index to the appropriate perspective warp index
	unsigned	convertIndex(unsigned index) const;

protected:
	WarpPerspectiveRef	mWarp;
};

} } // namespace ph::warping
