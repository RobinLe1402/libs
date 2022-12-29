#pragma once
#ifndef ROBINLE_FONT_PROJECT
#define ROBINLE_FONT_PROJECT





#if __cplusplus < 202004 // C++20 required for UNIX timestamp
	#error "A C++ standard >= C++20 is required for UNIX timestamps"
#endif

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>





namespace rl
{
	constexpr int64_t UNIXTimestamp_Now() noexcept
	{
		return std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	}

	namespace rlFNTPROJ
	{



		/// <summary>Default font weight values.</summary>
		namespace FontWeight
		{
			constexpr uint16_t
				Thin = 100,
				ExtraLight = 200,
				Light = 300,
				Regular = 400,
				Medium = 500,
				SemiBold = 600,
				Bold = 700,
				ExtraBold = 800,
				Black = 900;
		}

		/// <summary>Generic font types.</summary>
		enum class TypefaceClassification
		{
			/// <summary>Don't care or don't know.</summary>
			DontCare = 0,
			/// <summary>Font is proportional and uses serifs.</summary>
			Roman = 1,
			/// <summary><see cref="Roman"/></summary>
			Serif = Roman,
			/// <summary>Font is proportinal and doesn't use serifs.</summary>
			Swiss = 2,
			/// <summary><see cref="Swiss"/></summary>
			SansSerif = Swiss,
			/// <summary>Font is monowidth and might or might not use serifs.</summary>
			Modern = 3,
			/// <summary><see cref="Modern"/></summary>
			Mono = Modern,
			/// <summary>Font is designed to look like handwriting.</summary>
			Script = 4,
			/// <summary>Font is a novelty font.</summary>
			Decorative = 5
		};





		/// <summary>Metadata of a <c>Char</c>.</summary>
		struct CharMeta
		{
			uint16_t iBaseline = 0; // count of pixels from the top. 0 if unknown.
			uint16_t iPaddingLeft = 0;
			uint16_t iPaddingRight = 0;
		};

		/// <summary>A single bitmap character.</summary>
		class Char final
		{
		public: // methods

			Char(uint16_t iWidth, uint16_t iHeight, const CharMeta &oMetadata ={});
			Char() = default;
			Char(Char &&rval) = default;
			Char(const Char &other);

			~Char() = default;

			Char &operator=(Char &&rval) = default;
			Char &operator=(const Char &other);

			operator bool() const noexcept { return m_upData != nullptr; }



			void create(uint16_t iWidth, uint16_t iHeight);
			void clear() noexcept;



			auto getWidth() const noexcept { return m_iWidth; }
			auto getHeight() const noexcept { return m_iHeight; }

			const auto &getMetadata() const noexcept { return m_oMetadata; }
			void setMetadata(const CharMeta &oMetadata) noexcept { m_oMetadata = oMetadata; }

			bool getPixel(uint16_t iX, uint16_t iY) const;

			void setPixel(uint16_t iX, uint16_t iY, bool bNewVal);

			const uint8_t *getData() const noexcept { return m_upData.get(); }
			auto getDataSize() const noexcept { return m_iDataSize; }

			void setData(const uint8_t *pBuf, size_t lenBuf);



		private: // variables

			uint16_t m_iWidth = 0;
			uint16_t m_iHeight = 0;
			size_t m_iBytesPerRow = 0;
			size_t m_iDataSize = 0;
			std::unique_ptr<uint8_t[]> m_upData = nullptr;

			CharMeta m_oMetadata;
		};



		/// <summary>Metadata of a <c>FontFace</c>.</summary>
		struct FontFaceMeta
		{
			std::wstring sName;          // empty if unknown
			std::wstring sCopyright;     // empty if unknown
			char32_t cFallback = 0xFFFF; // 0xFFFF if unknown
			uint16_t iXHeight = 0;       // 0 if unknown
			uint16_t iCapHeight = 0;     // 0 if unknown
			uint16_t iPoints = 0;        // 0 if unknown
		};

		/// <summary>
		/// A font, rastered in a single pixel resolution.<para/>
		/// Example: <c>Arial Regular 8px</c>.
		/// </summary>
		class FontFace final
		{
		public: // methods

			FontFace(uint16_t iDefaultWidth, uint16_t iDefaultHeight,
				const CharMeta &oDefaultMetadata ={}, const FontFaceMeta &oMetadata ={});
			FontFace() = default;
			FontFace(const FontFace &other) = default;
			FontFace(FontFace &&rval) = default;
			~FontFace() = default;

			FontFace &operator=(const FontFace &other) = default;
			FontFace &operator=(FontFace &&rval) = default;

			void clear() noexcept;

			auto getDefaultCharWidth() const noexcept { return m_iDefaultCharWidth; }
			void setDefaultCharWidth(uint16_t iDefaultWidth) noexcept
			{
				m_iDefaultCharWidth = iDefaultWidth;
			}

			auto getDefaultCharHeight() const noexcept { return m_iDefaultCharHeight; }
			void setDefaultCharHeight(uint16_t iDefaultHeight) noexcept
			{
				m_iDefaultCharHeight = iDefaultHeight;
			}

