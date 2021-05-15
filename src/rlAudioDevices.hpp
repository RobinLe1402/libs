/***************************************************************************************************
 FILE:	rlAudioEngine_Devices.hpp
 CPP:	rlAudioEngine_Devices.cpp
 DESCR:	Help classes for managing audio output devices
***************************************************************************************************/


#pragma once
#ifndef ROBINLE_AUDIOENGINE_DEVICES
#define ROBINLE_AUDIOENGINE_DEVICES





//==================================================================================================
// FORWARD DECLARATIONS

//--------------------------------------------------------------------------------------------------
// <header.h>


#include <string>
#include <vector>

#include <initguid.h>
#include <mmdeviceapi.h>



//==================================================================================================
// DECLARATION
namespace rl
{

	/// <summary>
	/// A single audio device
	/// </summary>
	struct AudioDevice
	{
		std::wstring sID;
		std::wstring sFriendlyName;
		uint8_t iChannelCount;
	};



	/// <summary>
	/// Callback interface for audio device events
	/// </summary>
	class IAudioDeviceEventCallback
	{
	public: // methods

		/// <summary>
		/// Called when the last audio device is removed and there are no more audio devices
		/// </summary>
		virtual void OnNoDevice() {}

		/// <summary>
		/// Called when the monitored device changes<para/>
		/// (Either to another device, or in channel count)
		/// </summary>
		virtual void OnDeviceChanged(AudioDevice NewDevice) {}

		/// <summary>
		/// Called when the friendly name of the monitored device changes
		/// </summary>
		virtual void OnDeviceDetailsChanged(AudioDevice NewDetails) {}

	};



	class AudioDeviceManager
	{
	public: // methods

		static AudioDeviceManager& getInstance();



		/// <summary>
		/// Get the total count of current audio output devices
		/// </summary>
		inline auto deviceCount() { return m_oDevices.size(); }

		/// <summary>
		/// Enumerate all current audio output devices
		/// </summary>
		inline const std::vector<AudioDevice>& enumerateDevices() { return m_oDevices; }

		/// <summary>
		/// Get a device by ID<para/>
		/// If szID is nullptr, the default device will be chosen
		/// If there is no device, <c>dest</c> will be emptied
		/// </summary>
		void getDevice(AudioDevice& dest, const wchar_t* szID);

		/// <summary>
		/// Monitor a specific single audio output device
		/// </summary>
		/// <param name="szDeviceID">= ID of the MMEDevice (if nullptr --> default device)</param>
		bool registerCallback(const wchar_t* szDeviceID, IAudioDeviceEventCallback* callback);

		/// <summary>
		/// Stop monitoring an audio output device
		/// </summary>
		void unregisterCallback(IAudioDeviceEventCallback* callback);


	private: // methods

		AudioDeviceManager(); // --> singleton
		~AudioDeviceManager();



		void OnDefaultDeviceChanged(std::wstring sDeviceID);
		void OnDeviceRemoved(LPCWSTR pwstrId);
		void OnDeviceAdded(LPCWSTR pwstrId);
		void OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
		void OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);

		void addDevice(LPCWSTR pwstrId);
		void removeDevice(LPCWSTR pwstrId);


	private: // types

		struct CallbackInfo
		{
			IAudioDeviceEventCallback* pCallback;
			std::wstring sDeviceID;
		};

		class NotificationClient : public IMMNotificationClient
		{
		public: // methods

			NotificationClient(AudioDeviceManager& mgr) : m_oMgr(mgr), m_cRef(1) {}
			~NotificationClient() {}


		public: // IUnknown implementation

			ULONG STDMETHODCALLTYPE AddRef() override;
			ULONG STDMETHODCALLTYPE Release() override;
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface) override;


		public: // IMMNotificationClient

			HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role,
				LPCWSTR pwstrDefaultDeviceId) override;

			HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;

			HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;

			HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
				override;

			HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId,
				const PROPERTYKEY key) override;


		private: // variables

			AudioDeviceManager& m_oMgr;

			LONG m_cRef;

		};
		friend class NotificationClient;


	private: // variables

		NotificationClient m_oNotificationClient;
		std::vector<AudioDevice> m_oDevices;
		std::wstring m_sDefaultDevice;
		std::vector<CallbackInfo> m_oCallbacks;
		IMMDeviceEnumerator* m_pEnumerator = nullptr;

	};

}





#endif // ROBINLE_AUDIOENGINE_DEVICES