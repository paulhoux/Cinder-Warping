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

#include "cinder/Area.h"
#include "cinder/Color.h"
#include "cinder/DataSource.h"
#include "cinder/DataTarget.h"
#include "cinder/Matrix.h"
#include "cinder/Rect.h"
#include "cinder/Vector.h"

#include "cinder/gl/gl.h"

#include <atomic>
#include <vector>

// forward declarations

namespace cinder {
class XmlTree;

namespace app {
class KeyEvent;
class MouseEvent;
}

namespace gl {
class Texture2d;
typedef std::shared_ptr<Texture2d> Texture2dRef;
}
}

//

namespace ph {
namespace warping {

typedef std::shared_ptr<class Warp>      WarpRef;
typedef std::vector<WarpRef>             WarpList;
typedef WarpList::iterator               WarpIter;
typedef WarpList::const_iterator         WarpConstIter;
typedef WarpList::reverse_iterator       WarpReverseIter;
typedef WarpList::const_reverse_iterator WarpConstReverseIter;

class Warp : public std::enable_shared_from_this<Warp> {
  public:
	typedef enum { UNKNOWN, BILINEAR, PERSPECTIVE, PERSPECTIVE_BILINEAR } WarpType;

  public:
	Warp( WarpType type = UNKNOWN );
	virtual ~Warp() {}

	//! Returns \c TRUE if edit mode is enabled.
	static bool isEditModeEnabled() { return (bool)sIsEditMode; };
	//! Enables or disables edit mode.
	static void enableEditMode( bool enabled = true ) { sIsEditMode = enabled; };
	//! Disables edit mode.
	static void disableEditMode() { sIsEditMode = false; };
	//! Toggles edit mode.
	static void toggleEditMode()
	{
		bool exp = (bool)sIsEditMode;
		sIsEditMode.compare_exchange_strong( exp, !(bool)sIsEditMode );
	};

	//! Returns \c TRUE if gamma mode is enabled. If enabled, renders a gamma correction test image instead of the content.
	static bool isGammaModeEnabled() { return (bool)sIsGammaMode; };
	//! Enables or disables gamma mode. If enabled, renders a gamma correction test image instead of the content.
	static void enableGammaMode( bool enabled = true ) { sIsGammaMode = enabled; };
	//! Disables gamma mode.
	static void disableGammaMode() { sIsGammaMode = false; };
	//! Toggles gamma mode. If enabled, renders a gamma correction test image instead of the content.
	static void toggleGammaMode()
	{
		bool exp = (bool)sIsGammaMode;
		sIsGammaMode.compare_exchange_strong( exp, !(bool)sIsGammaMode );
	};

	//! Returns the type of the warp.
	WarpType getType() const { return mType; };
	//! Returns a shared pointer to this warp.
	WarpRef getPtr() { return shared_from_this(); }
	//!
	virtual ci::XmlTree toXml() const;
	//!
	virtual void fromXml( const ci::XmlTree &xml );

	//! Get the width of the content in pixels.
	int getWidth() const { return mWidth; };
	//! Get the height of the content in pixels.
	int getHeight() const { return mHeight; };
	//! Get the width and height of the content in pixels.
	ci::ivec2 getSize() const { return ci::ivec2( mWidth, mHeight ); };
	//! Get the width and height of the content in pixels.
	ci::Area getBounds() const { return ci::Area( 0, 0, mWidth, mHeight ); };
	//! Set the width of the content in pixels.
	virtual void setWidth( int w ) { setSize( w, mHeight ); }
	//! Set the height of the content in pixels.
	virtual void setHeight( int h ) { setSize( mWidth, h ); }
	//! Set the width and height of the content in pixels.
	virtual void setSize( const ci::ivec2 &size ) { setSize( size.x, size.y ); }
	//! Set the width and height of the content in pixels.
	virtual void setSize( int w, int h );

