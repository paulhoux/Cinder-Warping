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

#include "cinder/Area.h"
#include "cinder/Color.h"
#include "cinder/DataSource.h"
#include "cinder/DataTarget.h"
#include "cinder/Matrix.h"
#include "cinder/Rect.h"
#include "cinder/Vector.h"

#include <vector>
#include <boost/enable_shared_from_this.hpp>

// forward declarations

namespace cinder {
	class XmlTree;

	namespace app {
		class KeyEvent;
		class MouseEvent;
	}

	namespace gl {
		class Texture;
	}
}

//

namespace ph { namespace warping {

typedef boost::shared_ptr<class Warp>		WarpRef;
typedef std::vector<WarpRef>				WarpList;
typedef WarpList::iterator					WarpIter;
typedef WarpList::const_iterator			WarpConstIter;
typedef WarpList::reverse_iterator			WarpReverseIter;
typedef WarpList::const_reverse_iterator	WarpConstReverseIter;

class Warp
	: public boost::enable_shared_from_this<Warp>
{
public:
	typedef enum { UNKNOWN, BILINEAR, PERSPECTIVE, PERSPECTIVE_BILINEAR } WarpType;
public:
	Warp(WarpType type=UNKNOWN);
	virtual ~Warp(void);

	//!
	static bool			isEditModeEnabled() { return sIsEditMode; };
	static void			enableEditMode(bool enabled=true) { sIsEditMode = enabled; };
	static void			disableEditMode() { sIsEditMode = false; };
	static void			toggleEditMode() { sIsEditMode = !sIsEditMode; };

	//! returns the type of the warp
	WarpType			getType() const { return mType; };

	//!
	virtual ci::XmlTree	toXml() const;
	//!
	virtual void		fromXml(const ci::XmlTree &xml);

	//! get the width of the content in pixels
	int					getWidth() const { return mWidth; };
	//! get the height of the content in pixels
	int					getHeight() const { return mHeight; };
	//! get the width and height of the content in pixels
	ci::Vec2i			getSize() const { return ci::Vec2i(mWidth, mHeight); };
	//! get the width and height of the content in pixels
	ci::Area			getBounds() const { return ci::Area(0, 0, mWidth, mHeight); };

	//! set the width of the content in pixels
	virtual void		setWidth(int w) { setSize(w, mHeight); }
	//! set the height of the content in pixels
	virtual void		setHeight(int h) { setSize(mWidth, h); }
	//! set the width and height of the content in pixels
	virtual void		setSize(int w, int h);
	//! set the width and height of the content in pixels
	virtual void		setSize(const ci::Vec2i &size);

	//! reset control points to undistorted image
	virtual void		reset() = 0;
	//! setup the warp before drawing its contents
	//virtual void		begin() = 0;
	//! restore the warp after drawing
	//virtual void		end() = 0;

	//! draws a warped texture
	virtual void		draw(const ci::gl::Texture &texture);
	//! draws a warped texture
	virtual void		draw(const ci::gl::Texture &texture, const ci::Area &srcArea, const ci::Rectf &destRect) = 0;

	//! returns the coordinates of the specified control point
	virtual ci::Vec2f	getControlPoint(size_t index) const;
	//! sets the coordinates of the specified control point
	virtual void		setControlPoint(size_t index, const ci::Vec2f &pos);
	//! moves the specified control point 
	virtual void		moveControlPoint(size_t index, const ci::Vec2f &shift);
	//! select one of the control points
	virtual void		selectControlPoint(size_t index);
	//! deselect the selected control point
	virtual void		deselectControlPoint();
	//! returns the index of the closest control point, as well as the distance in pixels
	virtual size_t		findControlPoint(const ci::Vec2f &pos, float *distance) const ;

	//! checks all warps and selects the closest control point
	static void			selectClosestControlPoint( const WarpList &warps, const ci::Vec2i &position );		
	
	//! draw a control point in the correct preset color
	static void			drawControlPoint( const ci::Vec2f &pt, bool selected=false, bool attached=false );
	//! draw a control point in the specified color
	static void			drawControlPoint( const ci::Vec2f &pt, const ci::Color &clr, float scale=1.0f );

	//! read a settings xml file and pass back a vector of Warps
	static WarpList		readSettings( const ci::DataSourceRef &source );
	//! write a settings xml file
	static void			writeSettings( const WarpList &warps, const ci::DataTargetRef &target );

	//! handles mouseMove events for multiple warps
	static bool			handleMouseMove( WarpList &warps, ci::app::MouseEvent event );
	//! handles mouseDown events for multiple warps
	static bool			handleMouseDown( WarpList &warps, ci::app::MouseEvent event );
	//! handles mouseDrag events for multiple warps
	static bool			handleMouseDrag( WarpList &warps, ci::app::MouseEvent event );
	//! handles mouseUp events for multiple warps
	static bool			handleMouseUp( WarpList &warps, ci::app::MouseEvent event );

	//! handles keyDown events for multiple warps
	static bool			handleKeyDown( WarpList &warps, ci::app::KeyEvent event );
	//! handles keyUp events for multiple warps
	static bool			handleKeyUp( WarpList &warps, ci::app::KeyEvent event );

	//! handles resize events for multiple warps
	static bool			handleResize( WarpList &warps );

	virtual bool		mouseMove( ci::app::MouseEvent event );
	virtual bool		mouseDown( ci::app::MouseEvent event );
	virtual bool		mouseDrag( ci::app::MouseEvent event );
	virtual bool		mouseUp( ci::app::MouseEvent event );

	virtual bool		keyDown( ci::app::KeyEvent event );
	virtual bool		keyUp( ci::app::KeyEvent event );

	virtual bool		resize();
protected:
	//! draw the warp and its editing interface
	virtual void		draw(bool controls=true) = 0;

protected:
	WarpType		mType;

	bool			mIsDirty;

	int				mWidth;
	int				mHeight;
	ci::Vec2f		mWindowSize;

	float			mBrightness;

	size_t			mSelected;

	//! Determines the number of horizontal and vertical control points
	int				mControlsX;
	int				mControlsY;

	std::vector<ci::Vec2f>	mPoints;

private:
	//! edit mode for all warps
	static bool			sIsEditMode;
	//! time of last control point selection
	static double		sSelectedTime;
	//! keep track of mouse position
	static ci::Vec2i	sMouse;

	ci::Vec2f			mOffset;
};

} } // namespace ph::warping
