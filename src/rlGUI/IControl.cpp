#include "rl/lib/rlGUI/IControl.hpp"

namespace rl
{
	namespace GUI
	{

		IControl::IControl(IControl* pOwner, IControl* pParent,
			int iLeft, int iTop, unsigned iWidth, unsigned iHeight, Color clBackground) :
			m_pOwner(pOwner), m_pParent(pParent),
			m_iX(iLeft), m_iY(iTop), m_iWidth(iWidth), m_iHeight(iHeight)
		{
			setBackgroundColor(clBackground);

			if (pOwner != nullptr)
				pOwner->m_oOwnedControls.push_back(this);

			if (pParent != nullptr)
				pParent->m_oChildControls.push_back(this);
		}

		IControl::~IControl()
		{
			for (auto p : m_oOwnedControls)
				delete p;
		}

		void IControl::repaint()
		{
			if (getBackgroundColor().a)
			{
				const GLCoord oTopLeft = clientToScreen({ 0, 0 });
				m_oTexBG.drawStretched(oTopLeft.X, oTopLeft.Y, m_iWidth, m_iHeight);
			}
			onPaint();

			for (auto pControl : m_oChildControls)
			{
				pControl->repaint();
			}
		}

		GLCoord IControl::clientToScreen(GLCoord oClientCoord)
		{
			if (m_pParent)
			{
				oClientCoord.X += m_iX;
				oClientCoord.Y += m_iY;
				return oClientCoord;
			}
			else
				return oClientCoord;
		}

	}
}