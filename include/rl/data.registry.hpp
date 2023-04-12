/***************************************************************************************************
 FILE:	data.registry.hpp
 CPP:	data.registry.cpp
		runasadmin.cpp
 DESCR:	Interface to the RobinLe registry infrastructure.
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DATA_REGISTRY
#define ROBINLE_DATA_REGISTRY





#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <Windows.h>

using QWORD = uint64_t; // definition missing in Windows.h for some reason?



//==================================================================================================
// DECLARATION
namespace rl
{

	enum class RegistryType
	{
		Binary,       // binary data
		QWORD,        // 64-bit unsigned int
		DWORD,        // 32-bit unsigned int
		String,       // single string
		ExpandString, // string with environment variables
		MultiString,  // multiple strings

		Unsupported   // type is not supported by the RobinLe registry framework.
	};

	class RegistryValue final
	{
	public: // static methods

		static RegistryValue ByBinary(const uint8_t *pData, size_t iSize);
		static RegistryValue ByQWORD(QWORD qw);
		static RegistryValue ByDWORD(DWORD dw);
		static RegistryValue ByString(const wchar_t *sz);
		static RegistryValue ByExpandString(const wchar_t *sz);
		static RegistryValue ByMultiString(const wchar_t *sz);


	public: // methods

		RegistryValue() = default;
		RegistryValue(const uint8_t *pData, size_t iSize);
		RegistryValue(QWORD qw);
		RegistryValue(DWORD dw);
		RegistryValue(const wchar_t *sz);
		RegistryValue(const RegistryValue &) = default;
		RegistryValue(RegistryValue &&) = default;
		~RegistryValue() = default;

		RegistryValue &operator=(QWORD qw) noexcept;
		RegistryValue &operator=(DWORD dw) noexcept;
		RegistryValue &operator=(const wchar_t *sz) noexcept;
		RegistryValue &operator=(const RegistryValue &) = default;
		RegistryValue &operator=(RegistryValue &&) = default;

		explicit operator QWORD() const;
		explicit operator DWORD() const;
		explicit operator std::wstring() const;
		operator bool() const noexcept { return m_eType != RegistryType::Unsupported; }


		const uint8_t *data() const noexcept { return m_spData.get(); }
		uint8_t *data() noexcept { return m_spData.get(); }

		size_t size() const noexcept { return m_iSize; }
		auto type() const noexcept { return m_eType; }

		bool asQWORD(QWORD &qwDest) const noexcept;
		bool asDWORD(DWORD &dwDest) const noexcept;
		bool asString(std::wstring &sDest, bool bExpand = false) const noexcept;
		bool asMultiString(std::vector<std::wstring> &oDest) const noexcept;

		bool isIntegral() const noexcept { return m_bIntegral; }

		void clear() noexcept;


	private: // methods
		
		RegistryValue(const void *pData, size_t iSize, RegistryType eType);
		void recreate(size_t iSize, bool bForce = false);
		void analyze() noexcept;


	private: // variables

		union
		{
			const wchar_t *pSZ;
			const wchar_t *pEXPAND_SZ;
			const wchar_t *pMULTI_SZ;

			QWORD         *pQWORD;
			DWORD         *pDWORD;

			void          *p;
		} m_uData{};

		RegistryType m_eType = RegistryType::Unsupported;

		std::shared_ptr<uint8_t[]> m_spData;
		size_t m_iSize = 0;
		bool m_bIntegral = false;
	};

	

	namespace Registry
	{
		RegistryType Type_Win32ToRL(DWORD dwType);
		DWORD        Type_RLToWin32(RegistryType eType);

		RegistryValue GetValue(HKEY hKey, const wchar_t *szValueName);
		bool SetValue(HKEY hKey, const wchar_t *szValueName, const RegistryValue &oVal);
	}
	
}





#endif // ROBINLE_DATA_REGISTRY