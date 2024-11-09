//------------------------------------------------------------------------------
// <copyright file="AudioCaptureRaw.cpp" company="Microsoft">
// 	 
//	 Copyright 2013 Microsoft Corporation 
// 	 
//	Licensed under the Apache License, Version 2.0 (the "License"); 
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
// 	 
//		 http://www.apache.org/licenses/LICENSE-2.0 
// 	 
//	Unless required by applicable law or agreed to in writing, software 
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
//	See the License for the specific language governing permissions and 
//	limitations under the License. 
// 	 
// </copyright>
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <nuiapi.h>
#include <shlobj.h>
#include <wchar.h>
#include <devicetopology.h>

#include "WASAPICapture.h"

// Number of milliseconds of acceptable lag between live sound being produced and recording operation.
const int TargetLatency = 20;

/// <summary>
/// Get global ID for specified device.
/// </summary>
/// <param name="pDevice">
/// [in] Audio device for which we're getting global ID.
/// </param>
/// <param name="ppszGlobalId">
/// [out] Global ID corresponding to audio device.
/// </param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT GetGlobalId(IMMDevice *pDevice, wchar_t **ppszGlobalId)
{
    IDeviceTopology *pTopology = NULL;
    HRESULT hr = S_OK;

    hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void**>(&pTopology));
    if (SUCCEEDED(hr))
    {
        IConnector *pPlug = NULL;

        hr = pTopology->GetConnector(0, &pPlug);
        if (SUCCEEDED(hr))
        {
            IConnector *pJack = NULL;

            hr = pPlug->GetConnectedTo(&pJack);
            if (SUCCEEDED(hr))
            {
                IPart *pJackAsPart = NULL;
                pJack->QueryInterface(IID_PPV_ARGS(&pJackAsPart));

                hr = pJackAsPart->GetGlobalId(ppszGlobalId);
                SafeRelease(pJackAsPart);
            }

            SafeRelease(pPlug);
        }

        SafeRelease(pTopology);
    }

    return hr;
}

/// <summary>
/// Determine if a global audio device ID corresponds to a Kinect sensor.
/// </summary>
/// <param name="pNuiSensor">
/// [in] A Kinect sensor.
/// </param>
/// <param name="pszGlobalId">
/// [in] Global audio device ID to compare to the Kinect sensor's ID.
/// </param>
/// <returns>
/// true if the global device ID corresponds to the sensor specified, false otherwise.
/// </returns>
bool IsMatchingAudioDevice(INuiSensor *pNuiSensor, wchar_t *pszGlobalId)
{
    // Get USB device name from the sensor
    BSTR arrayName = pNuiSensor->NuiAudioArrayId(); // e.g. "USB\\VID_045E&PID_02BB&MI_02\\7&9FF7F87&0&0002"

    wistring strDeviceName(pszGlobalId); // e.g. "{2}.\\\\?\\usb#vid_045e&pid_02bb&mi_02#7&9ff7f87&0&0002#{6994ad04-93ef-11d0-a3cc-00a0c9223196}\\global/00010001"
    wistring strArrayName(arrayName);

    // Make strings have the same internal delimiters
    wistring::size_type findIndex = strArrayName.find(L'\\');
    while (strArrayName.npos != findIndex)
    {
        strArrayName[findIndex] = L'#';
        findIndex = strArrayName.find(L'\\', findIndex + 1);
    }

    // Try to match USB part names for sensor vs audio device global ID
    bool match = strDeviceName.find(strArrayName) != strDeviceName.npos;

    SysFreeString(arrayName);
    return match;
}

/// <summary>
/// Get an audio device that corresponds to the specified Kinect sensor, if such a device exists.
/// </summary>
/// <param name="pNuiSensor">
/// [in] Kinect sensor for which we'll find a corresponding audio device.
/// </param>
/// <param name="ppDevice">
/// [out] Pointer to hold matching audio device found.
/// </param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT GetMatchingAudioDevice(INuiSensor *pNuiSensor, IMMDevice **ppDevice)
{
    IMMDeviceEnumerator *pDeviceEnumerator = NULL;
    IMMDeviceCollection *pDdeviceCollection = NULL;
    HRESULT hr = S_OK;

    *ppDevice = NULL;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDeviceEnumerator));
    if (SUCCEEDED(hr))
    {
        hr = pDeviceEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pDdeviceCollection);
        if (SUCCEEDED(hr))
        {
            UINT deviceCount;
            hr = pDdeviceCollection->GetCount(&deviceCount);
            if (SUCCEEDED(hr))
            {
                // Iterate through all active audio capture devices looking for one that matches
                // the specified Kinect sensor.
                for (UINT i = 0 ; i < deviceCount; ++i)
                {
                    IMMDevice *pDevice = NULL;

                    hr = pDdeviceCollection->Item(i, &pDevice);
                    if (SUCCEEDED(hr))
                    {
                        wchar_t *pszGlobalId = NULL;
                        hr = GetGlobalId(pDevice, &pszGlobalId);
                        if (SUCCEEDED(hr) && IsMatchingAudioDevice(pNuiSensor, pszGlobalId))
                        {
                            *ppDevice = pDevice;
                            CoTaskMemFree(pszGlobalId);
                            break;
                        }

                        CoTaskMemFree(pszGlobalId);
                    }

                    SafeRelease(pDevice);
                }
            }

            SafeRelease(pDdeviceCollection);
        }

        SafeRelease(pDeviceEnumerator);
    }

    if (SUCCEEDED(hr) && (NULL == *ppDevice))
    {
        // If nothing went wrong but we haven't found a device, return failure
        hr = E_FAIL;
    }

    return hr;
}