	//! Returns the luminance value for the red, green and blue channels, used for edge blending (0.5 = linear).
	virtual const ci::vec3 &getLuminance() const { return mLuminance; }
	//! Set the luminance value for all color channels, used for edge blending (0.5 = linear).
	virtual void setLuminance( float gamma ) { mLuminance = ci::vec3( gamma ); }
	//! Set the luminance value for the red, green and blue channels, used for edge blending (0.5 = linear).
	virtual void setLuminance( float red, float green, float blue )
	{
		mLuminance.r = red;
		mLuminance.g = green;
		mLuminance.b = blue;
	}

	//! Returns the gamma curve value for the red, green and blue channels.
	virtual const ci::vec3 &getGamma() const { return mGamma; }
	//! Set the gamma curve value for all color channels. Gamma only affects edge blending, it does not alter the content.
	virtual void setGamma( float gamma ) { mGamma = ci::vec3( gamma ); }
	//! Set the gamma curve value for the red, green and blue channels.
	virtual void setGamma( float red, float green, float blue )
	{
		mGamma.r = red;
		mGamma.g = green;
		mGamma.b = blue;
	}

	//! Returns the edge blending curve exponent (1.0 = linear, 2.0 = quadratic).
	virtual float getExponent() const { return mExponent; }
	//! Set the edge blending curve exponent  (1.0 = linear, 2.0 = quadratic).
	virtual void setExponent( float e ) { mExponent = glm::clamp( e, 1.0f, 100.0f ); }
	//! Returns the edge blending area for the left, top, right and bottom edges (values between 0 and 1).
	virtual ci::vec4 getEdges() const { return ci::vec4( mEdges.x, mEdges.y, 1.0f - mEdges.z, 1.0f - mEdges.w ); }
	//! Set the edge blending area for the left, top, right and bottom edges (values between 0 and 1).
	virtual void setEdges( float left, float top, float right, float bottom )
	{
		mEdges.x = glm::clamp( left, 0.0f, 1.0f );
		mEdges.y = glm::clamp( top, 0.0f, 1.0f );
		mEdges.z = glm::clamp( 1.0f - right, 0.0f, 1.0f );
		mEdges.w = glm::clamp( 1.0f - bottom, 0.0f, 1.0f );
	}
	//! Set the edge blending area for the left, top, right and bottom edges (values between 0 and 1).
	virtual void setEdges( const ci::vec4 &edges )
	{
		mEdges.x = glm::clamp( edges.x, 0.0f, 1.0f );
		mEdges.y = glm::clamp( edges.y, 0.0f, 1.0f );
		mEdges.z = glm::clamp( 1.0f - edges.z, 0.0f, 1.0f );
		mEdges.w = glm::clamp( 1.0f - edges.w, 0.0f, 1.0f );
	}

	//! Reset control points to undistorted image.
	virtual void reset() = 0;
	//! Setup the warp before drawing its contents.
	virtual void begin() = 0;
	//! Restore the warp after drawing.
	virtual void end() = 0;

	//! Draws a warped texture.
	void draw( const ci::gl::Texture2dRef &texture );
	//! Draws a specific area of a warped texture.
	void draw( const ci::gl::Texture2dRef &texture, const ci::Area &srcArea );
	//! Draws a specific area of a warped texture to a specific region.
	virtual void draw( const ci::gl::Texture2dRef &texture, const ci::Area &srcArea, const ci::Rectf &destRect ) = 0;

	//! Adjusts both the source area and destination rectangle so that they are clipped against the warp's content.
	bool clip( ci::Area &srcArea, ci::Rectf &destRect ) const;

	//! Returns the coordinates of the specified control point.
	virtual ci::vec2 getControlPoint( unsigned index ) const;
	//! Sets the coordinates of the specified control point.
	virtual void setControlPoint( unsigned index, const ci::vec2 &pos );
	//! Moves the specified control point.
	virtual void moveControlPoint( unsigned index, const ci::vec2 &shift );
	//! Get the number of control points.
	virtual size_t getNumControlPoints() const { return mPoints.size(); }
	//! Get the index of the currently selected control point.
	virtual unsigned int getSelectedControlPoint() const { return mSelected; }
	//! Select one of the control points.
	virtual void selectControlPoint( unsigned index );
	//! Deselect the selected control point.
	virtual void deselectControlPoint();
	//! Returns the index of the closest control point, as well as the distance in pixels.
	virtual unsigned findControlPoint( const ci::vec2 &pos, float *distance ) const;

