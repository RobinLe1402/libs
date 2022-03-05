/***************************************************************************************************
 FILE:	data.endian.hpp
 CPP:	n/a (header-only)
 DESCR:	Endian-related code
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DATA_ENDIAN
#define ROBINLE_DATA_ENDIAN





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <stdint.h>
using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned;
using uint64_t = unsigned long long;



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// Is the current system big endian?
	/// </summary>
	constexpr bool BigEndian = (const uint8_t&)0x00FF == 0x00;



	namespace Endian
	{
		
		
		constexpr uint16_t Swap(uint16_t x) noexcept
		{
			return
				((x & 0x00FF) << 8) |
				((x & 0xFF00) >> 8);
		}

		constexpr uint32_t Swap(uint32_t x) noexcept
		{
			return
				((x & 0x000000FF) << 24) |
				((x & 0x0000FF00) << 8) |
				((x & 0x00FF0000) >> 8) |
				((x & 0xFF000000) >> 24);
		}

		constexpr uint64_t Swap(uint64_t x) noexcept
		{
			return
				((x & 0x00000000000000FF) << 56) |
				((x & 0x000000000000FF00) << 40) |
				((x & 0x0000000000FF0000) << 24) |
				((x & 0x00000000FF000000) << 8) |
				((x & 0x000000FF00000000) >> 8) |
				((x & 0x0000FF0000000000) >> 24) |
				((x & 0x00FF000000000000) >> 40) |
				((x & 0xFF00000000000000) >> 56);
		}

	

		namespace Convert
		{

		
			/// <summary>
			/// Convert a value from host endian to little endian
			/// </summary>
#define DECLARE_HOST_TO_LE(bits)								\
			constexpr uint##bits##_t HostToLE(uint##bits##_t x)	\
			{													\
				if constexpr (BigEndian)						\
					return Swap(x);								\
				else											\
					return x;									\
			}
			
			DECLARE_HOST_TO_LE(16)
			DECLARE_HOST_TO_LE(32)
			DECLARE_HOST_TO_LE(64)

#undef DECLARE_HOST_TO_LE



			/// <summary>
			/// Convert a value from host endian to big endian
			/// </summary>
#define DECLARE_HOST_TO_BE(bits)								\
			constexpr uint##bits##_t HostToLE(uint##bits##_t x)	\
			{													\
				if constexpr (BigEndian)						\
					return x;									\
				else											\
					return Swap(x);								\
			}

			DECLARE_HOST_TO_BE(16)
			DECLARE_HOST_TO_BE(32)
			DECLARE_HOST_TO_BE(64)

#undef DECLARE_HOST_TO_BE



			/// <summary>
			/// Convert a value from little endian to host endian
			/// </summary>
#define DECLARE_LE_TO_HOST(bits)								\
			constexpr uint##bits##_t LEToHost(uint##bits##_t x)	\
			{													\
				if constexpr (BigEndian)						\
					return Swap(x);								\
				else											\
					return x;									\
			}

			DECLARE_LE_TO_HOST(16)
			DECLARE_LE_TO_HOST(32)
			DECLARE_LE_TO_HOST(64)

#undef DECLARE_LE_TO_HOST



			/// <summary>
			/// Convert a value from big endian to host endian
			/// </summary>
#define DECLARE_BE_TO_HOST(bits)								\
			constexpr uint##bits##_t BEToHost(uint##bits##_t x)	\
			{													\
				if constexpr (BigEndian)						\
					return Swap(x);								\
				else											\
					return x;									\
			}

			DECLARE_BE_TO_HOST(16)
			DECLARE_BE_TO_HOST(32)
			DECLARE_BE_TO_HOST(64)

#undef DECLARE_BE_TO_HOST


		}


	}

}





// #undef foward declared definitions

#endif // ROBINLE_DATA_ENDIAN