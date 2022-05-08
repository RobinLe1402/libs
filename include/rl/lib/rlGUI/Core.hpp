/***************************************************************************************************
 FILE:	lib/rlGUI/Core.hpp
 LIB:	rlGUI.lib
 DESCR:	The basic framework for a OpenGL-based window with different controls
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_GUI__CORE
#define ROBINLE_LIB_GUI__CORE





//==================================================================================================
// INCLUDES

#include <string>
#include <vector>
#include <Windows.h>



//==================================================================================================
// DECLARATION
namespace rl
{
	namespace GLGUI
	{
		
		
		
		using ControlType = unsigned;

		constexpr ControlType CustomControlTypesStart = 0x10000000;
		namespace CustomControlTypes
		{
			// use this namespace for your custom control types
		}

		struct IControlGraphNode
		{
			const ControlType iType;
			int iX, iY;
			unsigned iWidth, iHeight;
			bool bEnabled;

			IControlGraphNode(ControlType iType, int iX, int iY,
				unsigned iWidth, unsigned iHeight, bool bEnabled)
				:
				iType(iType), iX(iX), iY(iY), iWidth(iWidth), iHeight(iHeight), bEnabled(bEnabled)
			{}

			virtual IControlGraphNode* clone() const = 0;
		};

		using GUIGraph = std::vector<const IControlGraphNode*>;





		class IControl
		{
		public: // methods

			IControl(ControlType iType, bool bVisible, int iX, int iY,
				unsigned iWidth, unsigned iHeight, bool bEnabled) :
				m_iType(iType), m_bVisible(bVisible), m_iX(iX), m_iY(iY),
				m_iWidth(iWidth), m_iHeight(iHeight), m_bEnabled(bEnabled) {}
			virtual ~IControl() = default;

			virtual void addToGraph(GUIGraph& graph) const = 0;

			ControlType getType() { return m_iType; }

			bool isVisible() const { return m_bVisible; }
			int getX() const { return m_iX; }
			int getY() const { return m_iY; }
			int getWidth() const { return m_iWidth; }
			int getHeight() const { return m_iHeight; }
			bool isEnabled() const { return m_bEnabled; }

			virtual void OnParentResize(unsigned iWidth, unsigned iHeight) {}


		protected: // variables

			const ControlType m_iType;
			bool m_bVisible;
			int m_iX, m_iY;
			unsigned m_iWidth, m_iHeight;
			bool m_bEnabled;

		};





		class PrivateImpl;

		struct GUIConfig
		{
			unsigned iMinWidth = 0, iMaxWidth = 0;
			unsigned iMinHeight = 0, iMaxHeight = 0;
			std::wstring sTitle;
			unsigned iWidth = 500, iHeight = 300;
		};

		class GUIWindow
		{
			friend class PrivateImpl;

		public: // methods

			// ToDo:
			virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) { return false; }


		private: // variables

			PrivateImpl* m_pPImpl;

		};

		class GUIRenderer
		{
			friend class PrivateImpl;

		public: // methods

			virtual void OnCreate() {}
			virtual void OnResize(unsigned iWidth, unsigned iHeight) {}
			virtual void OnDestroy() {}

			virtual bool render(const GUIGraph& oGraph);


		protected: // methods

			unsigned getWidth();
			unsigned getHeight();


		private: // variables

			PrivateImpl* m_pPImpl;

		};

		class IGUIApplication
		{
		public: // methods

			IGUIApplication(GUIWindow& oWindow, GUIRenderer& oRenderer);
			IGUIApplication(const IGUIApplication& other) = delete;
			IGUIApplication(IGUIApplication&& rval) = delete;
			virtual ~IGUIApplication();

			bool execute(const GUIConfig& config);

			virtual bool OnCreate() { return true; }
			virtual bool OnUpdate(float fElapsedTime);
			virtual bool OnDestroy() { return true; }
			virtual void OnResize(unsigned iWidth, unsigned iHeight);


		protected: // methods

			auto& window() { return m_oWindow; }
			auto& renderer() { return m_oRenderer; }
			unsigned getWidth() const;
			unsigned getHeight() const;
			GUIGraph& getGraph();


		protected: // variables

			std::vector<IControl*> m_oControls;


		private: // variables

			GUIWindow& m_oWindow;
			GUIRenderer& m_oRenderer;
			PrivateImpl* m_pPrivateImpl;

		};



	}
}





#endif // ROBINLE_LIB_GUI__CORE