	//! Set the width and height in pixels of the content of all warps.
	static void setSize( const WarpList &warps, int w, int h );
	//! Set the width and height in pixels of the content of all warps.
	static void setSize( const WarpList &warps, const ci::ivec2 &size ) { setSize( warps, size.x, size.y ); }
	//! Checks all warps and selects the closest control point.
	static void selectClosestControlPoint( const WarpList &warps, const ci::ivec2 &position );

	//! Draw a control point in the correct preset color.
	void queueControlPoint( const ci::vec2 &pt, bool selected = false, bool attached = false );
	//! Draw a control point in the specified color.
	void queueControlPoint( const ci::vec2 &pt, const ci::Color &clr, float scale = 1.0f );

	//! Read a settings xml file and pass back a vector of Warps.
	static WarpList readSettings( const ci::DataSourceRef &source );
	//! Write a settings xml file.
	static void writeSettings( const WarpList &warps, const ci::DataTargetRef &target );

	//! Handles mouseMove events for multiple warps.
	static bool handleMouseMove( WarpList &warps, ci::app::MouseEvent &event );
	//! Handles mouseDown events for multiple warps.
	static bool handleMouseDown( WarpList &warps, ci::app::MouseEvent &event );
	//! Handles mouseDrag events for multiple warps.
	static bool handleMouseDrag( WarpList &warps, ci::app::MouseEvent &event );
	//! Handles mouseUp events for multiple warps.
	static bool handleMouseUp( WarpList &warps, ci::app::MouseEvent &event );

	//! Handles keyDown events for multiple warps.
	static bool handleKeyDown( WarpList &warps, ci::app::KeyEvent &event );
	//! Handles keyUp events for multiple warps.
	static bool handleKeyUp( WarpList &warps, ci::app::KeyEvent &event );

	//! Handles resize events for multiple warps.
	static bool handleResize( WarpList &warps );
	static bool handleResize( WarpList &warps, const ci::ivec2 &size );

	virtual void mouseMove( ci::app::MouseEvent &event );
	virtual void mouseDown( ci::app::MouseEvent &event );
	virtual void mouseDrag( ci::app::MouseEvent &event );
	virtual void mouseUp( ci::app::MouseEvent &event );

	virtual void keyDown( ci::app::KeyEvent &event );
	virtual void keyUp( ci::app::KeyEvent &event ) {}

	virtual void resize();
	virtual void resize( const ci::ivec2 &size );

  protected:
	//! Draw the warp and its editing interface.
	virtual void draw( bool controls = true ) = 0;
	//! Draw the control points.
	void drawControlPoints();

  protected:
	WarpType mType;

	bool     mIsDirty;
	int      mWidth;
	int      mHeight;
	ci::vec2 mWindowSize;
	float    mBrightness;
	unsigned mSelected;

	//! Determines the number of horizontal and vertical control points.
	int mControlsX;
	int mControlsY;

	std::vector<ci::vec2> mPoints;

	//! Edge blending parameters.
	ci::vec3 mLuminance;
	ci::vec3 mGamma;
	ci::vec4 mEdges;
	float    mExponent;

	//! Time of last control point selection.
	double mSelectedTime;
	//! Keep track of mouse position.
	mutable ci::ivec2 mMouse;

	static const int MAX_NUM_CONTROL_POINTS = 1024;

  private:
	ci::vec2 mOffset;

	//! Instanced control points.
	typedef struct Data {
		ci::vec2 position;
		float    scale;
		float    reserved;
		ci::vec4 color;

		Data() {}
		Data( const ci::vec2 &pt, const ci::vec4 &clr, float scale )
		    : position( pt )
		    , scale( scale )
		    , color( clr )
		{
		}
	} Data;

	std::vector<Data> mControlPoints;

	ci::gl::VboRef   mInstanceDataVbo;
	ci::gl::BatchRef mInstancedBatch;

