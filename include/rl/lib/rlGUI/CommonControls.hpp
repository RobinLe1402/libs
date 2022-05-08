/***************************************************************************************************
 FILE:	lib/rlGUI/CommonControls.hpp
 LIB:	rlGUI.lib
 DESCR:	The declarations for the most common control types
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_LIB_GUI__COMMONCONTROLS
#define ROBINLE_LIB_GUI__COMMONCONTROLS





#include "Core.hpp"
#include "rl/lib/rlOpenGL/Pixel.hpp"

namespace GL = rl::OpenGL;



namespace rl
{
	namespace GLGUI
	{





		namespace StdControlTypes
		{
			constexpr ControlType Panel = 1;
			constexpr ControlType Button = 2;
			constexpr ControlType Label = 3;
			constexpr ControlType Edit = 4;
			constexpr ControlType ComboBox = 5;
			constexpr ControlType CheckBox = 6;
			constexpr ControlType Image = 7;
			constexpr ControlType Menu = 8;
			constexpr ControlType BitmapMenu = 9;
			constexpr ControlType StatusBar = 10;
			constexpr ControlType ScrollBar = 11;
			constexpr ControlType Tabs = 12;
			// all remaining values up to CustomControlTypesStart are reserved for standard types.
		}



		struct PanelGraphNode : public IControlGraphNode
		{
			PanelGraphNode(int iX, int iY, unsigned iWidth, unsigned iHeight, GL::Pixel oColor) :
				IControlGraphNode::IControlGraphNode(
					StdControlTypes::Panel, iX, iY, iWidth, iHeight, true
				),
				oColor(oColor)
			{}
			PanelGraphNode(const PanelGraphNode& other) = default;
			~PanelGraphNode() = default;

			GL::Pixel oColor;

			IControlGraphNode* clone() const override;
		};
		
		class Panel : public IControl
		{
		public: // methods

			Panel(bool bVisible, int iX, int iY, unsigned iWidth, unsigned iHeight,
				GL::Pixel oColor)
				:
				IControl::IControl(StdControlTypes::Panel, bVisible, iX, iY, iWidth, iHeight, true),
				m_oColor(oColor)
			{}
			virtual ~Panel() = default;

			void addToGraph(GUIGraph& graph) const override;

			auto getColor() const { return m_oColor; }
			void setColor(GL::Pixel oColor) { m_oColor = oColor; }


		private: // variables

			GL::Pixel m_oColor;

		};





	}
}





#endif // ROBINLE_LIB_GUI__COMMONCONTROLS