			const auto &getDefaultCharMetadata() const noexcept
			{
				return m_oDefaultCharMetadata;
			}
			void setDefaultCharMetadata(const CharMeta &oDefaultMetadata) noexcept
			{
				m_oDefaultCharMetadata = oDefaultMetadata;
			}

			const auto &getMetadata() const noexcept { return m_oMetadata; }
			void setMetadata(const FontFaceMeta &oMetadata) noexcept { m_oMetadata = oMetadata; }

			const auto &getCharacters() const noexcept { return m_oChars; }
			auto &getCharacters() noexcept { return m_oChars; }



		private: // variables

			std::map<char32_t, Char> m_oChars;
			FontFaceMeta m_oMetadata;

			uint16_t m_iDefaultCharWidth = 0;
			uint16_t m_iDefaultCharHeight = 0;
			CharMeta m_oDefaultCharMetadata;

		};



		/// <summary>Metadata of a <c>Font</c>.</summary>
		struct FontMeta
		{
			std::wstring sName;
			std::wstring sCopyright;
			char32_t cFallback = 0xFFFF; // default for faces; 0xFFFF if unknown
			uint16_t iWeight = FontWeight::Regular;
			bool bItalic = false;
			bool bUnderline = false;
			bool bStrikeout = false;
		};

		/// <summary>
		/// A font in multiple pixel resolutions.<para/>
		/// Example: <c>Arial Regular</c>.
		/// </summary>
		class Font final
		{
		public: // methods

			Font(const FontMeta &oMetadata);
			Font() = default;
			Font(const Font &other) = default;
			Font(Font &&rval) = default;
			~Font() = default;

			Font &operator=(const Font &other) = default;
			Font &operator=(Font &&other) = default;

			void clear() noexcept;

			const auto &getMetadata() const noexcept { return m_oMetadata; }
			void setMetadata(const FontMeta &oMetadata) noexcept { m_oMetadata = oMetadata; }

			const std::vector<FontFace> &getFaces() const noexcept { return m_oFaces; }
			std::vector<FontFace> &getFaces() noexcept { return m_oFaces; }



		private: // variables

			FontMeta m_oMetadata;
			std::vector<FontFace> m_oFaces;

		};



		/// <summary>Metadata of a <c>FontFamily</c>.</summary>
		struct FontFamilyMeta
		{
			std::wstring sName;
			std::wstring sCopyright;
			char32_t cFallback = 0xFFFF; // default for fonts; 0xFFFF if unknown
			TypefaceClassification eType = TypefaceClassification::DontCare;
		};

		/// <summary>A collection of fonts with similar design.</summary>
		class FontFamily final
		{
		public: // methods

			FontFamily(const FontFamilyMeta &oMetadata);
			FontFamily() = default;
			FontFamily(const FontFamily &other) = default;
			FontFamily(FontFamily &&rval) = default;
			~FontFamily() = default;

			FontFamily &operator=(const FontFamily &other) = default;
			FontFamily &operator=(FontFamily &&rval) = default;

			void clear() noexcept;

			const auto &getFonts() const noexcept { return m_oFonts; }
			auto &getFonts() noexcept { return m_oFonts; }

			const auto &getMetdata() const noexcept { return m_oMeta; }
			void setMetadata(const FontFamilyMeta &oMetadata) noexcept { m_oMeta = oMetadata; }



		private: // variables

			std::vector<Font> m_oFonts;
			FontFamilyMeta m_oMeta;

		};





		/// <summary>Metadata of a <c>Project</c>.</summary>
		struct ProjectMeta
		{
			std::wstring sFoundry;
			std::wstring sDesigner;
			std::wstring sComments;
			int64_t unixtimeCreated = UNIXTimestamp_Now();
		};

		/// <summary>
		/// A RobinLe Font Workshop project containing a single font family as well as additional
		/// metadata.
		/// </summary>
		class Project final
		{
		public: // methods

			Project(const ProjectMeta &oMetadata, const FontFamilyMeta &oFFMeta);
			Project() = default;
			Project(const Project &other) = default;
			Project(Project &&rval) = default;
			~Project() = default;

			Project &operator=(const Project &other) = default;
			Project &operator=(Project &&rval) = default;


			void clear() noexcept;

			bool loadFromFile(const wchar_t *szFilename);

			bool saveToFile(const wchar_t *szFilename) const;

			const auto &getFontFamily() const { return m_oFontFamily; }
			auto &getFontFamily() { return m_oFontFamily; }

			const auto &getMetadata() const noexcept { return m_oMeta; }
			void setMetadata(const ProjectMeta &oMetadata) noexcept { m_oMeta = oMetadata; }



		private: // methods

			void setCreationUNIXTimestamp();

			bool loadFromFileV1(void *pIfstream, const void *pFileHeader);



		private: // variables

			FontFamily m_oFontFamily;
			ProjectMeta m_oMeta;

		};



	}
}





#endif // ROBINLE_FONT_PROJECT