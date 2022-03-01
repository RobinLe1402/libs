/***************************************************************************************************
 FILE:	graphics.opengl.gui.hpp
 CPP:	graphics.opengl.gui.cpp
		graphics.opengl.texture.cpp
		graphics.opengl.window.cpp
 DESCR:	The basic framework for an OpenGL-based GUI system
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_GRAPHICS_OPENGL_GUI
#define ROBINLE_GRAPHICS_OPENGL_GUI





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef int int32_t;


#include <vector>

#include "graphics.opengl.texture.hpp"
#include "graphics.opengl.window.hpp"



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// Signed 32-Bit integer coordinates
	/// </summary>
	struct Coord32 { int32_t x, y; };

	/// <summary>
	/// Signed 32-Bit integer rectangle
	/// </summary>
	struct Rect32 { int32_t left, top, width, height; };

	/// <summary>
	/// Priority for the canvas of an OpenGL control
	/// </summary>
	enum class GLCanvasPriority
	{
		/// <summary>
		/// An object of highest priority, like a text hint.<para/>
		/// Should only be used by very few special controls.
		/// </summary>
		Top,
		/// <summary>
		/// An object in front of the regular controls, like a popup or dropdown menu.<para/>
		/// Should be used less often than the default priority.
		/// </summary>
		Foreground,
		/// <summary>
		/// A normal priority object. All regular controls should have this priority.
		/// </summary>
		Default,
		/// <summary>
		/// A background object, behind the regular controls.<param/>
		/// Should be used by objects that can never be interacted with
		/// </summary>
		Background
	};





	// forward declaration
	class IGLControl;

	/// <summary>
	/// The base interface for all OpenGL controls (both the screen and actual controls)
	/// </summary>
	class IGLControlBase
	{
	protected: // interface methods

		/// <summary>
		/// Redraw the canvas content
		/// </summary>
		virtual void redraw() = 0;


	public: // methods

		IGLControlBase(int32_t left, int32_t top, int32_t width, int32_t height, bool transparent);

		inline OpenGLTexture& getCanvas() { return m_oCanvas; }
		inline const OpenGLTexture& getCanvas() const { return m_oCanvas; }

		inline bool getTransparent() { return m_bTransparent; }
		// setTransparent() is to be implemented by derived classes, if wanted

		inline const std::vector<IGLControl*>& getChildren() const { return m_oChildren; }
		// non-const version of getChildren() is to be implemented by derived classes, if wanted

		/// <summary>
		/// Mark this control's canvas as dirty to force a redraw on next screen update
		/// </summary>
		inline void invalidate(bool recursive = false);


	protected: // methods

		/// <summary>
		/// Check if size data is valid<para/>
		/// Throws a <c>std::exception</c> if width or height is smaller than zero
		/// </summary>
		static void checkSize(int32_t width, int32_t height);


	protected: // variables

		OpenGLTexture m_oCanvas;
		int32_t m_iLeft = 0;
		int32_t m_iTop = 0;
		int32_t m_iWidth = 0;
		int32_t m_iHeight = 0;

		bool m_bTransparent = false;
		bool m_bDirty = true;
		std::vector<IGLControl*> m_oChildren;

	};





	/// <summary>
	/// An optical component for an OpenGL-based GUI
	/// </summary>
	class IGLControl : public IGLControlBase
	{
	public: // methods

		IGLControl(IGLControl* parent, uint32_t left, uint32_t top, uint32_t width, uint32_t height,
			bool transparent, GLCanvasPriority priority = GLCanvasPriority::Default) :
			IGLControlBase(left, top, width, height, transparent),
			m_pParent(parent),
			m_ePriority(priority)
		{}

		/// <summary>
		/// Force a repaint of this control to the parent control
		/// </summary>
		/// <param name="RepaintChildren">= should all child controls also be repainted?</param>
		void repaint(bool RepaintChildren = false);


		inline bool getVisible() const { return m_bVisible; }
		void setVisible(bool val) { m_bVisible = val; }

		inline GLCanvasPriority getPriority() { return m_ePriority; }

		inline bool getTabStop() const { return m_bTabStop; }
		// setTabStop() is to be implemented by derived classes, if wanted

		inline bool isDirty() const { return m_bDirty; }


	protected: // methods

		// wrapper around redraw() that sets m_bDirty to false
		inline void callRedraw()
		{
			m_bDirty = false;
			redraw();
		}


	protected: // variables

		IGLControl* m_pParent;
		const GLCanvasPriority m_ePriority;
		bool m_bVisible = true;
		bool m_bDirty = true; // must the texture be redrawn?
		bool m_bTabStop = true;

	};





	/// <summary>
	/// The screen for OpenGL GUI controls
	/// </summary>
	class GLScreen : public IGLControlBase
	{
	protected: // IGLControlBase implementation

		void redraw() override;


	public: // methods

		GLScreen(int32_t width, int32_t height, Pixel bgcolor) :
			IGLControlBase(0, 0, width, height, false), m_oBackgroundColor(bgcolor)
		{}


		/// <summary>
		/// Update the screen
		/// </summary>
		/// <param name="force">= should all controls be redrawn, ignoring the dirtyness?</param>
		void update(bool force);


	private: // methods

	private: // variables

		Pixel m_oBackgroundColor;
	};

}





// #undef foward declared definitions

#endif // ROBINLE_GRAPHICS_OPENGL_GUI