#pragma once
#ifndef ROBINLE_LIB_GUI_IFONT
#define ROBINLE_LIB_GUI_IFONT





#include "BaseTypes.hpp"



namespace rl
{
	namespace GUI
	{

		class IFont
		{
		public: // methods

			IFont(Color cl, char32_t chFallback = U'?');
			virtual ~IFont() = default;

			inline char32_t getFallbackChar() const noexcept { return m_chFallback; }

			Color getColor() const noexcept { return m_cl; }
			void setColor(Color cl) noexcept;

			void drawText(const wchar_t* szText, PxPos iX, PxPos iY) const noexcept;
			void drawText(const char* szText, PxPos iX, PxPos iY) const noexcept;


		public: // pure virtual methods

			virtual PxSize getCharHeight() const = 0;

			virtual bool containsChar(char32_t ch) const = 0;
			virtual PxSize getCharWidth(char32_t ch) const = 0;


		protected: // pure virtual methods

			virtual void drawChar(char32_t ch, PxPos iX, PxPos iY) const = 0;


		protected: // methods

			// called on color change
			virtual void onSetColor(Color cl) noexcept {}

			virtual PxPos getDistanceBetweenChars(char32_t chLeft, char32_t chRight) const
			{
				return 0;
			}


		private: // methods

			void checkFallback() const;


		private: // variables

			const char32_t m_chFallback;
			Color m_cl;

		};

	}
}





#endif // ROBINLE_LIB_GUI_IFONT