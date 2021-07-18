/***************************************************************************************************
 FILE:	rlfnt.hpp
 CPP:	rlfnt.cpp
 DESCR:	Template for new source files
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DEV_RLFNT
#define ROBINLE_DEV_RLFNT





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;


#include <map>
#include <string>



//==================================================================================================
// DECLARATION
namespace rl
{

#define RL_FNT_WEIGHT_UNDEFINED		0
#define RL_FNT_WEIGHT_THIN			10
#define RL_FNT_WEIGHT_EXTRALIGHT	20
#define RL_FNT_WEIGHT_LIGHT			30
#define RL_FNT_WEIGHT_REGULAR		40
#define RL_FNT_WEIGHT_MEDIUM		50
#define RL_FNT_WEIGHT_SEMIBOLD		60
#define RL_FNT_WEIGHT_BOLD			70
#define RL_FNT_WEIGHT_EXTRABOLD		80
#define RL_FNT_WEIGHT_BLACK			90

#define RL_FNT_FLAG_ITALIC			0b00000001
#define RL_FNT_FLAG_SERIFS			0b00000010





	/// <summary>
	/// Default names for bitmap font faces
	/// </summary>
	namespace BitmapFontWeightStrings
	{
		const char szUndefined[] = "";

		const char szThin[] = "Thin";
		const char szExtraLight[] = "Extra-Light";
		const char szLight[] = "Light";
		const char szRegular[] = "Regular";
		const char szMedium[] = "Medium";
		const char szSemiBold[] = "Semi-Bold";
		const char szBold[] = "Bold";
		const char szExtraBold[] = "Extra-Bold";
		const char szBlack[] = "Black";

		/// <summary>
		/// Get the default font face name by the weight of the face
		/// </summary>
		/// <param name="weight">
		/// = Either zero or a value between 10 and 99; values rounded down to the value of one of
		/// the <c>RL_FNT_WEIGHT_[...]</c> defines
		/// </param>
		/// <returns>
		/// If param <c>weight</c> is valid, a pointer to a <c>const char[]</c> containing the
		/// weight's default face name
		/// </returns>
		const char* GetString(uint8_t weight) noexcept;

	}

	/// <summary>
	/// A bitmap font<para/>
	/// Representing a <c>.rlFON</c> file
	/// </summary>
	class BitmapFont
	{
	public: // types

		// forward declaration
		class Face;



		/// <summary>
		/// A single bitmap character
		/// </summary>
		class Char
		{
			friend class Face;

		public: // operators

			Char();
			Char(const Char& other);
			Char(uint8_t BitsPerPixel, uint32_t width, uint32_t height);
			~Char();

			Char& operator=(const Char& other);
			bool operator==(const Char& other) const;


		public: //methods

			/// <summary>
			/// Size of the binary data of a character
			/// </summary>
			/// <returns>
			/// If the parameters are valid --> The required size, in bytes<para/>
			/// If there are faulty parameters --> zero
			/// </returns>
			static size_t DataSize(uint8_t BitsPerPixel, uint32_t width, uint32_t height);

			/// <summary>
			/// Create a bitmap char
			/// </summary>
			/// <returns>Did the method succeed?</returns>
			bool create(uint8_t BitsPerPixel, uint16_t width, uint16_t height);

			/// <summary>
			/// Clear this bitmap char's data
			/// </summary>
			void destroy();

			/// <summary>
			/// Does this character have data?
			/// </summary>
			bool hasData() const;

			/// <summary>
			/// Is the character empty (are all pixels set to zero)?<para/>
			/// Throws an <c>std::exception</c> if this character has no data
			/// </summary>
			bool isEmpty() const;

			/// <summary>
			/// Get the size, in bytes, of the binary data that can be obtained via <c>getData()</c>
			/// <para/>
			/// Throws an <c>std::exception</c> if this character has no data
			/// </summary>
			size_t getDataSize() const;

			/// <summary>
			/// Get this character's data
			/// </summary>
			/// <returns>
			/// If the character has data, a pointer to the data (size can be obtained via
			/// <c>getDataSize()</c>)<para />
			/// Else, <c>nullptr</c>
			/// </returns>
			const uint8_t* getData() const { return m_pData; }

			/// <summary>
			/// Get a single pixel<para/>
			/// Throws an <c>std::exception</c> if the position is invalid or this character has no
			/// data
			/// </summary>
			uint32_t getPixel(uint16_t x, uint16_t y) const;

			/// <summary>
			/// Set a single pixel<para/>
			/// Throws an <c>std::exception</c> if the position is invalid or this character has no
			/// data
			/// </summary>
			void setPixel(uint16_t x, uint16_t y, uint32_t val);

			/// <summary>
			/// Get this character's bits per pixel<para/>
			/// Returns zero if the character has no data
			/// </summary>
			inline uint8_t getBitsPerPixel() const { return m_iBitsPerPixel; }

			/// <summary>
			/// Get this character's width, in pixels<para/>
			/// Returns zero if the character has no data
			/// </summary>
			inline uint16_t getWidth() const { return m_iWidth; }

			/// <summary>
			/// Get this character's height, in pixels<para/>
			/// Returns zero if the character has no data
			/// </summary>
			inline uint16_t getHeight() const { return m_iHeight; }


		private: // methods

			/// <summary>
			/// Check if a position is valid<para/>
			/// Throws an <c>std::exception</c> if it is not
			/// </summary>
			void checkPos(uint16_t iX, uint16_t iY) const;

			/// <summary>
			/// Check if this character has data<para/>
			/// Throws an <c>std::exception</c> if not
			/// </summary>
			void checkData() const;


			/// <summary>
			/// Create a bitmap char and initialize it with raw data
			/// </summary>
			/// <returns>Did the method succeed?</returns>
			bool create(uint8_t iBitsPerPixel, uint16_t iWidth, uint16_t iHeight,
				const uint8_t* pData);


		private: // variables

			uint8_t* m_pData = nullptr;
			uint8_t m_iBitsPerPixel = 0;
			uint16_t m_iWidth = 0;
			uint16_t m_iHeight = 0;
		};



		/// <summary>
		/// A bitmap font face<para/>
		/// Representing a <c>.rlFNT</c> file
		/// </summary>
		class Face
		{
		public: // types

			using iterator = std::map<uint32_t, Char>::iterator;
			using const_iterator = std::map<uint32_t, Char>::const_iterator;

			using reverse_iterator = std::map<uint32_t, Char>::reverse_iterator;
			using const_reverse_iterator = std::map<uint32_t, Char>::const_reverse_iterator;

			/// <summary>
			/// Result of a file validation
			/// </summary>
			enum class FileStatus
			{
				OK,							// file is valid
				FileDoesntExist,			// file doesn't exist
				UnexpectedEOF,				// file was too small/information was cut off
				WrongMagicNo,				// the file didn't start with "rlFONTFACE"
				UnknownFiletypeVer,			// unknown filetype version
				InvalidBPP,					// invalid bits per pixel
				ZeroHeight,					// character height was set to zero
				InvalidWeight,				// invalid font weight
				UnknownFlags,				// unknown flag bits were set
				NotAscending,				// chars are not saved in ascending order
				DuplicateChar,				// char defined multiple times
				InvalidFallback,			// the fallback character isn't present
				InvalidOffset_Copyright,	// offset of the copyright string was invalid
				InvalidOffset_Family,		// offset to the font family string was invalid
				InvalidOffset_Face,			// offset to the font face string was invalid
			};


		public: // operators

			inline iterator begin() { return m_oChars.begin(); }
			inline const_iterator begin() const { return m_oChars.begin(); }

			inline iterator end() { return m_oChars.end(); }
			inline const_iterator end() const { return m_oChars.end(); }


			inline reverse_iterator rbegin() { return m_oChars.rbegin(); }
			inline const_reverse_iterator rbegin() const { return m_oChars.rbegin(); }

			inline reverse_iterator rend() { return m_oChars.rend(); }
			inline const_reverse_iterator rend() const { return m_oChars.rend(); }


			Face& operator=(const Face& other);


		public: // methods

			/// <summary>
			/// Validate a rlFNT file
			/// </summary>
			/// <returns>Is the file a valid FontFace file?</returns>
			static FileStatus validate(const wchar_t* szFilename);

			/// <summary>
			/// Create a bitmap font face
			/// </summary>
			/// <param name="fixedwidth">= fixed width. Zero if width is not fixed.</param>
			/// <param name="height">= height of all characters</param>
			/// <param name="iPoints">
			/// = point size the face looks best at. Can be zero --> Undefined
			/// </param>
			/// <param name="weight">= weight of the face. Either zero or between 10 and 99.</param>
			/// <param name="szFamilyName">= name of the font family</param>
			/// <param name="szFaceName">= name of the font face</param>
			/// <param name="szCopyright">= copyright note</param>
			/// <returns>Was the font face successfully created?</returns>
			bool create(uint16_t fixedwidth, uint16_t height, uint8_t BitsPerPixel,
				uint16_t iPoints = 0, uint8_t weight = RL_FNT_WEIGHT_REGULAR, uint8_t iFlags = 0,
				const char* szFamilyName = nullptr, const char* szFaceName = nullptr,
				const char* szCopyright = nullptr);

			/// <summary>
			/// Destroy the bitmap font face
			/// </summary>
			void destroy();

			/// <summary>
			/// Save the bitmap font face to a rlFNT file
			/// </summary>
			/// <returns>Was the file saved successfully?</returns>
			bool saveToFile(const wchar_t* szFilename) const;

			/// <summary>
			/// Load the bitmap font face from a rlFNT file
			/// </summary>
			/// <returns>Was the file loaded successfully?</returns>
			bool loadFromFile(const wchar_t* szFilename);
			


			/// <summary>
			/// Get the count of characters
			/// </summary>
			/// <returns>
			/// If the font face has no data: Zero<para/>
			/// Else: The count of characters defined
			/// </returns>
			inline size_t charCount() const { return m_oChars.size(); }


			/// <summary>
			/// Add/replace data for a character<para/>
			/// Throws an <c>std::exception</c> if there already is data for the codepoint or this
			/// font face has not been initialized
			/// </summary>
			/// <param name="codepoint">= raw unicode codepoint value</param>
			/// <param name="width">= width of the character. Ignored if mono-width.</param>
			/// <returns></returns>
			Char& add(uint32_t codepoint, uint16_t width);

			/// <summary>
			/// Add/replace data for a character<para/>
			/// Throws an <c>std::exception</c> if this font face has not been initialized or the
			/// given data is incompatible with this face
			/// </summary>
			void set(uint32_t codepoint, const Char& data);





			/// <summary>
			/// Set the fallback character<para/>
			/// Throws an <c>std::exception</c> if the given character is not set
			/// </summary>
			/// <param name="NewFallback">= raw unicode value of the new fallback character</param>
			void setFallback(uint32_t NewFallback);

			/// <summary>
			/// Get the fallback character
			/// </summary>
			/// <returns>
			/// If this font face has not been initialized: Zero<para/>
			/// Else: The raw unicode value of the fallback character
			/// </returns>
			uint32_t getFallback() const { return m_iFallback; }



			/// <summary>
			/// Get the fixed pixel width of the font face
			/// </summary>
			/// <returns>
			/// Zero if there is no fixed width or the font has not been initialized
			/// </returns>
			inline uint16_t getFixedWidth() const { return m_iFixedWidth; }



			/// <summary>
			/// Get the face's bits per pixel
			/// </summary>
			/// <returns>Zero if the face has not been initialized</returns>
			inline uint8_t getBitsPerPixel() const { return m_iBitsPerPixel; }



			/// <summary>
			/// Get the pixel height of the font face
			/// </summary>
			/// <returns>Zero if the font has not been initialized</returns>
			inline uint16_t getHeight() const { return m_iHeight; };



			/// <summary>
			/// Get the points value of the font
			/// </summary>
			/// <returns>Zero if the font face has not been initialized</returns
			inline uint32_t getPoints() const { return m_iPoints; }

			/// <summary>
			/// Set the points value of the font
			/// </summary>
			void setPoints(uint32_t points);



			/// <summary>
			/// Get the weight of the font<para/>
			/// Either zero or between 10 and 99 (see <c>RL_FNT_WEIGHT_[...]</c> defines)
			/// </summary>
			/// <returns></returns>
			inline uint8_t getWeight() const { return m_iWeight; }



			/// <summary>
			/// Get the face's version
			/// </summary>
			void getVersion(uint8_t(&dest)[4]) const;

			/// <summary>
			/// Set the face version<para/>
			/// Resulting version would be
			/// v<c>&lt;part1&gt;.&lt;part2&gt;.&lt;part3&gt;.&lt;part4&gt;</c>
			/// </summary>
			void setVersion(uint8_t part1, uint8_t part2 = 0, uint8_t part3 = 0, uint8_t part4 = 0);



			/// <summary>
			/// Get the face's flags<para/>
			/// Consists of <c>RL_FNT_FLAGS_[...]</c> defines
			/// </summary>
			/// <returns></returns>
			inline uint8_t getFlags() const { return m_iFlags; }

			/// <summary>
			/// Set the face's flags<para/>
			/// Consisting of (and only of) <c>RL_FNT_FLAGS_[...]</c> defines<para/>
			/// If there are unknown flags set, the method throws an <c>std::exception</c>
			/// </summary>
			/// <param name="flags"></param>
			void setFlags(uint8_t flags);



			/// <summary>
			/// Get the family name
			/// </summary>
			inline const std::string& getFamilyName() const { return m_sFamilyName; }

			/// <summary>
			/// Set the family name
			/// </summary>
			void setFamilyName(const char* szFamilyName);


			/// <summary>
			/// Get the face name
			/// </summary>
			inline const std::string& getFaceName() const { return m_sFaceName; }

			/// <summary>
			/// Set the face name
			/// </summary>
			void setFaceName(const char* szFaceName);


			/// <summary>
			/// Get the copyright text
			/// </summary>
			inline const std::string& getCopyright() const { return m_sCopyright; }

			/// <summary>
			///  Set the copyright text
			/// </summary>
			void setCopyright(const char* szCopyright);





			/// <summary>
			/// Remove a bitmap character
			/// </summary>
			/// <param name="codepoint">= raw unicode codepoint value</param>
			void remove(uint32_t codepoint);

			/// <summary>
			/// Does this bitmap font face contain a certain unicode codepoint?
			/// </summary>
			/// <param name="codepoint">= raw unicode codepoint value</param>
			bool containsChar(uint32_t codepoint) const;

			/// <summary>
			/// Get the data of a character<para/>
			/// Throws an <c>std::exception</c> if there is no data for this character or this font
			/// face has not been initialized
			/// </summary>
			/// <param name="codepage">= raw unicode codepoint value</param>
			const Char& getChar(uint32_t codepoint) const;


		private: // methods

			/// <summary>
			/// Check if this font face has been initialized<para/>
			/// Throws an <c>std::exception</c> if not
			/// </summary>
			void checkData() const;

			/// <summary>
			/// Check if there is data for a codepoint<para/>
			/// Throws an <c>std::exception</c> if there is
			/// </summary>
			void checkCharFree(uint32_t iCodepoint) const;

			/// <summary>
			/// Check if there is data for a codepoint<para/>
			/// Throws an <c>std::exception</c> if there is not
			/// </summary>
			void checkCharSet(uint32_t iCodepoint) const;


		private: // variables

			bool m_bData = false; // does this font have data (i.e. is the metadata set)?
			std::map<uint32_t, Char> m_oChars;
			uint32_t m_iFallback = 0;
			uint32_t m_iFixedWidth = 0; // zero if not fixed-width
			uint8_t m_iBitsPerPixel = 0;
			uint32_t m_iHeight = 0;
			uint16_t m_iPoints = 0; // zero if not of interest
			uint8_t m_iWeight = 0; // either zero or between 10 and 99
			uint8_t m_oFileVersion[4] = { 1, 0, 0, 0 };
			uint8_t m_iFlags = 0;

			std::string m_sFamilyName;
			std::string m_sFaceName;
			std::string m_sCopyright;

		};

	};

}





#endif // ROBINLE_DEV_RLFNT