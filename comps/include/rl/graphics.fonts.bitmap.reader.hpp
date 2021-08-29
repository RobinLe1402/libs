/***************************************************************************************************
 FILE:	graphics.fonts.bitmap.reader.hpp
 CPP:	graphics.fonts.bitmap.reader.cpp
 DESCR:	Classes for reading rlFNT files (v2.0)
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_GRAPHICS_FONTS_BITMAP_READER
#define ROBINLE_GRAPHICS_FONTS_BITMAP_READER





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#ifdef _WIN64
typedef uint64_t size_t;
#else
typedef uint32_t size_t;
#endif

//--------------------------------------------------------------------------------------------------
// <Windows.h>
typedef const char* LPCSTR;
#define DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name
DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE HMODULE;



//==================================================================================================
// DECLARATION
namespace rl
{

#define RL_FNT_WEIGHT_THIN			100ui16
#define RL_FNT_WEIGHT_ULTRALIGHT	200ui16
#define RL_FNT_WEIGHT_LIGHT			300ui16
#define RL_FNT_WEIGHT_SEMILIGHT		350ui16
#define RL_FNT_WEIGHT_BOOK			380ui16
#define RL_FNT_WEIGHT_REGULAR		400ui16
#define RL_FNT_WEIGHT_MEDIUM		500ui16
#define RL_FNT_WEIGHT_SEMIBOLD		600ui16
#define RL_FNT_WEIGHT_BOLD			700ui16
#define RL_FNT_WEIGHT_ULTRABOLD		800ui16
#define RL_FNT_WEIGHT_BLACK			900ui16
#define RL_FNT_WEIGHT_ULTRABLACK	950ui16

#define RL_FNT_FLAG_MONOSPACED		0b0000000000000001ui16
#define RL_FNT_FLAG_PADDED			0b0000000000000010ui16
#define RL_FNT_FLAG_ITALIC			0b0000000000000100ui16
#define RL_FNT_FLAG_DIRECTCOLOR		0b0000000000010000ui16
#define RL_FNT_FLAG_ALPHA			0b0000000000100000ui16
#define RL_FNT_FLAG_NONASCII		0b0000000100000000ui16
#define RL_FNT_FLAG_SYMBOLS			0b0000001000000000ui16
#define RL_FNT_FLAG_EMOJI			0b0000010000000000ui16
#define RL_FNT_FLAG_PRIVATEUSEAREA	0b0000100000000000ui16
#define RL_FNT_FLAG_GENERICUSE		0b0001000000000000ui16
#define RL_FNT_FLAG_HIGHRES			0b0010000000000000ui16

#define RL_FNT_PADFLAG_LEFT			0b00000001ui8
#define RL_FNT_PADFLAG_TOP			0b00000010ui8
#define RL_FNT_PADFLAG_RIGHT		0b00000100ui8
#define RL_FNT_PADFLAG_BOTTOM		0b00001000ui8





	enum FontFaceBinaryFormat : uint8_t
	{
		BitPlanes = 0x00,
		FullBytes = 0x01,
		RGB = 0x02,
		RGBA = 0x03
	};

	enum FontFaceClassification : uint8_t
	{
		SansSerif = 0x00,
		Serif = 0x01,
		Script = 0x02,
		Decorative = 0x03
	};





	/// <summary>
	/// Header of a font face
	/// </summary>
	struct FontFaceHeader
	{
		/// <summary>
		/// Size of the data (from after header to end of data)
		/// </summary>
		uint32_t iDataSize;

		/// <summary>
		/// Count of characters defined in this font face
		/// </summary>
		uint32_t iCharCount;

		/// <summary>
		/// Character to use if a character is invalide/undefined
		/// </summary>
		uint32_t iFallbackChar;

		/// <summary>
		/// Offset to the null-terminated ASCII font family name string
		/// </summary>
		uint32_t iOffsetFontFamName;

		/// <summary>
		/// Offset to the null-terminated ASCII face name string
		/// </summary>
		uint32_t iOffsetFontFaceName;

		/// <summary>
		/// Offset to the null-terminated ASCII copyright string
		/// </summary>
		uint32_t iOffsetCopyright;

		/// <summary>
		/// Offset to the start of the character table
		/// </summary>
		uint32_t iOffsetCharTable;

		/// <summary>
		/// Count of bytes each column occupies
		/// </summary>
		uint32_t iBytesPerColumn;

		/// <summary>
		/// A value with a meaning dependent on <c>iBinaryFormat</c>
		/// </summary>
		uint32_t iFormatExtra;

		/// <summary>
		/// Zero if variable-width face, otherwise the width of every character
		/// </summary>
		uint16_t iGlobalCharWidth;

		/// <summary>
		/// Height of every character
		/// </summary>
		uint16_t iCharHeight;

		/// <summary>
		/// Weight of this font face
		/// </summary>
		uint16_t iWeight;

		/// <summary>
		/// Flags regarding look and character range of the face<para/>
		/// Consisting of "RL_FNT_FLAG_[...]" define values
		/// </summary>
		uint16_t iFlags;

		/// <summary>
		/// Count of bits each pixel consists of.<para/>
		/// Must be between 1 and 32.<para/>
		/// <para/>
		/// Data format depends on this value:<para/>
		/// * 8/16/24/32  => byte chains<para/>
		/// * else        => bit layers
		/// </summary>
		uint8_t iBitsPerPixel;

		/// <summary>
		/// Format of the binary data<para/>
		/// One of the "RL_FNT_FORMAT_[...]" define values
		/// </summary>
		uint8_t iBinaryFormat;

		/// <summary>
		/// Padding flags<para/>
		/// Consisting of the "RL_FNT_PADFLAG_[...]" define values
		/// </summary>
		uint8_t iPaddingFags;

		/// <summary>
		/// Optical classification of the font face<para/>
		/// One of the "RL_FNT_CLASS_[...]" define values
		/// </summary>
		uint8_t iClassification;

		/// <summary>
		/// Version of the font face
		/// </summary>
		uint8_t iFaceVersion[4];
	};


	/// <summary>
	/// Metadata of a font face character
	/// </summary>
	struct FontFaceCharInfo
	{
		/// <summary>
		/// Unicode codepoint of the character
		/// </summary>
		uint32_t iCodepoint;
		/// <summary>
		/// Size of the binary data, in bytes
		/// </summary>
		uint32_t iSize;
		/// <summary>
		/// Offset to the binary data
		/// </summary>
		uint32_t iOffset;
		/// <summary>
		/// Width of the character
		/// </summary>
		uint16_t iWidth;
	private:
		uint16_t unused1; // padding
	};


	/// <summary>
	/// Data of a font face (in-memory representation of a rlFNT file [v2.0])<para/>
	/// C-compatible struct
	/// </summary>
	struct FontFace
	{
		/// <summary>
		/// The binary data of the font face<para/>
		/// Must be deleted to free occupied memory
		/// </summary>
		uint8_t* pData;
		/// <summary>
		/// Pointer to the file header
		/// </summary>
		FontFaceHeader* pHeader;
		/// <summary>
		/// Pointer to the first character info<para/>
		/// Use like an array of size <c>pHeader-&gt;iCharCount</c>
		/// </summary>
		FontFaceCharInfo* pInfos;
	};





	/// <summary>
	/// Load a <c>FontFace</c> struct from a byte buffer
	/// </summary>
	/// <param name="dest">= a pointer to the destination struct</param>
	/// <param name="buf">
	/// = the byte buffer with the binary font face data (without overhead)
	/// </param>
	/// <param name="cb">= the size, in bytes, of the buffer pointed to by <c>buf</c></param>
	/// <returns>Did the function succeed?</returns>
	bool FontFaceCreate(FontFace* dest, const uint8_t* buf, size_t cb, const uint8_t(&typever)[2]);

	/// <summary>
	/// Load a <c>FontFace</c> struct from a byte buffer, including file overhead
	/// </summary>
	/// <param name="dest">= a pointer to the destination struct</param>
	/// <param name="buf">= the byte buffer with the binary font face data</param>
	/// <param name="cb">= the size, in bytes, of the buffer pointed to by <c>buf</c></param>
	/// <returns>Did the function succeed?</returns>
	bool FontFaceCreateWithOverhead(FontFace* dest, const uint8_t* buf, size_t cb);

	/// <summary>
	/// Copy data from one <c>FontFace</c> struct to another<para/>
	/// ~Both pointers must point to an existing <c>FontFace</c> struct<para/>
	/// ~<c>src</c> must hold valid data<para/>
	/// ~<c>dest</c> gets freed before it's being written to.
	/// </summary>
	void FontFaceCopy(FontFace* dest, const FontFace* src);

	/// <summary>
	/// Free the memory occupied by a font face<para/>
	/// (Only deletes associated data, not the struct itself)
	/// </summary>
	/// <param name="data">
	/// = a pointer to the font face which's data should be freed (cannot be a <c>nullptr</c>)
	/// </param>
	void FontFaceFree(FontFace* data);

	/// <summary>
	/// Get information about a character from a font face<para/>
	/// </summary>
	/// <param name="face">- must point to a valid font face</param>
	/// <returns><c>nullptr</c> if the function failed</returns>
	const FontFaceCharInfo* FontFaceFindChar(const FontFace* face, uint32_t ch);

	/// <summary>
	/// Get the value of a single pixel from a font face
	/// </summary>
	/// <param name="face">= the face to get pixel data from</param>
	/// <param name="ch">= the character to use</param>
	/// <param name="dest">= a pointer to the destination variable</param>
	/// <returns>Did the method succeed?</returns>
	bool FontFaceGetPixel(const FontFace* face, uint32_t ch, uint16_t x, uint16_t y,
		uint32_t* dest);










	/// <summary>
	/// Data of a font face (in-memory representation of a rlFNT file [v2.0])<para/>
	/// C++ wrapper around <c>FontFace</c> struct, for simplification
	/// </summary>
	class FontFaceClass
	{
	public: // types

		typedef const FontFaceCharInfo* const_iterator;


	public: // operators

		FontFaceClass& operator=(const FontFaceClass& other);
		FontFaceClass& operator=(FontFaceClass&& rval) noexcept;

		inline const_iterator begin() const { return m_oData.pInfos; }
		const_iterator end() const;


	public: // methods

		FontFaceClass() = default;
		FontFaceClass(const FontFace& other);
		FontFaceClass(const FontFaceClass& other);
		FontFaceClass(FontFaceClass&& rval) noexcept;
		~FontFaceClass();


		/// <summary>
		/// Clear all data of this fontface
		/// </summary>
		void clear();



		/// <summary>
		/// Load a font face from memory, including file overhead
		/// </summary>
		/// <param name="buf">= buffer containing the font face data</param>
		/// <param name="size">= size of the buffer pointed to by <c>buf</c></param>
		/// <returns>Did the method succeed?</returns>
		bool loadFromDataWithOverhead(const uint8_t* buf, size_t size);

		/// <summary>
		/// Load a font face from memory
		/// </summary>
		/// <param name="buf">= buffer containing the font face data</param>
		/// <param name="size">= size of the buffer pointed to by <c>buf</c></param>
		/// <param name="typever">= type version of the font face</param>
		/// <returns>Did the method succeed?</returns>
		bool loadFromData(const uint8_t* buf, size_t size, uint8_t(&typever)[2]);
		bool loadFromResource(HMODULE hModule, LPCSTR lpName);
		bool loadFromFile(const wchar_t* szFileName);

		/// <summary>
		/// Has a font face been loaded into this class?
		/// </summary>
		inline bool hasData() const { return m_oData.pData != nullptr; }


		const char* getFamilyName() const;
		const char* getFaceName() const;
		const char* getCopyright() const;

		uint32_t getCharCount() const;
		uint32_t getFallback() const;
		uint16_t getFixedWidth() const;
		uint16_t getHeight() const;
		uint16_t getWeight() const;
		uint16_t getFlags() const;
		uint8_t  getBitsPerPixel() const;
		void     getFaceVersion(uint8_t(&dest)[4]) const;

		/// <summary>
		/// Obtain the pointer to the character info of a certain codepoint
		/// </summary>
		/// <returns>
		/// If the function succeds, the return value is a pointer to the character info of the
		/// codepoint<para/>
		/// If the function fails, it returns <c>nullptr</c>
		/// </returns>
		const FontFaceCharInfo* findChar(uint32_t codepoint) const;

		uint16_t getCharWidth(uint32_t codepoint) const;
		uint32_t getPixel(uint32_t codepoint, uint16_t x, uint16_t y) const;


	private: // methods

		/// <summary>
		/// Throw an <c>std::exception</c> if this font face has no data
		/// </summary>
		void checkData() const;
		/// <summary>
		/// Throw an <c>std::exception</c> if a certain codepoint doesn't have any data
		/// </summary>
		/// <param name="codepoint"></param>
		const FontFaceCharInfo* checkChar(uint32_t codepoint) const;


	private: // variables

		FontFace m_oData = {};
	};

}





#undef DECLARE_HANDLE

#endif // ROBINLE_GRAPHICS_FONTS_BITMAP_READER