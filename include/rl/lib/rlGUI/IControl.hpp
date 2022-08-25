#pragma once
#ifndef ROBINLE_LIB_GUI_ICONTROL
#define ROBINLE_LIB_GUI_ICONTROL





#include "BaseTypes.hpp"
#include "Texture.hpp"

#include <vector>



namespace rl
{
	namespace GUI
	{

		class IControl
		{
		public: // methods

			/// <param name="pOwner">
			/// The owner of the control. Deletes this control on destruction.
			/// Must be <c>nullptr</c> on stack objects.
			/// </param>
			IControl(IControl* pOwner, IControl* pParent,
				int iLeft, int iTop, unsigned iWidth, unsigned iHeight, Color clBackground);

			virtual ~IControl();

			auto getOwner() const { return m_pOwner; }
			auto getParent() const { return m_pParent; }
			const auto& getChildren() const { return m_oChildControls; }

			auto getLeft() const { return m_iX; }
			auto getTop() const { return m_iY; }
			auto getWidth() const { return m_iWidth; }
			auto getHeight() const { return m_iHeight; }
			auto getBackgroundColor() const { return m_oTexBG.getPixel(0, 0); }

			virtual void repaint();


		protected: // methods

			virtual void onPaint() {}

			void setBackgroundColor(Color cl) { m_oTexBG.setPixel(0, 0, cl); }
			GLCoord clientToScreen(GLCoord oClientCoord);


		protected: // variables

			const IControl* m_pOwner;
			IControl* m_pParent;
			std::vector<IControl*> m_oOwnedControls;
			std::vector<IControl*> m_oChildControls;

			int m_iX, m_iY;
			unsigned m_iWidth, m_iHeight;


		private: // variables

			Texture m_oTexBG = Texture(1, 1, TextureScalingMethod::NearestNeighbor);

		};

	}
}





#endif // ROBINLE_LIB_GUI_ICONTROL