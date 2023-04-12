#include <rl/data.registry.hpp>

#include <rl/runasadmin.hpp>

#include <exception>





namespace
{
	template <typename TUInt>
	bool WStringToUInt(TUInt &iDest, const wchar_t *sz)
	{
		iDest = 0;
		std::wstring_view sv = sz;

		if (sv.length() == 0)
			return false;

		bool bValid;

		// DECIMAL ------------------------------------
		bValid = true;
		for (auto c : sv)
		{
			if (c < '0' || c > '9')
			{
				bValid = false;
				break;
			}
		}
		if (bValid)
		{
			TUInt iMult = 1;
			for (size_t i = 0; i < sv.length(); ++i)
			{
				iDest += (sv[sv.length() - 1 - i] - '0') * iMult;
				iMult *= 10;
			}

			const auto sTMP = std::to_wstring(iDest);
			return sTMP == sz; // compare results
		}

		// HEXADECIMAL --------------------------------
		const wchar_t *szPREF_HEX[] =
		{
			L"$",
			L"0x"
		};
		for (auto szPrefix : szPREF_HEX)
		{
			if (sv.starts_with(szPrefix))
			{
				sv = sv.substr(wcslen(szPrefix));
				if (sv.empty() || sv.length() > (sizeof(QWORD) * 2))
					return false;

				bValid = true;
				for (auto c : sv)
				{
					if (c < '0' || (c > '9' && c < 'A') || (c > 'F' && c < 'a') || c > 'f')
					{
						bValid = false;
						break;
					}
				}

				if (!bValid)
					return false;

				for (size_t i = 0; i < sv.length(); ++i)
				{
					iDest <<= 4;
					const auto c = sv[i];

					TUInt iVal;
					if (c <= L'9')
						iVal = c - L'0';
					else if (c <= 'F')
						iVal = 0xA + (c - 'A');
					else
						iVal = 0xA + (c - 'a');

					iDest |= iVal;
				}

				return true;
			}
		}


		// BINARY -------------------------------------
		if (sv.starts_with(L"0b"))
		{
			sv = sv.substr(2);
			if (sv.empty() || sv.length() > sizeof(QWORD) * 8)
				return false;

			bValid = true;
			for (auto c : sv)
			{
				if (c < '0' || c > '1')
				{
					bValid = false;
					break; // for
				}
			}

			if (bValid)
			{
				for (size_t i = 0; i < sv.length(); ++i)
				{
					iDest <<= 1;
					iDest |= sv[0] - '0';
				}
				return true;
			}
		}

		return false;
	}

	const std::wstring &GetAppName()
	{
		static std::wstring s_sAppName;
		if (!s_sAppName.empty())
			return s_sAppName;

		wchar_t szPath[MAX_PATH + 1];

		if (GetModuleFileNameW(NULL, szPath, MAX_PATH + 1) == 0)
			return s_sAppName;

		std::wstring_view sv = szPath;
		auto iLastDelim = sv.find_last_of('\\');
		if (iLastDelim != sv.npos)
			sv = sv.substr(iLastDelim + 1);
		iLastDelim = sv.find_last_of('.');
		if (iLastDelim != sv.npos)
			sv = sv.substr(0, iLastDelim);

		s_sAppName = sv;
		return s_sAppName;
	}

}



namespace rl
{



	RegistryValue RegistryValue::ByBinary(const uint8_t *pData, size_t iSize)
	{
		return RegistryValue(pData, iSize);
	}

	RegistryValue RegistryValue::ByQWORD(QWORD qw) { return RegistryValue(qw); }
	RegistryValue RegistryValue::ByDWORD(DWORD dw) { return RegistryValue(dw); }

	RegistryValue RegistryValue::ByString(const wchar_t *sz)
	{
		if (!sz)
			throw std::exception("Empty string in RegistryValue constructor");

		return RegistryValue(sz, (wcslen(sz) + 1) * sizeof(wchar_t), RegistryType::String);
	}

	RegistryValue RegistryValue::ByExpandString(const wchar_t *sz)
	{
		if (!sz)
			throw std::exception("Empty string in RegistryValue constructor");

		return RegistryValue(sz, (wcslen(sz) + 1) * sizeof(wchar_t), RegistryType::ExpandString);
	}

	RegistryValue RegistryValue::ByMultiString(const wchar_t *sz)
	{
		if (!sz)
			throw std::exception("Empty string in RegistryValue constructor");

		size_t iTotalLength = 0;
		do
		{
			iTotalLength += wcslen(sz + iTotalLength) + 1;
		} while (sz[iTotalLength] != 0);
		++iTotalLength; // 2nd terminating zero

		return RegistryValue(sz, iTotalLength * sizeof(wchar_t), RegistryType::MultiString);
	}


