#include "rlAudioDevices.hpp"

#include <functiondiscoverykeys_devpkey.h>
#include <Windows.h>
#include <mmeapi.h>

#pragma comment(lib, "winmm.lib")





namespace rl
{

	/***********************************************************************************************
	class AudioDeviceManager
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	AudioDeviceManager::AudioDeviceManager() : m_oNotificationClient(*this)
	{
		HRESULT hr;
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
			__uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);

		IMMDeviceCollection* pCollection = nullptr;
		m_pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
		UINT iCount = 0;
		hr = pCollection->GetCount(&iCount);
		for (UINT i = 0; i < iCount; i++)
		{
			IMMDevice* pDevice;
			pCollection->Item(i, &pDevice);
			LPWSTR szID = NULL;
			hr = pDevice->GetId(&szID);
			addDevice(szID);
			pDevice->Release();
		}
		pCollection->Release();

		IMMDevice* pDevice = NULL;
		hr = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
		LPWSTR szID = NULL;
		hr = pDevice->GetId(&szID);
		m_sDefaultDevice = szID;
		pDevice->Release();

		m_pEnumerator->RegisterEndpointNotificationCallback(&m_oNotificationClient);
	}

	AudioDeviceManager::~AudioDeviceManager()
	{
		m_pEnumerator->UnregisterEndpointNotificationCallback(&m_oNotificationClient);
		m_pEnumerator->Release();
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	AudioDeviceManager& AudioDeviceManager::getInstance()
	{
		static AudioDeviceManager oMgr;
		return oMgr;
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void AudioDeviceManager::getDevice(AudioDevice& dest, const wchar_t* szID)
	{
		if (szID == nullptr && m_sDefaultDevice.empty())
		{
			dest = {};
			return;
		}

		if (szID == nullptr)
			dest.sID = m_sDefaultDevice;
		else
			dest.sID = szID;

		for (auto& o : m_oDevices)
		{
			if (o.sID == dest.sID)
			{
				dest = o;
				return;
			}
		}

		dest = {};
	}

	bool AudioDeviceManager::registerCallback(const wchar_t* szDeviceID,
		IAudioDeviceEventCallback* callback)
	{
		for (size_t i = 0; i < m_oCallbacks.size(); i++)
		{
			if (m_oCallbacks[i].pCallback == callback)
				return false;
		}

		CallbackInfo info = { callback, szDeviceID };
		m_oCallbacks.push_back(info);

		return true;
	}

	void AudioDeviceManager::unregisterCallback(IAudioDeviceEventCallback* callback)
	{
		for (size_t i = 0; i < m_oCallbacks.size(); i++)
		{
			if (m_oCallbacks[i].pCallback == callback)
			{
				m_oCallbacks.erase(m_oCallbacks.begin() + i);
				break;
			}
		}
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void AudioDeviceManager::OnDefaultDeviceChanged(std::wstring sDeviceID)
	{
		m_sDefaultDevice = sDeviceID;

		AudioDevice device = {};
		getDevice(device, m_sDefaultDevice.c_str());

		for (auto o : m_oCallbacks)
			if (o.sDeviceID.empty())
				o.pCallback->OnDeviceChanged(device);
	}

	void AudioDeviceManager::OnDeviceRemoved(LPCWSTR pwstrId)
	{
		removeDevice(pwstrId);
	}

	void AudioDeviceManager::OnDeviceAdded(LPCWSTR pwstrId) { addDevice(pwstrId); }

	void AudioDeviceManager::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
	{
		if (dwNewState == DEVICE_STATE_ACTIVE)
			addDevice(pwstrDeviceId);
		else
			removeDevice(pwstrDeviceId);
	}

	void AudioDeviceManager::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
	{
		if (key != PKEY_AudioEngine_DeviceFormat &&
			key != PKEY_Device_FriendlyName)
			return;

		HRESULT hr;
		IMMDevice* pDevice = NULL;
		hr = m_pEnumerator->GetDevice(pwstrDeviceId, &pDevice);
		IPropertyStore* pProps = NULL;
		hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
		PROPVARIANT pv = {};
		hr = pProps->GetValue(key, &pv);

		size_t index;
		for (index = 0; index < m_oDevices.size(); index++)
		{
			if (m_oDevices[index].sID == pwstrDeviceId)
				break;
		}

		if (key == PKEY_AudioEngine_DeviceFormat)
		{
			if (((WAVEFORMATEX*)pv.blob.pBlobData)->nChannels != m_oDevices[index].iChannelCount)
			{
				m_oDevices[index].iChannelCount =
					(uint8_t)((WAVEFORMATEX*)pv.blob.pBlobData)->nChannels;
				bool bDefault = m_sDefaultDevice == pwstrDeviceId;
				for (auto o : m_oCallbacks)
				{
					if (o.sDeviceID == pwstrDeviceId || (bDefault && o.sDeviceID.empty()))
						o.pCallback->OnDeviceChanged(m_oDevices[index]);
				}
			}
		}
		else // PKEY_Device_FriendlyName
		{
			m_oDevices[index].sFriendlyName = pv.pwszVal;
			for (auto o : m_oCallbacks)
			{
				o.pCallback->OnDeviceDetailsChanged(m_oDevices[index]);
			}
		}

		pProps->Release();
		pDevice->Release();
	}

	void AudioDeviceManager::addDevice(LPCWSTR pwstrId)
	{
		AudioDevice device = {};
		device.sID = pwstrId;

		HRESULT hr;
		IMMDevice* pDevice = NULL;
		hr = m_pEnumerator->GetDevice(pwstrId, &pDevice);

		// check if output

		IMMEndpoint* pEndpoint = NULL;
		hr = pDevice->QueryInterface(__uuidof(IMMEndpoint), (VOID**)&pEndpoint);
		EDataFlow eFlow = {};
		hr = pEndpoint->GetDataFlow(&eFlow);
		pEndpoint->Release();

		if (eFlow != eRender)
		{
			pDevice->Release();
			return;
		}



		// get properties

		IPropertyStore* pProps = NULL;
		hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);

		PROPVARIANT oProp;

		hr = pProps->GetValue(PKEY_Device_FriendlyName, &oProp);
		device.sFriendlyName = oProp.pwszVal;

		hr = pProps->GetValue(PKEY_AudioEngine_DeviceFormat, &oProp);
		device.iChannelCount = (uint8_t)((WAVEFORMATEX*)oProp.blob.pBlobData)->nChannels;

		pProps->Release();
		pDevice->Release();

		m_oDevices.push_back(device);
	}

	void AudioDeviceManager::removeDevice(LPCWSTR pwstrId)
	{
		AudioDevice device = {};
		getDevice(device, m_sDefaultDevice.c_str());

		for (auto o : m_oCallbacks)
		{
			if (o.sDeviceID == pwstrId)
			{
				o.sDeviceID = L"";
				if (m_oDevices.size() > 0)
					o.pCallback->OnDeviceChanged(device);
				else
					o.pCallback->OnNoDevice();
			}
		}

		for (size_t i = 0; i < m_oDevices.size(); i++)
		{
			if (m_oDevices[i].sID == pwstrId)
			{
				m_oDevices.erase(m_oDevices.begin() + i);
				break;
			}
		}
	}





	/***********************************************************************************************
	class AudioDeviceManager::NotificationClient
	***********************************************************************************************/

	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	ULONG AudioDeviceManager::NotificationClient::AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	ULONG AudioDeviceManager::NotificationClient::Release()
	{
		ULONG ulRef = InterlockedDecrement(&m_cRef);
		if (ulRef == 0)
		{
			delete this;
		}
		return ulRef;
	}