	//! Edit mode for all warps.
	static std::atomic<bool> sIsEditMode;
	//! Gamma mode for all warps.
	static std::atomic<bool> sIsGammaMode;
};

// ----------------------------------------------------------------------------------------------------------------

typedef std::shared_ptr<class WarpBilinear> WarpBilinearRef;

class WarpBilinear : public Warp {
  public:
	static WarpBilinearRef create( const ci::gl::Fbo::Format &format = ci::gl::Fbo::Format() ) { return std::make_shared<WarpBilinear>( format ); }

	WarpBilinear( const ci::gl::Fbo::Format &format = ci::gl::Fbo::Format() );
	virtual ~WarpBilinear() {}

	//! Returns a shared pointer to this warp.
	WarpBilinearRef getPtr() { return std::static_pointer_cast<WarpBilinear>( shared_from_this() ); }
	//!
	virtual ci::XmlTree toXml() const override;
	//!
	virtual void fromXml( const ci::XmlTree &xml ) override;

	//! Set the width and height of the content in pixels.
	void setSize( int w, int h ) override
	{
		Warp::setSize( w, h );
		mFbo.reset();
	}
	//! Set the frame buffer format, so you have control over its quality settings.
	void setFormat( const ci::gl::Fbo::Format &format )
	{
		mFboFormat = format;
		mFbo.reset();
	}
	//!
	void setLinear( bool enabled = true )
	{
		mIsLinear = enabled;
		mIsDirty = true;
	};
	//!
	void setCurved( bool enabled = true )
	{
		mIsLinear = !enabled;
		mIsDirty = true;
	};

	//! Reset control points to undistorted image.
	virtual void reset() override;
	//! Setup the warp before drawing its contents.
	virtual void begin() override;
	//! Restore the warp after drawing.
	virtual void end() override;

	//! Draws a warped texture.
	virtual void draw( const ci::gl::Texture2dRef &texture, const ci::Area &srcArea, const ci::Rectf &destRect ) override;

	//! Set the number of horizontal control points for this warp.
	void setNumControlX( int n );
	//! Set the number of vertical control points for this warp.
	void setNumControlY( int n );
	//!
	void setTexCoords( float x1, float y1, float x2, float y2 );

	virtual void keyDown( ci::app::KeyEvent &event ) override;

  protected:
	//! Draws the warp as a mesh, allowing you to use your own texture instead of the FBO.
	virtual void draw( bool controls = true ) override;
	//! Creates the shader that renders the content with a wireframe overlay.
	void createShader();
	//! Creates the frame buffer object and updates the vertex buffer object if necessary.
	void createBuffers();
	//! Creates the vertex buffer object.
	void createMesh( int resolutionX = 36, int resolutionY = 36 );
	//! Updates the vertex buffer object based on the control points.
	void updateMesh();
	//!	Returns the specified control point. Values for col and row are clamped to prevent errors.
	ci::vec2 getPoint( int col, int row ) const;
	//! Performs fast Catmull-Rom interpolation, returns the interpolated value at t.
	ci::vec2 cubicInterpolate( const std::vector<ci::vec2> &knots, float t ) const;
	//!
	ci::Rectf getMeshBounds() const;

  private:
	//! Greatest common divisor using Euclidian algorithm (from: http://en.wikipedia.org/wiki/Greatest_common_divisor)
	inline int gcd( int a, int b ) const
	{
		if( b == 0 )
			return a;
		else
			return gcd( b, a % b );
	};

  protected:
	ci::gl::FboRef      mFbo;
	ci::gl::Fbo::Format mFboFormat;
	ci::gl::VboMeshRef  mVboMesh;
	ci::gl::GlslProgRef mShader2D;
	ci::gl::GlslProgRef mShader2DRect;
	ci::gl::BatchRef    mBatch2D;
	ci::gl::BatchRef    mBatch2DRect;
	GLenum              mTarget;

	//! Linear or curved interpolation.
	bool mIsLinear;
	//!
	bool mIsAdaptive;

	//! Texture coordinates of corners.
	float mX1, mY1, mX2, mY2;

	//! Determines the detail of the generated mesh. Multiples of 5 seem to work best.
	int mResolution;

