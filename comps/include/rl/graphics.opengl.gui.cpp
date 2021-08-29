#include "graphics.opengl.gui.hpp"

#include <exception>
#include <functional>
#include <stdint.h>





namespace rl
{

	/***********************************************************************************************
	 class IGLControlBase
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	IGLControlBase::IGLControlBase(int32_t left, int32_t top, int32_t width, int32_t height,
		bool transparent)
	{
		checkSize(width, height);


		m_iLeft = left;
		m_iTop = top;
		m_iWidth = width;
		m_iHeight = height;

		m_bTransparent = transparent;

		OpenGLTexture_Config cfg;
		m_oCanvas.create(width, height, cfg);
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	void IGLControlBase::checkSize(int32_t width, int32_t height)
	{
		if (width < 0 || height < 0)
			throw std::exception("rl::IGLControlBase: Invalid size");
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	inline void IGLControlBase::invalidate(bool recursive)
	{
		m_bDirty = true;

		if (recursive)
		{
			// todo: find solution for invalidation of child controls
		}
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods










	/***********************************************************************************************
	 class IGLControl
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

#define REPAINT(prio)												\
		for (auto p : m_oChildren)									\
			if (p->getPriority() == GLCanvasPriority::prio)			\
				p->repaint(true)
	void IGLControl::repaint(bool RepaintChildren)
	{
		if (RepaintChildren)
		{
			REPAINT(Background);
			REPAINT(Default);
			REPAINT(Foreground);
			REPAINT(Top);
		}

		callRedraw();
		m_pParent->getCanvas().draw(getCanvas(), m_iLeft, m_iTop, m_bTransparent);
	}
#undef REPAINT





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods










	/***********************************************************************************************
	 class GLScreen
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void GLScreen::update(bool force)
	{
		if (force)
		{
			for (auto p : m_oChildren)
				if (p->getPriority() == GLCanvasPriority::Background)
					p->repaint(true);
			for (auto p : m_oChildren)
				if (p->getPriority() == GLCanvasPriority::Default)
					p->repaint(true);
			for (auto p : m_oChildren)
				if (p->getPriority() == GLCanvasPriority::Foreground)
					p->repaint(true);
			for (auto p : m_oChildren)
				if (p->getPriority() == GLCanvasPriority::Top)
					p->repaint(true);
		}
		else
		{
			std::function<void(IGLControl*)> fn = [&](IGLControl* p) -> void
			{
				for (auto p : p->getChildren())
					fn(p);

				if (p->isDirty())
					p->repaint();
			};

			// todo: Priosisierungs-System umstellen
		}
	}





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	void GLScreen::redraw()
	{

	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods










	/***********************************************************************************************
	 class
	***********************************************************************************************/

	//==============================================================================================
	// STATIC VARIABLES

	// static variables





	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	// constructors, destructors





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	// static methods





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	// public methods





	//----------------------------------------------------------------------------------------------
	// PROTECTED METHODS

	// protected methods





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	// private methods

}