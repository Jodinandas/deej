// adapted from https://www.reddit.com/r/C_Programming/comments/la9ba2/detecting_current_audio_level_windows_api/ comment by Neui

#define COBJMACROS // Allow INTERFACE_METHOD(This, xxx)
#include <initguid.h> // https://stackoverflow.com/a/31757757
#include <windows.h>
#include <mmdeviceapi.h> // IMMDevice, IMMDeviceEnumerator
#include <endpointvolume.h> // IAudioMeterInformation
#include <stdio.h>

#if !defined(_MSC_VER) // MinGW doesn't seem to have those, copied from the official SDK and cleaned up a bit
typedef struct IAudioMeterInformationVtbl {
	BEGIN_INTERFACE
		HRESULT(STDMETHODCALLTYPE* QueryInterface)(IAudioMeterInformation* This, REFIID riid, void** ppvObject);
		ULONG(STDMETHODCALLTYPE* AddRef)(IAudioMeterInformation* This);
		ULONG(STDMETHODCALLTYPE* Release)(IAudioMeterInformation* This);
		HRESULT(STDMETHODCALLTYPE* GetPeakValue)(IAudioMeterInformation* This, float* pfPeak);
		HRESULT(STDMETHODCALLTYPE* GetMeteringChannelCount)(IAudioMeterInformation* This, UINT* pnChannelCount);
		HRESULT(STDMETHODCALLTYPE* GetChannelsPeakValues)(IAudioMeterInformation* This, UINT32 u32ChannelCount, float* afPeakValues);
		HRESULT(STDMETHODCALLTYPE* QueryHardwareSupport)(IAudioMeterInformation* This, DWORD* pdwHardwareSupportMask);
	END_INTERFACE
} IAudioMeterInformationVtbl;

interface IAudioMeterInformation {
	CONST_VTBL struct IAudioMeterInformationVtbl* lpVtbl;
};

#define IAudioMeterInformation_Release(This) ((This)->lpVtbl->Release(This))
#define IAudioMeterInformation_GetPeakValue(This,pfPeak) ((This)->lpVtbl->GetPeakValue(This, pfPeak))

DEFINE_GUID(IID_IAudioMeterInformation, 0xC02216F6, 0x8C67, 0x4B5B, 0x9D, 0x00, 0xD0, 0x08, 0xE7, 0x3E, 0x00, 0x64);
// C02216F6-8C67-4B5B-9D00-D008E73E0064
#else // Visual Studio in C mode
DEFINE_GUID(IID_IMMDeviceEnumerator, 0xa95664d2, 0x9614, 0x4f35, 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6);
DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xbcde0395, 0xe52f, 0x467c, 0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e);
DEFINE_GUID(IID_IAudioMeterInformation, 0xC02216F6, 0x8C67, 0x4B5B, 0x9D, 0x00, 0xD0, 0x08, 0xE7, 0x3E, 0x00, 0x64);
#endif
HRESULT hr;


IMMDevice* mmdevice = NULL;
IAudioMeterInformation* meter_info = NULL;

// initializes the windows audio device to provide access to IAudioMeter
int init_device() {
	IMMDeviceEnumerator* dev_enumerator = NULL;

	hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		puts("CoInitialize failed");
		return 1;
	}

	hr = CoCreateInstance(
		&CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, &IID_IMMDeviceEnumerator,
		(void**)&dev_enumerator);
	if (FAILED(hr)) {
		puts("CoCreateInstance failed");
		return 1;
	}

	hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(dev_enumerator,
		eRender,
		eConsole,
		&mmdevice);
	if (FAILED(hr)) {
		puts("GetDefaultAudioEndpoint failed");
		IMMDeviceEnumerator_Release(mmdevice);
		CoUninitialize();
		return 1;
	}
	return 0;
}

int init_meter() {
	hr = IMMDevice_Activate(mmdevice,
		&IID_IAudioMeterInformation,
		CLSCTX_ALL,
		NULL,
		(void**)&meter_info);
	if (FAILED(hr)) {
		puts("Activate failed");
		IMMDeviceEnumerator_Release(mmdevice);
		IMMDevice_Release(mmdevice);
		CoUninitialize();
		return 1;
	}
	return 0;
}

// returns the peak meter level since the last call, keep in mind this value is beetween 0-1 and normalized to 100% volume!
float get_meter_level() {
	float peak = -1.0f;
	hr = IAudioMeterInformation_GetPeakValue(meter_info, &peak);
	if (FAILED(hr)) {
		puts("GetPeakValue failed");
		IMMDeviceEnumerator_Release(mmdevice);
		IMMDevice_Release(mmdevice);
		IAudioMeterInformation_Release(meter_info);
		CoUninitialize();
		return 0.0;
	}
	return peak;
}

void cleanup() {
	IMMDeviceEnumerator_Release(mmdevice);
	IMMDevice_Release(mmdevice);
	IAudioMeterInformation_Release(meter_info);
	CoUninitialize();
	return;
}