	RegistryValue::RegistryValue(const uint8_t *pData, size_t iSize) :
		RegistryValue(pData, iSize, RegistryType::Binary) {}
	RegistryValue::RegistryValue(QWORD qw) :
		RegistryValue(&qw, sizeof(QWORD), RegistryType::QWORD) {}
	RegistryValue::RegistryValue(DWORD dw) :
		RegistryValue(&dw, sizeof(DWORD), RegistryType::DWORD) {}
	RegistryValue::RegistryValue(const wchar_t *sz) :
		RegistryValue(sz, (wcslen(sz) + 1) * sizeof(wchar_t), RegistryType::String) {}

	RegistryValue::RegistryValue(const void *pData, size_t iSize, RegistryType eType) :
		m_eType(eType), m_iSize(iSize)
	{
		if (eType == RegistryType::Unsupported)
			throw std::exception();

		if (iSize > 0)
		{
			recreate(iSize, true);
			memcpy_s(m_uData.p, m_iSize, pData, iSize);
		}

		analyze();
	}

	RegistryValue &RegistryValue::operator=(QWORD qw) noexcept
	{
		if (m_iSize != sizeof(QWORD))
			recreate(sizeof(QWORD));
		memcpy_s(m_spData.get(), m_iSize, &qw, sizeof(qw));
		m_eType = RegistryType::QWORD;

		analyze();
		return *this;
	}

	RegistryValue &RegistryValue::operator=(DWORD dw) noexcept
	{
		if (m_iSize != sizeof(DWORD))
			recreate(sizeof(DWORD));
		memcpy_s(m_spData.get(), m_iSize, &dw, sizeof(dw));
		m_eType = RegistryType::DWORD;

		analyze();
		return *this;
	}

	RegistryValue &RegistryValue::operator=(const wchar_t *sz) noexcept
	{
		const size_t len = wcslen(sz);
		const size_t size = (len + 1) * sizeof(wchar_t);

		if (m_iSize != size)
			recreate(size);
		memcpy_s(m_spData.get(), m_iSize, sz, size);
		m_eType = RegistryType::String;

		analyze();
		return *this;
	}

	RegistryValue::operator QWORD() const
	{
		QWORD result;
		if (!asQWORD(result))
			throw std::exception("Invalid cast");
		return result;
	}

	RegistryValue::operator DWORD() const
	{
		DWORD result;
		if (!asDWORD(result))
			throw std::exception("Invalid cast");
		return result;
	}

	RegistryValue::operator std::wstring() const
	{
		std::wstring result;
		if (!asString(result))
			throw std::exception("Invalid cast");
		return result;
	}

	bool RegistryValue::asQWORD(QWORD &qwDest) const noexcept
	{
		if (m_iSize == 0)
			return false;

		switch (m_eType)
		{
		case RegistryType::QWORD:
			qwDest = *m_uData.pQWORD;
			break;

		case RegistryType::DWORD:
			qwDest = *m_uData.pDWORD;
			break;

		case RegistryType::String:
		case RegistryType::ExpandString:
		{
			std::wstring s;
			if (!asString(s, true))
				return false;

			return WStringToUInt(qwDest, s.c_str());
		}

		default:
			return false;
		}
		return true;
	}

	bool RegistryValue::asDWORD(DWORD &dwDest) const noexcept
	{
		QWORD qwTMP;
		if (!asQWORD(qwTMP))
			return false;

		if (qwTMP & 0xFFFFFFFF00000000)
			return false; // cutoff

		dwDest = (DWORD)qwTMP;
		return true;
	}

	bool RegistryValue::asString(std::wstring &sDest, bool bExpand) const noexcept
	{
		sDest.clear();

		if (m_iSize == 0)
			return true;

		switch (m_eType)
		{
		case RegistryType::QWORD:
		{
			const auto qw = *m_uData.pQWORD;
			sDest = std::move(std::to_wstring(qw));
		}
		break;

		case RegistryType::DWORD:
		{
			const auto dw = *m_uData.pDWORD;
			sDest = std::move(std::to_wstring(dw));
		}
		break;

		case RegistryType::ExpandString:

			if (bExpand)
			{
				const DWORD dwSize = ExpandEnvironmentStringsW(m_uData.pEXPAND_SZ, NULL, 0);
				if (dwSize == 0)
					return false;
				sDest.resize((size_t)dwSize - 1);
				return (ExpandEnvironmentStringsW(m_uData.pEXPAND_SZ, sDest.data(),
					(DWORD)sDest.length() + 1) != 0);
			}
			else
				[[fallthrough]];
		case RegistryType::String:
			sDest.resize((m_iSize / sizeof(wchar_t)) - 1);
			memcpy_s(sDest.data(), m_iSize, m_uData.pSZ, m_iSize);
			break;


		default:
			return false;
		}

		return true;
	}

