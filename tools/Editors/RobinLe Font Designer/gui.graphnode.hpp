#pragma once
#ifndef ROBINLE_FONT_DESIGNER__GUI_GRAPHNODE
#define ROBINLE_FONT_DESIGNER__GUI_GRAPHNODE





#include <string>
#include <vector>



namespace GUI
{

	/// <summary>
	/// The type of a graph node<para/>
	/// When the documentation starts with "[custom]", the node is specifically designed for the
	/// RobinLe Font Designer.
	/// </summary>
	enum class GraphNodeType
	{
		Button, // fixed-height button with text on it
		CheckBox, // a checkbox
		TextBox, // fixed-height, single-line text box
		ComboBox, // fixed-height combobox
		StatusBar, // fixed-height area at the bottom of the window with text on it
		ToolBar, // a fixed-height area at the top of the window with icons buttons on it
		Tabs, // a collection of pages navigated by tabs
		ScrollBar, // a scrollbar
		CharList, // [custom] a list of all available characters
		Canvas, // [custom] a canvas for editing bitmap data
		PreviewBox // [custom] a preview of a bitmap font
	};



	struct IGraphNode
	{
		GraphNodeType eType;
		bool bVisible;
		bool bEnabled;
		bool bChanged;
		bool bResized;
		unsigned iX, iY;
		unsigned iWidth, iHeight;
	};



	struct ButtonNode : IGraphNode
	{
		std::string sCaption;
	};

	struct CheckBoxNode : IGraphNode
	{
		bool bChecked;
		std::string sCaption;
	};

	struct TextBoxNode : IGraphNode
	{
		std::string sText;
	};

	struct ComboBoxNode : IGraphNode
	{
		bool bExpanded;
		bool bCustomInput;
		std::string sText;
		std::vector<std::string> sItems;
		int iItemIndex;
	};

	struct StatusBarNode : IGraphNode
	{
		std::string sCaption;
	};

	struct ToolBarNode : IGraphNode
	{
		//std::vector<> // ToDo
	}

	// ToDo...

}





#endif // ROBINLE_FONT_DESIGNER__GUI_GRAPHNODE