//
//  A wave file consists of:
//
//  RIFF header:    8 bytes consisting of the signature "RIFF" followed by a 4 byte file length.
//  WAVE header:    4 bytes consisting of the signature "WAVE".
//  fmt header:     4 bytes consisting of the signature "fmt " followed by a WAVEFORMATEX 
//  WAVEFORMAT:     <n> bytes containing a waveformat structure.
//  DATA header:    8 bytes consisting of the signature "data" followed by a 4 byte file length.
//  wave data:      <m> bytes containing wave data.
//

//  Header for a WAV file - we define a structure describing the first few fields in the header for convenience.
struct WAVEHEADER
{
    DWORD   dwRiff;                     // "RIFF"
    DWORD   dwSize;                     // Size
    DWORD   dwWave;                     // "WAVE"
    DWORD   dwFmt;                      // "fmt "
    DWORD   dwFmtSize;                  // Wave Format Size
};

//  Static RIFF header, we'll append the format to it.
const BYTE WaveHeaderTemplate[] = 
{
    'R',   'I',   'F',   'F',  0x00,  0x00,  0x00,  0x00, 'W',   'A',   'V',   'E',   'f',   'm',   't',   ' ', 0x00, 0x00, 0x00, 0x00
};

//  Static wave DATA tag.
const BYTE WaveData[] = { 'd', 'a', 't', 'a'};

/// <summary>
/// Write the WAV file header contents. 
/// </summary>
/// <param name="waveFile">
/// [in] Handle to file where header will be written.
/// </param>
/// <param name="pWaveFormat">
/// [in] Format of file to write.
/// </param>
/// <param name="dataSize">
/// Number of bytes of data in file's data section.
/// </param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT WriteWaveHeader(HANDLE waveFile, const WAVEFORMATEX *pWaveFormat, DWORD dataSize)
{
    DWORD waveHeaderSize = sizeof(WAVEHEADER) + sizeof(WAVEFORMATEX) + pWaveFormat->cbSize + sizeof(WaveData) + sizeof(DWORD);
    WAVEHEADER waveHeader;
    DWORD bytesWritten;

    // Update the sizes in the header
    memcpy_s(&waveHeader, sizeof(waveHeader), WaveHeaderTemplate, sizeof(WaveHeaderTemplate));
    waveHeader.dwSize = waveHeaderSize + dataSize - (2 * sizeof(DWORD));
    waveHeader.dwFmtSize = sizeof(WAVEFORMATEX) + pWaveFormat->cbSize;

    // Write the file header
    if (!WriteFile(waveFile, &waveHeader, sizeof(waveHeader), &bytesWritten, NULL))
    {
        return E_FAIL;
    }

    // Write the format
    if (!WriteFile(waveFile, pWaveFormat, sizeof(WAVEFORMATEX) + pWaveFormat->cbSize, &bytesWritten, NULL))
    {
        return E_FAIL;
    }

    // Write the data header
    if (!WriteFile(waveFile, WaveData, sizeof(WaveData), &bytesWritten, NULL))
    {
        return E_FAIL;
    }

    if (!WriteFile(waveFile, &dataSize, sizeof(dataSize), &bytesWritten, NULL))
    {
        return E_FAIL;
    }

    return S_OK;
}

/// <summary>
/// Get the name of the file where WAVE data will be stored.
/// </summary>
/// <param name="waveFileName">
/// [out] String buffer that will receive wave file name.
/// </param>
/// <param name="waveFileNameSize">
/// [in] Number of characters in waveFileName string buffer.
/// </param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT GetWaveFileName(wchar_t *waveFileName, UINT waveFileNameSize)
{
    wchar_t *knownPath = NULL;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Music, 0, NULL, &knownPath);

    if (SUCCEEDED(hr))
    {
        // Get the time
        wchar_t timeString[MAX_PATH];
        GetTimeFormatEx(NULL, 0, NULL, L"hh'-'mm'-'ss", timeString, _countof(timeString));

        // File name will be KinectAudio-HH-MM-SS.wav
        StringCchPrintfW(waveFileName, waveFileNameSize, L"%s\\KinectAudio-%s.wav", knownPath, timeString);
    }

    CoTaskMemFree(knownPath);
    return hr;
}