	HRESULT AudioDeviceManager::NotificationClient::QueryInterface(REFIID riid, VOID** ppvInterface)
	{
		if (riid == IID_IUnknown)
		{
			AddRef();
			*ppvInterface = (IUnknown*)this;
		}
		else if (riid == __uuidof(IMMNotificationClient))
		{
			AddRef();
			*ppvInterface = (IMMNotificationClient*)this;
		}
		else
		{
			*ppvInterface = NULL;
			return E_NOINTERFACE;
		}
		return S_OK;
	}

	HRESULT AudioDeviceManager::NotificationClient::OnDefaultDeviceChanged(EDataFlow flow,
		ERole role, LPCWSTR pwstrDefaultDeviceId)
	{
		if (flow != eRender) // only handle output devices
			return S_OK;

		m_oMgr.OnDefaultDeviceChanged(pwstrDefaultDeviceId);
		return S_OK;
	}

	HRESULT AudioDeviceManager::NotificationClient::OnDeviceAdded(LPCWSTR pwstrDeviceId)
	{
		m_oMgr.OnDeviceAdded(pwstrDeviceId);
		return S_OK;
	}

	HRESULT AudioDeviceManager::NotificationClient::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
	{
		m_oMgr.OnDeviceRemoved(pwstrDeviceId);
		return S_OK;
	}

	HRESULT AudioDeviceManager::NotificationClient::OnDeviceStateChanged(LPCWSTR pwstrDeviceId,
		DWORD dwNewState)
	{
		m_oMgr.OnDeviceStateChanged(pwstrDeviceId, dwNewState);
		return S_OK;
	}

	HRESULT AudioDeviceManager::NotificationClient::OnPropertyValueChanged(LPCWSTR pwstrDeviceId,
		const PROPERTYKEY key)
	{
		m_oMgr.OnPropertyValueChanged(pwstrDeviceId, key);
		return S_OK;
	}

}