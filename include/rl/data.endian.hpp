/***************************************************************************************************
 FILE:	data.endian.hpp
 CPP:	n/a (header-only)
 DESCR:	Endian-related code
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DATA_ENDIAN
#define ROBINLE_DATA_ENDIAN





#include <type_traits>



namespace rl
{

	/// <summary>
	/// Does the current system use big endian?
	/// </summary>
	constexpr bool BigEndian = (const uint8_t &)0x00FF == 0x00;



	namespace Endian
	{

		/// <summary>
		/// Swap the byte order of a value.
		/// </summary>
		template <typename T>
		constexpr typename std::enable_if<
			(sizeof(T) > 1) &&
			std::is_fundamental<T>::value,
			T>::type // = T
			Swap(T val) noexcept
		{
			T tResult = 0;

			if constexpr(std::is_integral<T>::value)
			{
				for (size_t i = 0; i < sizeof(T); ++i)
				{
					tResult <<= 8;
					tResult |= (val & 0xFF);
					val >>= 8;
				}
			}
			else
			{
				auto pSrc  = reinterpret_cast<const uint8_t *>(&val);
				auto pDest = reinterpret_cast<uint8_t *>(&tResult);

				for (size_t i = 0; i < sizeof(T); ++i)
				{
					pDest[i] = pSrc[sizeof(T) - 1 - i];
				}
			}

			return tResult;
		}





		namespace Convert
		{

			/// <summary>
			/// Convert a value from host endian to little endian
			/// </summary>
			template <typename T>
			constexpr typename std::enable_if<
				(sizeof(T) > 1) && std::is_fundamental<T>::value,
				T>::type // = T
				HostToLE(T val) noexcept
			{
				if constexpr (BigEndian)
					return Swap(val);
				else
					return val;
			}



			/// <summary>
			/// Convert a value from host endian to big endian
			/// </summary>
			template <typename T>
			inline constexpr typename std::enable_if<
				(sizeof(T) > 1) && std::is_fundamental<T>::value,
				T>::type // = T
				HostToBE(T val) noexcept
			{
				if constexpr (BigEndian)
					return val;
				else
					return Swap(val);
			}



			/// <summary>
			/// Convert a value from little endian to host endian
			/// </summary>
			template <typename T>
			inline constexpr typename std::enable_if<
				(sizeof(T) > 1) && std::is_fundamental<T>::value,
				T>::type // = T
				LEToHost(T val) noexcept { return HostToLE(val); }



			/// <summary>
			/// Convert a value from big endian to host endian
			/// </summary>
			template <typename T>
			inline constexpr typename std::enable_if<
				(sizeof(T) > 1) && std::is_fundamental<T>::value,
				T>::type // = T
				BEToHost(T val) noexcept { return HostToBE(val); }

		}
	}
}





#endif // ROBINLE_DATA_ENDIAN