/// <summary>
/// Create the first connected Kinect sensor found.
/// </summary>
/// <param name="ppNuiSensor">
/// [out] Pointer to hold reference to created INuiSensor object.
/// </param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT CreateFirstConnected(INuiSensor **ppNuiSensor)
{
    INuiSensor *pNuiSensor = NULL;
    int iSensorCount = 0;
    HRESULT hr = S_OK;

    *ppNuiSensor = NULL;

    hr = NuiGetSensorCount(&iSensorCount);
    if (FAILED(hr))
    {
        return hr;
    }

    // Look at each Kinect sensor
    for (int i = 0; i < iSensorCount; ++i)
    {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &pNuiSensor);
        if (FAILED(hr))
        {
            continue;
        }

        // Get the status of the sensor, and if connected, then we can initialize it
        hr = pNuiSensor->NuiStatus();
        if (S_OK == hr)
        {
            *ppNuiSensor = pNuiSensor;
            pNuiSensor = NULL;
            break;
        }

        // This sensor wasn't OK, so release it since we're not using it
        SafeRelease(pNuiSensor);
    }

    if (SUCCEEDED(hr) && (NULL == *ppNuiSensor))
    {
        // If nothing went wrong but we haven't found a sensor, return failure
        hr = E_FAIL;
    }

    SafeRelease(pNuiSensor);
    return hr;
}

/// <summary>
/// Capture raw audio from Kinect USB audio device and write it out to a WAVE file.
/// </summary>
/// <param name="capturer">
/// [in] Object used to capture raw audio data from Kinect USB audio device.
/// </param>
/// <param name="waveFile">
/// [in] Handle to file where audio data will be written.
/// </param>
/// <param name="waveFileName">
/// [in] Name of file where audio data will be written.
/// </param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT CaptureAudio(CWASAPICapture *capturer, HANDLE waveFile, const wchar_t *waveFileName)
{
    HRESULT hr = S_OK;
    wchar_t ch;

    // Write a placeholder wave file header. Actual size of data section will be fixed up later.
    hr = WriteWaveHeader(waveFile, capturer->GetOutputFormat(), 0);
    if (SUCCEEDED(hr))
    {
        if (capturer->Start(waveFile))
        {
            printf_s("Capturing audio data to file %S\nPress 's' to stop capturing.\n", waveFileName);

            do
            {
                ch = _getwch();
            } while (L'S' != towupper(ch));

            printf_s("\n");

            capturer->Stop();

            // Fix up the wave file header to reflect the right amount of captured data.
            SetFilePointer(waveFile, 0, NULL, FILE_BEGIN);
            hr = WriteWaveHeader(waveFile, capturer->GetOutputFormat(), capturer->BytesCaptured());
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

/// <summary>
/// The core of the sample.
///
/// Pick an audio device that corresponds to a Kinect sensor, then capture data from
/// that device and write it to a file.
/// </summary>
/// <returns>
/// EXIT_SUCCESS if function was successful, otherwise EXIT_FAILURE.
/// </returns>
int wmain()
{
    wchar_t waveFileName[MAX_PATH];
    INuiSensor *pNuiSensor = NULL;
    IMMDevice *device = NULL;
    HANDLE waveFile = INVALID_HANDLE_VALUE;
    CWASAPICapture *capturer = NULL;

    printf_s("Raw Kinect Audio Data Capture Using WASAPI\n");
    printf_s("Copyright (c) Microsoft.  All Rights Reserved\n");
    printf_s("\n");

    //  A GUI application should use COINIT_APARTMENTTHREADED instead of COINIT_MULTITHREADED.
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        // Create the first connected Kinect sensor found.
        hr = CreateFirstConnected(&pNuiSensor);
        if (SUCCEEDED(hr))
        {
            //  Find the audio device corresponding to the kinect sensor.
            hr = GetMatchingAudioDevice(pNuiSensor, &device);
            if (SUCCEEDED(hr))
            {
                // Create the wave file that will contain audio data
                hr = GetWaveFileName(waveFileName, _countof(waveFileName));
                if (SUCCEEDED(hr))
                {
                    waveFile = CreateFile(waveFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 
                        NULL);
                    if (INVALID_HANDLE_VALUE != waveFile)
                    {
                        //  Instantiate a capturer
                        capturer = new (std::nothrow) CWASAPICapture(device);
                        if ((NULL != capturer) && capturer->Initialize(TargetLatency))
                        {
                            hr = CaptureAudio(capturer, waveFile, waveFileName);
                            if (FAILED(hr))
                            {
                                printf_s("Unable to capture audio data.\n");
                            }
                        }
                        else
                        {
                            printf_s("Unable to initialize capturer.\n");
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                        printf_s("Unable to create output WAV file %S.\nAnother application might be using this file.\n", waveFileName);
                        hr = E_FAIL;
                    }
                }
                else
                {
                    printf_s("Unable to construct output WAV file path.\n");
                }
            }
            else
            {
                printf_s("No matching audio device found!\n");
            }
        }
        else
        {
            printf_s("No ready Kinect found!\n");
        }
    }

    printf_s("Press any key to continue.\n");
    wchar_t ch = _getwch();
    UNREFERENCED_PARAMETER(ch);

    if (INVALID_HANDLE_VALUE != waveFile)
    {
        CloseHandle(waveFile);
    }

    delete capturer;
    SafeRelease(pNuiSensor);
    SafeRelease(device);
    CoUninitialize();
    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
