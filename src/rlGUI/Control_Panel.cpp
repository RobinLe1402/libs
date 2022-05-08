#include "rl/lib/rlGUI/CommonControls.hpp"

namespace lib = rl::GLGUI;



lib::IControlGraphNode* lib::PanelGraphNode::clone() const { return new PanelGraphNode(*this); }



void lib::Panel::addToGraph(GUIGraph& graph) const
{
	PanelGraphNode* pNode = new PanelGraphNode(m_iX, m_iY, m_iWidth, m_iHeight, m_oColor);
	graph.push_back(pNode);
}