	bool RegistryValue::asMultiString(std::vector<std::wstring> &oDest) const noexcept
	{
		oDest.clear();

		if (m_iSize == 0)
			return true;



		switch (m_eType)
		{
		case RegistryType::QWORD:
		case RegistryType::DWORD:
		case RegistryType::String:
		case RegistryType::ExpandString:
		{
			std::wstring s;
			if (!asString(s))
				return false;

			oDest.push_back(std::move(s));
		}
		break;

		case RegistryType::MultiString:
		{
			auto sz = m_uData.pMULTI_SZ;
			while (sz[0] != 0)
			{
				if (sz[1] == 0) // end of multistring
					break; // while

				const size_t iLen = wcslen(sz);
				std::wstring s(iLen, '\0');
				wcscpy_s(s.data(), s.length() + 1, sz);
				oDest.push_back(std::move(s));

				sz += iLen + 1;
			}
		}
		break;

		default:
			return false;
		}
		return true;
	}

	void RegistryValue::clear() noexcept
	{
		m_uData.p   = nullptr;
		m_eType     = RegistryType::Unsupported;
		m_spData    = nullptr;
		m_iSize     = 0;
		m_bIntegral = false;
	}

	void RegistryValue::recreate(size_t iSize, bool bForce)
	{
		if (!bForce && iSize == m_iSize)
			return;

		if (iSize == 0)
		{
			clear();
			return;
		}

		m_iSize   = iSize;
		m_spData  = std::make_shared<uint8_t[]>(iSize);
		m_uData.p = m_spData.get();
	}

	void RegistryValue::analyze() noexcept
	{
		// INTEGRAL? ----------------------------------
		switch (m_eType)
		{
		case RegistryType::QWORD:
		case RegistryType::DWORD:
			m_bIntegral = true;
			break;

		case RegistryType::String:
		case RegistryType::ExpandString:
		{
			QWORD qwDummy;
			m_bIntegral = asQWORD(qwDummy);
		}
		break;

		default:
			m_bIntegral = false;
		}
	}





	namespace Registry
	{

		RegistryType Type_Win32ToRL(DWORD dwType)
		{
			switch (dwType)
			{
			case REG_BINARY:    return RegistryType::Binary;
			case REG_QWORD:     return RegistryType::QWORD;
			case REG_DWORD:     return RegistryType::DWORD;
			case REG_SZ:        return RegistryType::String;
			case REG_EXPAND_SZ: return RegistryType::ExpandString;
			case REG_MULTI_SZ:  return RegistryType::MultiString;
			default:            return RegistryType::Unsupported;
			}
		}

		DWORD Type_RLToWin32(RegistryType eType)
		{
			switch (eType)
			{
			case RegistryType::Binary:       return REG_BINARY;
			case RegistryType::QWORD:        return REG_QWORD;
			case RegistryType::DWORD:        return REG_DWORD;
			case RegistryType::String:       return REG_SZ;
			case RegistryType::ExpandString: return REG_EXPAND_SZ;
			case RegistryType::MultiString:  return REG_MULTI_SZ;
			default:                         return REG_NONE;
			}
		}



		RegistryValue GetValue(HKEY hKey, const wchar_t *szValueName)
		{
			DWORD dwType = 0;
			DWORD dwSize = 0;
			if (RegQueryValueExW(hKey, szValueName, 0, &dwType, NULL, &dwSize) != ERROR_SUCCESS)
				return {};

			const auto eType = Type_Win32ToRL(dwType);
			if (eType == RegistryType::Unsupported)
				return {};

			

			std::unique_ptr<uint8_t[]> up;
			if (dwSize > 0)
			{
				up = std::make_unique<uint8_t[]>(dwSize);
				if (RegQueryValueExW(hKey, szValueName, 0, NULL, up.get(), &dwSize)
					!= ERROR_SUCCESS)
					return {};
			}
			void *p = up.get();

			switch (eType)
			{
			case RegistryType::Binary:
			{
				if (dwSize == 0)
					return RegistryValue::ByBinary(nullptr, 0);

				return RegistryValue::ByBinary(up.get(), dwSize);
			}

			case RegistryType::QWORD:
				if (dwSize != sizeof(QWORD))
					return {};
				return RegistryValue::ByQWORD(*(QWORD *)p);

			case RegistryType::DWORD:
				if (dwSize != sizeof(DWORD))
					return {};
				return RegistryValue::ByDWORD(*(DWORD *)p);

			case RegistryType::String:
				return RegistryValue::ByString((const wchar_t *)p);

			case RegistryType::ExpandString:
				return RegistryValue::ByExpandString((const wchar_t *)p);

			case RegistryType::MultiString:
				return RegistryValue::ByMultiString((const wchar_t *)p);

			default:
				return {};
			}
		}

		bool SetValue(HKEY hKey, const wchar_t *szValueName, const RegistryValue &oVal)
		{
			if (!oVal || oVal.size() > MAXDWORD)
				return false;

			const DWORD dwType = Type_RLToWin32(oVal.type());

			return RegSetValueExW(hKey, szValueName, 0, dwType, oVal.data(), (DWORD)oVal.size())
				== ERROR_SUCCESS;
		}

	}
}