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
	ci::Vec2f	getControlPoint(size_t index) const;
	//! sets the coordinates of the specified control point
	void		setControlPoint(size_t index, const ci::Vec2f &pos);
	//! moves the specified control point 
	void		moveControlPoint(size_t index, const ci::Vec2f &shift);
	//! select one of the control points
	void		selectControlPoint(size_t index);
	//! deselect the selected control point
	void		deselectControlPoint();
protected:
	//! 
	void		draw(bool controls=true);

	//! returns whether or not the control point is one of the 4 corners and should be treated as a perspective control point
	bool		isCorner(size_t index) const;
	//! converts the control point index to the appropriate perspective warp index
	size_t		convertIndex(size_t index) const;

protected:
	WarpPerspectiveRef	mWarp;
};

} } // namespace ph::warping