	//! Determines the number of horizontal and vertical quads.
	int mResolutionX;
	int mResolutionY;
};

// ----------------------------------------------------------------------------------------------------------------

typedef std::shared_ptr<class WarpPerspective> WarpPerspectiveRef;

class WarpPerspective : public Warp {
  public:
	//!
	static WarpPerspectiveRef create() { return std::make_shared<WarpPerspective>(); }

	WarpPerspective( void );
	virtual ~WarpPerspective() {}

	//! Returns a shared pointer to this warp.
	WarpPerspectiveRef getPtr() { return std::static_pointer_cast<WarpPerspective>( shared_from_this() ); }
	//! Get the transformation matrix.
	ci::mat4 getTransform();
	//! Get the inverted transformation matrix.
	ci::mat4 getInvertedTransform() { return mInverted; }
	//! Reset control points to undistorted image.
	void reset() override;
	//! Setup the warp before drawing its contents.
	void begin() override;
	//! Restore the warp after drawing.
	void end() override;

	//! Draws a warped texture.
	void draw( const ci::gl::Texture2dRef &texture, const ci::Area &srcArea, const ci::Rectf &destRect ) override;

	//! Override keyDown method to add additional key handling.
	void keyDown( ci::app::KeyEvent &event ) override;

	//! Allow WarpPerspectiveBilinear to access the protected class members.
	friend class WarpPerspectiveBilinear;

  protected:
	//!
	void draw( bool controls = true ) override;

	//! Find homography based on source and destination quad.
	ci::mat4 getPerspectiveTransform( const ci::vec2 src[4], const ci::vec2 dst[4] ) const;
	//! Helper function.
	void gaussianElimination( float *input, int n ) const;

	//!
	void createShader();

  protected:
	ci::vec2 mSource[4];
	ci::vec2 mDestination[4];

	ci::mat4 mTransform;
	ci::mat4 mInverted;

	ci::gl::GlslProgRef mShader2D;
	ci::gl::GlslProgRef mShader2DRect;
};

// ----------------------------------------------------------------------------------------------------------------

typedef std::shared_ptr<class WarpPerspectiveBilinear> WarpPerspectiveBilinearRef;

class WarpPerspectiveBilinear : public WarpBilinear {
  public:
	//!
	static WarpPerspectiveBilinearRef create( const ci::gl::Fbo::Format &format = ci::gl::Fbo::Format() ) { return std::make_shared<WarpPerspectiveBilinear>( format ); }

	WarpPerspectiveBilinear( const ci::gl::Fbo::Format &format = ci::gl::Fbo::Format() );
	virtual ~WarpPerspectiveBilinear( void ) {}

	//! Returns a shared pointer to this warp.
	WarpPerspectiveBilinearRef getPtr() { return std::static_pointer_cast<WarpPerspectiveBilinear>( shared_from_this() ); }
	//!
	ci::XmlTree toXml() const override;
	//!
	void fromXml( const ci::XmlTree &xml ) override;
	void mouseMove( ci::app::MouseEvent &event ) override;
	void mouseDown( ci::app::MouseEvent &event ) override;
	void mouseDrag( ci::app::MouseEvent &event ) override;

	void keyDown( ci::app::KeyEvent &event ) override;

	void resize() override;

	//! Set the width and height of the content in pixels.
	void setSize( int w, int h ) override;

	//! Returns the coordinates of the specified control point.
	ci::vec2 getControlPoint( unsigned index ) const override;
	//! Sets the coordinates of the specified control point.
	void setControlPoint( unsigned index, const ci::vec2 &pos ) override;
	//! Moves the specified control point.
	void moveControlPoint( unsigned index, const ci::vec2 &shift ) override;
	//! Select one of the control points.
	void selectControlPoint( unsigned index ) override;
	//! Deselect the selected control point.
	void deselectControlPoint() override;

  protected:
	//!
	void draw( bool controls = true ) override;

	//! Returns whether or not the control point is one of the 4 corners and should be treated as a perspective control point.
	bool isCorner( unsigned index ) const;
	//! Converts the control point index to the appropriate perspective warp index.
	unsigned convertIndex( unsigned index ) const;

  protected:
	WarpPerspectiveRef mWarp;
};
}
} // namespace ph::warping
