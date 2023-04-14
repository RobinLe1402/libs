/***************************************************************************************************
 FILE:	data.registry.hpp
 CPP:	data.registry.cpp
 DESCR:	Helper functionality for interacting with the registry.
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_DATA_REGISTRY
#define ROBINLE_DATA_REGISTRY





// Win32
#include <Windows.h>

// STL
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using QWORD = uint64_t; // definition missing in Windows.h for some reason?



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// Registry data types.
	/// </summary>
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





	/// <summary>
	/// A registry-compatible value.
	/// </summary>
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

		operator bool() const noexcept { return m_eType != RegistryType::Unsupported; }


		const uint8_t *data() const noexcept { return m_spData.get(); }
		uint8_t       *data() noexcept       { return m_spData.get(); }

		size_t size() const noexcept { return m_iSize; }
		auto type()   const noexcept { return m_eType; }

		bool tryGetQWORD(QWORD &qwDest) const noexcept;
		bool tryGetDWORD(DWORD &dwDest) const noexcept;
		bool tryGetString(std::wstring &sDest, bool bExpand = false) const noexcept;
		bool tryGetMultiString(std::vector<std::wstring> &oDest) const noexcept;

		QWORD                     asQWORD() const;
		DWORD                     asDWORD() const;
		std::wstring              asString(bool bExpand = false) const;
		std::vector<std::wstring> asMultiString() const;

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





	/// <summary>
	/// A wrapper for a <c>HKEY</c>.<para />
	/// Behaves the same as a <c>std::unique_ptr</c>.
	/// </summary>
	class UniqueRegistryKey final
	{
	public: // methods

		UniqueRegistryKey() = default;
		UniqueRegistryKey(HKEY hKey) noexcept;
		UniqueRegistryKey(UniqueRegistryKey &&rval) noexcept;
		UniqueRegistryKey(const UniqueRegistryKey &) = delete;
		~UniqueRegistryKey();

		UniqueRegistryKey &operator=(HKEY hKey) noexcept;
		UniqueRegistryKey &operator=(UniqueRegistryKey &&rval) noexcept;
		UniqueRegistryKey &operator=(const UniqueRegistryKey &) = delete;

		operator bool()  const { return m_hKey != NULL; }
		bool operator!() const { return m_hKey == NULL; }

		void reset();

		HKEY get() const { return m_hKey; }


	private: // variables

		HKEY m_hKey = NULL;

	};





	/// <summary>
	/// A wrapper for a <c>HKEY</c>.<para />
	/// Behaves the same as a <c>std::shared_key</c>.
	/// </summary>
	class SharedRegistryKey final
	{
	public: // methods

		// default methods
		
		SharedRegistryKey()                                     = default;
		SharedRegistryKey(const SharedRegistryKey &)            = default;
		SharedRegistryKey(SharedRegistryKey &&)                 = default;
		~SharedRegistryKey()                                    = default;

		SharedRegistryKey &operator=(const SharedRegistryKey &) = default;
		SharedRegistryKey &operator=(SharedRegistryKey &&)      = default;



		// custom methods

		SharedRegistryKey(HKEY hKey) noexcept;

		SharedRegistryKey &operator=(HKEY hKey) noexcept;

		operator bool()  const { return m_spKey != nullptr; }
		bool operator!() const { return !m_spKey; }


		void reset() noexcept { m_spKey.reset(); };

		HKEY get() const noexcept { return m_spKey != nullptr ? m_spKey->get() : NULL; }


	private: // variables

		std::shared_ptr<UniqueRegistryKey> m_spKey;

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