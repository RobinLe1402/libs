#pragma once
#ifndef ROBINLE_RASTERFONTREADER_CODEPAGETOUNICODE
#define ROBINLE_RASTERFONTREADER_CODEPAGETOUNICODE



#include <cstdint>


/// <summary>Decode a codepoint value to a raw unicode value.</summary>
/// <param name="iCodePage">The codepage that <c>iCodePageID</c> is encoded in.</param>
/// <param name="iCodePageID">The encoded character.</param>
/// <param name="iDest">The destination for the decoded value.</param>
/// <returns>Did the decoding succeed?</returns>
bool CodePageToUnicode(uint16_t iCodePage, uint16_t iCodePageID, char32_t& iDest);



#endif // ROBINLE_RASTERFONTREADER_CODEPAGETOUNICODE