//------------------------------------------------------------------------------
// <copyright file="KinectFusionProcessor.cpp" company="Microsoft">
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

// System includes
#include "stdafx.h"

#pragma warning(push)
#pragma warning(disable:6255)
#pragma warning(disable:6263)
#include "ppl.h"
#pragma warning(pop)

// Project includes
#include "KinectFusionProcessor.h"
#include "KinectFusionHelper.h"

#define AssertOwnThread() \
    _ASSERT_EXPR(GetCurrentThreadId() == m_threadId, __FUNCTIONW__ L" called on wrong thread!");

#define AssertOtherThread() \
    _ASSERT_EXPR(GetCurrentThreadId() != m_threadId, __FUNCTIONW__ L" called on wrong thread!");

/// <summary>
/// Constructor
/// </summary>
KinectFusionProcessor::KinectFusionProcessor() :
    m_hWnd(nullptr),
    m_msgFrameReady(WM_NULL),
    m_msgUpdateSensorStatus(WM_NULL),
    m_hThread(nullptr),
    m_threadId(0),
    m_pVolume(nullptr),
    m_hrRecreateVolume(S_OK),
    m_pSensorChooser(nullptr),
    m_hStatusChangeEvent(nullptr),
    m_pNuiSensor(nullptr),
    m_hNextDepthFrameEvent(INVALID_HANDLE_VALUE),
    m_pDepthStreamHandle(INVALID_HANDLE_VALUE),
    m_cLostFrameCounter(0),
    m_bTrackingFailed(false),
    m_cFrameCounter(0),
    m_fFrameCounterStartTime(0),
    m_fMostRecentRaycastTime(0),
    m_pDepthImagePixelBuffer(nullptr),
    m_cPixelBufferLength(0),
    m_pDepthFloatImage(nullptr),
    m_pPointCloud(nullptr),
    m_pShadedSurface(nullptr),
    m_pShadedSurfaceNormals(nullptr),
    m_pFloatDeltaFromReference(nullptr),
    m_pShadedDeltaFromReference(nullptr),
    m_bKinectFusionInitialized(false),
    m_bResetReconstruction(false),
    m_bStopProcessing(false),
    m_bResolveSensorConflict(false),
    m_bIntegrationResumed(false)
{
    // Initialize synchronization objects
    InitializeCriticalSection(&m_lockParams);
    InitializeCriticalSection(&m_lockFrame);
    InitializeCriticalSection(&m_lockVolume);
    m_hStatusChangeEvent = CreateEvent(
        nullptr,
        FALSE, /* bManualReset */ 
        FALSE, /* bInitialState */
        nullptr);
    m_hNextDepthFrameEvent = CreateEvent(
        nullptr,
        TRUE, /* bManualReset */ 
        FALSE, /* bInitialState */
        nullptr);

    SetIdentityMatrix(m_worldToCameraTransform);
    SetIdentityMatrix(m_defaultWorldToVolumeTransform);

    m_cLastFrameTimeStamp = 0;
}

/// <summary>
/// Destructor
/// </summary>
KinectFusionProcessor::~KinectFusionProcessor()
{
    AssertOtherThread();

    // Shutdown the sensor
    StopProcessing();

    // Clean up Kinect Fusion
    SafeRelease(m_pVolume);

    if (m_pDepthFloatImage)
    {
        static_cast<void>(NuiFusionReleaseImageFrame(m_pDepthFloatImage));
    }
    if (m_pPointCloud)
    {
        static_cast<void>(NuiFusionReleaseImageFrame(m_pPointCloud));
    }
    if (m_pShadedSurface)
    {
        static_cast<void>(NuiFusionReleaseImageFrame(m_pShadedSurface));
    }
    if (m_pShadedSurfaceNormals)
    {
        static_cast<void>(NuiFusionReleaseImageFrame(m_pShadedSurfaceNormals));
    }
    if (m_pFloatDeltaFromReference)
    {
        static_cast<void>(NuiFusionReleaseImageFrame(m_pFloatDeltaFromReference));
    }

    if (m_pShadedDeltaFromReference)
    {
        static_cast<void>(NuiFusionReleaseImageFrame(m_pShadedDeltaFromReference));
    }

    // Clean up the depth pixel array
    SAFE_DELETE_ARRAY(m_pDepthImagePixelBuffer);

    // Clean up synchronization objects
    CloseHandle(m_hNextDepthFrameEvent);
    CloseHandle(m_hStatusChangeEvent);
    DeleteCriticalSection(&m_lockParams);
    DeleteCriticalSection(&m_lockFrame);
    DeleteCriticalSection(&m_lockVolume);
}

/// <summary>
/// Shuts down the sensor
/// </summary>
void KinectFusionProcessor::ShutdownSensor()
{
    AssertOwnThread();

    // Clean up Kinect
    if (m_pNuiSensor != nullptr)
    {
        m_pNuiSensor->NuiShutdown();
        SafeRelease(m_pNuiSensor);
    }
}

/// <summary>
/// Starts Kinect Fusion processing.
/// </summary>
/// <param name="phThread">returns the new processing thread's handle</param>
HRESULT KinectFusionProcessor::StartProcessing()
{
    AssertOtherThread();

    if (m_hThread == nullptr)
    {
        m_hThread = CreateThread(nullptr, 0, ThreadProc, this, 0, &m_threadId);
    }

    return (m_hThread != nullptr) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

/// <summary>
/// Stops Kinect Fusion processing.
/// </summary>
HRESULT KinectFusionProcessor::StopProcessing()
{
    AssertOtherThread();

    if (m_hThread != nullptr)
    {
        EnterCriticalSection(&m_lockParams);
        m_bStopProcessing = true;
        LeaveCriticalSection(&m_lockParams);

        WaitForSingleObject(m_hThread, INFINITE);
        m_hThread = nullptr;
    }

    return S_OK;
}

/// <summary>
/// Attempt to resolve a sensor conflict.
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::ResolveSensorConflict()
{
    AssertOtherThread();

    EnterCriticalSection(&m_lockParams);
    m_bResolveSensorConflict = true;
    LeaveCriticalSection(&m_lockParams);

    return S_OK;
}

/// <summary>
/// Thread procedure
/// </summary>
DWORD WINAPI KinectFusionProcessor::ThreadProc(LPVOID lpParameter)
{
    return reinterpret_cast<KinectFusionProcessor*>(lpParameter)->MainLoop();
}

/// <summary>
/// Is reconstruction volume initialized
/// </summary>
bool KinectFusionProcessor::IsVolumeInitialized()
{
    AssertOtherThread();

    return nullptr != m_pVolume;
}

/// <summary>
/// Main processing function
/// </summary>
DWORD KinectFusionProcessor::MainLoop()
{
    AssertOwnThread();

    // Bring in the first set of parameters
    EnterCriticalSection(&m_lockParams);
    m_paramsCurrent = m_paramsNext;
    LeaveCriticalSection(&m_lockParams);

    // Set the sensor status callback
    NuiSetDeviceStatusCallback(StatusChangeCallback, this);

    // Init the sensor chooser to find a valid sensor
    m_pSensorChooser = new(std::nothrow) NuiSensorChooser();
    if (nullptr == m_pSensorChooser)
    {
        SetStatusMessage(L"Memory allocation failure");
        NotifyEmptyFrame();
        return 1;
    }

    // Propagate any updates to the gpu index in use
    m_paramsNext.m_deviceIndex = m_paramsCurrent.m_deviceIndex;

    // Attempt to find a sensor for the first time
    UpdateSensorAndStatus(NUISENSORCHOOSER_SENSOR_CHANGED_FLAG);

    bool bStopProcessing = false;

    // Main loop
    while (!bStopProcessing)
    {
        HANDLE handles[] = { m_hNextDepthFrameEvent, m_hStatusChangeEvent };
        DWORD waitResult = WaitForMultipleObjects(ARRAYSIZE(handles), handles, FALSE, 100);

        // Get parameters and other external signals

        EnterCriticalSection(&m_lockParams);
        bool bChangeNearMode = m_paramsCurrent.m_bNearMode != m_paramsNext.m_bNearMode;
        bool bRecreateVolume = m_paramsCurrent.VolumeChanged(m_paramsNext);
        bStopProcessing = m_bStopProcessing;
        bool bResetReconstruction = m_bResetReconstruction;
        m_bResetReconstruction = false;
        bool bResolveSensorConflict = m_bResolveSensorConflict;
        m_bResolveSensorConflict = false;
        m_paramsCurrent = m_paramsNext;
        LeaveCriticalSection(&m_lockParams);

        if (bStopProcessing)
        {
            continue;
        }

        if (m_pNuiSensor == nullptr && bResolveSensorConflict)
        {
            DWORD dwChangeFlags;
            if (SUCCEEDED(m_pSensorChooser->TryResolveConflict(&dwChangeFlags)))
            {
                SetStatusMessage(L"");
                UpdateSensorAndStatus(dwChangeFlags);
            }
        }

        switch (waitResult)
        {
        case WAIT_OBJECT_0: // m_hNextDepthFrameEvent

            if (m_bKinectFusionInitialized)
            {
                // Clear status message from previous frame
                SetStatusMessage(L"");

                if (bChangeNearMode)
                {
                    if (nullptr != m_pNuiSensor)
                    {
                        DWORD flags =
                            m_paramsCurrent.m_bNearMode ?
                            NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE :
                            0;

                        m_pNuiSensor->NuiImageStreamSetImageFrameFlags(
                            m_pDepthStreamHandle,
                            flags);
                    }
                }

                EnterCriticalSection(&m_lockVolume);

                if (nullptr == m_pVolume && !FAILED(m_hrRecreateVolume))
                {
                    m_hrRecreateVolume = RecreateVolume();

                    // Set an introductory message on success
                    if (SUCCEEDED(m_hrRecreateVolume))
                    {
                        SetStatusMessage(
                            L"Click ‘Near Mode’ to change sensor range, and ‘Reset Reconstruction’ to clear!");
                    }
                }
                else if (bRecreateVolume)
                {
                    m_hrRecreateVolume = RecreateVolume();
                }
                else if (bResetReconstruction)
                {
                    HRESULT hr = InternalResetReconstruction();

                    if (SUCCEEDED(hr))
                    {
                        SetStatusMessage(L"Reconstruction has been reset.");
                    }
                    else
                    {
                        SetStatusMessage(L"Failed to reset reconstruction.");
                    }
                }

                ProcessDepth();

                LeaveCriticalSection(&m_lockVolume);

                NotifyFrameReady();
            }
            break;

        case WAIT_OBJECT_0 + 1: // m_hStatusChangeEvent

            if (nullptr != m_pSensorChooser)
            {
                // Handle sensor status change event
                DWORD dwChangeFlags = 0;

                HRESULT hr = m_pSensorChooser->HandleNuiStatusChanged(&dwChangeFlags);
                if (SUCCEEDED(hr))
                {
                    UpdateSensorAndStatus(dwChangeFlags);
                }
            }
            break;

        case WAIT_TIMEOUT:
            break;

        default:
            bStopProcessing = true;
        }

        if (m_pNuiSensor == nullptr)
        {
            // We have no sensor: Set frame rate to zero and notify the UI
            NotifyEmptyFrame();
        }
    }

    ShutdownSensor();

    return 0;
}

/// <summary>
/// Update the sensor and status based on the changed flags
/// </summary>
void KinectFusionProcessor::UpdateSensorAndStatus(DWORD dwChangeFlags)
{
    DWORD dwSensorStatus = NuiSensorChooserStatusNone;

    switch (dwChangeFlags)
    {
        case NUISENSORCHOOSER_SENSOR_CHANGED_FLAG:
        {
            // Free the previous sensor and try to get a new one
            SafeRelease(m_pNuiSensor);
            if (SUCCEEDED(CreateFirstConnected()))
            {
                if (SUCCEEDED(InitializeKinectFusion()))
                {
                    m_bKinectFusionInitialized = true;
                }
                else
                {
                    NotifyEmptyFrame();
                }
            }
        }
        __fallthrough;

    case NUISENSORCHOOSER_STATUS_CHANGED_FLAG:
        if (SUCCEEDED(m_pSensorChooser->GetStatus(&dwSensorStatus)))
        {
            if (m_hWnd != nullptr && m_msgUpdateSensorStatus != WM_NULL)
            {
                PostMessage(m_hWnd, m_msgUpdateSensorStatus, dwSensorStatus, 0);
            }
        }
        break;
    }
}

/// <summary>
/// This function will be called when Kinect device status changed
/// </summary>
void CALLBACK KinectFusionProcessor::StatusChangeCallback(
    HRESULT hrStatus,
    const OLECHAR* instancename,
    const OLECHAR* uniqueDeviceName,
    void* pUserData)
{
    KinectFusionProcessor* pThis = reinterpret_cast<KinectFusionProcessor*>(pUserData);
    SetEvent(pThis->m_hStatusChangeEvent);
}

/// <summary>
/// Create the first connected Kinect found 
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT KinectFusionProcessor::CreateFirstConnected()
{
    AssertOwnThread();

    // Get the Kinect and specify that we'll be using depth
    HRESULT hr = m_pSensorChooser->GetSensor(NUI_INITIALIZE_FLAG_USES_DEPTH, &m_pNuiSensor);

    if (SUCCEEDED(hr) && nullptr != m_pNuiSensor)
    {
        // Open a depth image stream to receive depth frames
        hr = m_pNuiSensor->NuiImageStreamOpen(
            NUI_IMAGE_TYPE_DEPTH,
            m_paramsCurrent.m_imageResolution,
            0,
            2,
            m_hNextDepthFrameEvent,
            &m_pDepthStreamHandle);
    }
    else
    {
        // Reset the event to non-signaled state
        ResetEvent(m_hNextDepthFrameEvent);
    }

    if (nullptr == m_pNuiSensor || FAILED(hr))
    {
        SafeRelease(m_pNuiSensor);
        SetStatusMessage(L"No ready Kinect found!");
        return E_FAIL;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Sets the UI window handle.
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::SetWindow(HWND hWnd, UINT msgFrameReady, UINT msgUpdateSensorStatus)
{
    AssertOtherThread();

    m_hWnd = hWnd;
    m_msgFrameReady = msgFrameReady;
    m_msgUpdateSensorStatus = msgUpdateSensorStatus;
    return S_OK;
}

/// <summary>
/// Sets the parameters.
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::SetParams(const KinectFusionParams& params)
{
    AssertOtherThread();

    EnterCriticalSection(&m_lockParams);
    m_paramsNext = params;
    LeaveCriticalSection(&m_lockParams);
    return S_OK;
}

/// <summary>
/// Lock the current frame while rendering it to the screen.
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::LockFrame(KinectFusionProcessorFrame const** ppFrame)
{
    AssertOtherThread();

    EnterCriticalSection(&m_lockFrame);
    *ppFrame = &m_frame;

    return S_OK;
}

/// <summary>
/// Unlock the previously locked frame.
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::UnlockFrame()
{
    AssertOtherThread();

    LeaveCriticalSection(&m_lockFrame);

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Initialize Kinect Fusion volume and images for processing
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::InitializeKinectFusion()
{
    AssertOwnThread();

    HRESULT hr = S_OK;

    hr = m_frame.Initialize(m_paramsCurrent.m_cImageSize);
    if (FAILED(hr))
    {
        SetStatusMessage(L"Failed to allocate frame buffers.");
        return hr;
    }

    // Frames generated from the depth input
    if (FAILED(hr = CreateFrame(NUI_FUSION_IMAGE_TYPE_FLOAT, &m_pDepthFloatImage)))
    {
        return hr;
    }

    // Point Cloud generated from ray-casting the volume
    if (FAILED(hr = CreateFrame(NUI_FUSION_IMAGE_TYPE_POINT_CLOUD, &m_pPointCloud)))
    {
        return hr;
    }

    // Image of the raycast Volume to display
    if (FAILED(hr = CreateFrame(NUI_FUSION_IMAGE_TYPE_COLOR, &m_pShadedSurface)))
    {
        return hr;
    }

    // Image of the raycast Volume with surface normals to display
    if (FAILED(hr = CreateFrame(NUI_FUSION_IMAGE_TYPE_COLOR, &m_pShadedSurfaceNormals)))
    {
        return hr;
    }

    // Image of the camera tracking deltas to display
    if (FAILED(hr = CreateFrame(NUI_FUSION_IMAGE_TYPE_FLOAT, &m_pFloatDeltaFromReference)))
    {
        return hr;
    }

    // Image of the camera tracking deltas to display
    if (FAILED(hr = CreateFrame(NUI_FUSION_IMAGE_TYPE_COLOR, &m_pShadedDeltaFromReference)))
    {
        return hr;
    }

    if (nullptr != m_pDepthImagePixelBuffer)
    {
        // If buffer length has changed, delete the old one.
        if (m_paramsCurrent.m_cImageSize != m_cPixelBufferLength)
        {
            SAFE_DELETE_ARRAY(m_pDepthImagePixelBuffer);
        }
    }

    if (nullptr == m_pDepthImagePixelBuffer)
    {
        // Depth pixel array to capture data from Kinect sensor
        m_pDepthImagePixelBuffer =
            new(std::nothrow) NUI_DEPTH_IMAGE_PIXEL[m_paramsCurrent.m_cImageSize];

        if (nullptr == m_pDepthImagePixelBuffer)
        {
            SetStatusMessage(L"Failed to initialize Kinect Fusion depth image pixel buffer.");
            return hr;
        }

        m_cPixelBufferLength = m_paramsCurrent.m_cImageSize;
    }

    return hr;
}

HRESULT KinectFusionProcessor::CreateFrame(
    NUI_FUSION_IMAGE_TYPE frameType,
    NUI_FUSION_IMAGE_FRAME** ppImageFrame)
{
    HRESULT hr = S_OK;
    
    if (nullptr != *ppImageFrame)
    {
        // If image size or type has changed, release the old one.
        if ((*ppImageFrame)->width != static_cast<UINT>(m_paramsCurrent.m_cDepthWidth) ||
            (*ppImageFrame)->height != static_cast<UINT>(m_paramsCurrent.m_cDepthHeight) ||
            (*ppImageFrame)->imageType != frameType)
        {
            static_cast<void>(NuiFusionReleaseImageFrame(*ppImageFrame));
            *ppImageFrame = nullptr;
        }
    }

    // Create a new frame as needed.
    if (nullptr == *ppImageFrame)
    {
        hr = NuiFusionCreateImageFrame(
            frameType,
            m_paramsCurrent.m_cDepthWidth,
            m_paramsCurrent.m_cDepthHeight,
            nullptr,
            ppImageFrame);

        if (FAILED(hr))
        {
            SetStatusMessage(L"Failed to initialize Kinect Fusion image.");
        }
    }

    return hr;
}

/// <summary>
/// Release and re-create a Kinect Fusion Reconstruction Volume
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::RecreateVolume()
{
    AssertOwnThread();

    HRESULT hr = S_OK;

    // Clean up Kinect Fusion
    SafeRelease(m_pVolume);

    SetIdentityMatrix(m_worldToCameraTransform);

    // Create the Kinect Fusion Reconstruction Volume
    hr = NuiFusionCreateReconstruction(
        &m_paramsCurrent.m_reconstructionParams,
        m_paramsCurrent.m_processorType,
        m_paramsCurrent.m_deviceIndex,
        &m_worldToCameraTransform,
        &m_pVolume);

    if (FAILED(hr))
    {
        if (E_NUI_GPU_FAIL == hr)
        {
            WCHAR buf[MAX_PATH];
            swprintf_s(buf, ARRAYSIZE(buf), L"Device %d not able to run Kinect Fusion, or error initializing.", m_paramsCurrent.m_deviceIndex);
            SetStatusMessage(buf);
        }
        else if (E_NUI_GPU_OUTOFMEMORY == hr)
        {
            WCHAR buf[MAX_PATH];
            swprintf_s(buf, ARRAYSIZE(buf), L"Device %d out of memory error initializing reconstruction - try a smaller reconstruction volume.", m_paramsCurrent.m_deviceIndex);
            SetStatusMessage(buf);
        }
        else if (NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_CPU != m_paramsCurrent.m_processorType)
        {
            WCHAR buf[MAX_PATH];
            swprintf_s(buf, ARRAYSIZE(buf), L"Failed to initialize Kinect Fusion reconstruction volume on device %d.", m_paramsCurrent.m_deviceIndex);
            SetStatusMessage(buf);
        }
        else
        {
            WCHAR buf[MAX_PATH];
            swprintf_s(buf, ARRAYSIZE(buf), L"Failed to initialize Kinect Fusion reconstruction volume on CPU %d.", m_paramsCurrent.m_deviceIndex);
            SetStatusMessage(buf);
        }

        return hr;
    }
    else
    {
        // Save the default world to volume transformation to be optionally used in ResetReconstruction
        hr = m_pVolume->GetCurrentWorldToVolumeTransform(&m_defaultWorldToVolumeTransform);
        if (FAILED(hr))
        {
            SetStatusMessage(L"Failed in call to GetCurrentWorldToVolumeTransform.");
            return hr;
        }
        
        if (m_paramsCurrent.m_bTranslateResetPoseByMinDepthThreshold)
        {
            // This call will set the world-volume transformation
            hr = InternalResetReconstruction();
            if (FAILED(hr))
            {
                return hr;
            }
        }

        // Reset pause and signal that the integration resumed
        m_paramsCurrent.m_bPauseIntegration = false;
        m_paramsNext.m_bPauseIntegration = false;
        m_bIntegrationResumed = true;
        m_frame.m_bIntegrationResumed = true;

        SetStatusMessage(L"Reconstruction has been reset.");
    }

    return hr;
}

/// <summary>
/// Get Extended depth data
/// </summary>
/// <param name="imageFrame">The extended depth image frame to copy.</param>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::CopyExtendedDepth(NUI_IMAGE_FRAME &imageFrame)
{
    AssertOwnThread();

    HRESULT hr = S_OK;

    if (nullptr == m_pDepthImagePixelBuffer)
    {
        SetStatusMessage(L"Error depth image pixel buffer is nullptr.");
        return E_FAIL;
    }

    INuiFrameTexture *extendedDepthTex = nullptr;

    // Extract the extended depth in NUI_DEPTH_IMAGE_PIXEL format from the frame
    BOOL nearModeOperational = FALSE;
    hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
        m_pDepthStreamHandle,
        &imageFrame,
        &nearModeOperational,
        &extendedDepthTex);

    if (FAILED(hr))
    {
        SetStatusMessage(L"Error getting extended depth texture.");
        return hr;
    }

    NUI_LOCKED_RECT extendedDepthLockedRect;

    // Lock the frame data to access the un-clamped NUI_DEPTH_IMAGE_PIXELs
    hr = extendedDepthTex->LockRect(0, &extendedDepthLockedRect, nullptr, 0);

    if (FAILED(hr) || extendedDepthLockedRect.Pitch == 0)
    {
        SetStatusMessage(L"Error getting extended depth texture pixels.");
        return hr;
    }

    // Copy the depth pixels so we can return the image frame
    errno_t err = memcpy_s(
        m_pDepthImagePixelBuffer,
        m_paramsCurrent.m_cImageSize * sizeof(NUI_DEPTH_IMAGE_PIXEL),
        extendedDepthLockedRect.pBits,
        extendedDepthTex->BufferLen());

    extendedDepthTex->UnlockRect(0);

    if (0 != err)
    {
        SetStatusMessage(L"Error copying extended depth texture pixels.");
        return hr;
    }

    return hr;
}

/// <summary>
/// Color the residual/delta image from the AlignDepthFloatToReconstruction call
/// </summary>
/// <param name="imageFrame">The extended depth image frame to copy.</param>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::ColorResiduals()
{
    HRESULT hr = S_OK;

    if (nullptr == m_pShadedDeltaFromReference || 
        nullptr == m_pFloatDeltaFromReference)
    {
        return E_FAIL;
    }
    if (nullptr == m_pShadedDeltaFromReference->pFrameTexture ||
        nullptr == m_pFloatDeltaFromReference->pFrameTexture)
    {
        return E_NOINTERFACE;
    }

    unsigned int width = m_pShadedDeltaFromReference->width;
    unsigned int height = m_pShadedDeltaFromReference->height;

    if (width != m_pFloatDeltaFromReference->width 
        || height != m_pFloatDeltaFromReference->height)
    {
        return E_INVALIDARG;
    }

    // 32bit ABGR color pixels for shaded image
    NUI_LOCKED_RECT shadedDeltasLockedRect;
    hr = m_pShadedDeltaFromReference->pFrameTexture->LockRect(0, &shadedDeltasLockedRect, nullptr, 0);
    if (FAILED(hr) || shadedDeltasLockedRect.Pitch == 0)
    {
        return hr;
    }

    // 32bit float per pixel signifies distance delta from the reconstructed surface model after AlignDepthFloatToReconstruction
    NUI_LOCKED_RECT floatDeltasLockedRect;
    hr = m_pFloatDeltaFromReference->pFrameTexture->LockRect(0, &floatDeltasLockedRect, nullptr, 0);
    if (FAILED(hr) || floatDeltasLockedRect.Pitch == 0)
    {
        return hr;
    }

    unsigned int *pColorBuffer = reinterpret_cast<unsigned int *>(shadedDeltasLockedRect.pBits);
    const float *pFloatBuffer = reinterpret_cast<float *>(floatDeltasLockedRect.pBits);

    Concurrency::parallel_for(0u, height, [&](unsigned int y)
    {
        unsigned int* pColorRow = reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(pColorBuffer) + (y * shadedDeltasLockedRect.Pitch));
        const float* pFloatRow = reinterpret_cast<const float*>(reinterpret_cast<const unsigned char*>(pFloatBuffer) + (y * floatDeltasLockedRect.Pitch));

        for (unsigned int x = 0; x < width; ++x)
        {
            float residue = pFloatRow[x];
            unsigned int color = 0;

            if (residue <= 1.0f)   // Pixel byte ordering: ARGB
            {
                color |= (255 << 24);                                                                               // a
                color |= (static_cast<unsigned char>(255.0f * clamp(1.0f + residue, 0.0f, 1.0f)) << 16);            // r
                color |= (static_cast<unsigned char>(255.0f * clamp(1.0f - std::abs(residue), 0.0f, 1.0f)) << 8);   // g
                color |= (static_cast<unsigned char>(255.0f * clamp(1.0f - residue, 0.0f, 1.0f)));                  // b
            }

            pColorRow[x] = color;
        }
    });

    m_pShadedDeltaFromReference->pFrameTexture->UnlockRect(0);
    m_pFloatDeltaFromReference->pFrameTexture->UnlockRect(0);

    return hr;
}

/// <summary>
/// Handle new depth data and perform Kinect Fusion processing
/// </summary>
void KinectFusionProcessor::ProcessDepth()
{
    AssertOwnThread();

    HRESULT hr = S_OK;
    bool depthAvailable = false;
    NUI_IMAGE_FRAME imageFrame;
    LONGLONG currentFrameTime = 0;
    bool raycastFrame = false;

    ////////////////////////////////////////////////////////
    // Get an extended depth frame from Kinect

    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
    if (FAILED(hr))
    {
        SetStatusMessage(L"Kinect NuiImageStreamGetNextFrame call failed.");
        goto FinishFrame;
    }

    hr = CopyExtendedDepth(imageFrame);

    currentFrameTime = imageFrame.liTimeStamp.QuadPart;

    // Release the Kinect camera frame
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);

    if (FAILED(hr))
    {
        goto FinishFrame;
    }

    // To enable playback of a .xed file through Kinect Studio and reset of the reconstruction
    // if the .xed loops, we test for when the frame timestamp has skipped a large number. 
    // Note: this will potentially continually reset live reconstructions on slow machines which
    // cannot process a live frame in less time than the reset threshold. Increase the number of
    // milliseconds in cResetOnTimeStampSkippedMilliseconds if this is a problem.

    int cResetOnTimeStampSkippedMilliseconds = cResetOnTimeStampSkippedMillisecondsGPU;

    if (m_paramsCurrent.m_processorType == NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_CPU)
    {
        cResetOnTimeStampSkippedMilliseconds = cResetOnTimeStampSkippedMillisecondsCPU;
    }

    if (m_cFrameCounter > 0 &&
        abs(currentFrameTime - m_cLastFrameTimeStamp) > cResetOnTimeStampSkippedMilliseconds)
    {
        HRESULT hr = InternalResetReconstruction();

        if (SUCCEEDED(hr))
        {
            SetStatusMessage(L"Reconstruction has been reset.");
        }
        else
        {
            SetStatusMessage(L"Failed to reset reconstruction.");
        }
    }

    m_cLastFrameTimeStamp = currentFrameTime;

    ////////////////////////////////////////////////////////
    // Depth to DepthFloat

    // Convert the pixels describing extended depth as unsigned short type in millimeters to depth
    // as floating point type in meters.
    hr = NuiFusionDepthToDepthFloatFrame(
            m_pDepthImagePixelBuffer,
            m_paramsCurrent.m_cDepthWidth,
            m_paramsCurrent.m_cDepthHeight,
            m_pDepthFloatImage,
            m_paramsCurrent.m_fMinDepthThreshold,
            m_paramsCurrent.m_fMaxDepthThreshold,
            m_paramsCurrent.m_bMirrorDepthFrame);

    if (FAILED(hr))
    {
        SetStatusMessage(L"Kinect Fusion NuiFusionDepthToDepthFloatFrame call failed.");
        goto FinishFrame;
    }

    depthAvailable = true;

    // Return if the volume is not initialized, just drawing the depth image
    if (nullptr == m_pVolume)
    {
        SetStatusMessage(
            L"Kinect Fusion reconstruction volume not initialized. "
            L"Please try reducing volume size or restarting.");
        goto FinishFrame;
    }

    ////////////////////////////////////////////////////////
    // Align Depth Image to Reconstruction Volume

    // Run the camera tracking algorithm with the volume
    HRESULT tracking = m_pVolume->AlignDepthFloatToReconstruction(
        m_pDepthFloatImage,
        NUI_FUSION_DEFAULT_ALIGN_ITERATION_COUNT,
        m_pFloatDeltaFromReference,
        nullptr,
        nullptr);

    if (FAILED(tracking))
    {
        if (tracking == E_NUI_FUSION_TRACKING_ERROR)
        {
            m_cLostFrameCounter++;
            m_bTrackingFailed = true;
            SetStatusMessage(
                L"Kinect Fusion camera tracking failed! Align the camera to the last tracked position.");
        }
        else
        {
            m_cLostFrameCounter++;
            m_bTrackingFailed = true;
            SetStatusMessage(L"Kinect Fusion AlignDepthFloatToReconstruction call failed!");
            hr = tracking;
        }
    }
    else
    {
        m_pVolume->GetCurrentWorldToCameraTransform(&m_worldToCameraTransform);
        m_cLostFrameCounter = 0;
        m_bTrackingFailed = false;
    }

    if (m_paramsCurrent.m_bAutoResetReconstructionWhenLost &&
        m_bTrackingFailed &&
        m_cLostFrameCounter >= cResetOnNumberOfLostFrames)
    {
        // Automatically Clear Volume and reset tracking if tracking fails
        hr = InternalResetReconstruction();

        if (SUCCEEDED(hr))
        {
            // Set bad tracking message
            SetStatusMessage(
                L"Kinect Fusion camera tracking failed, "
                L"automatically reset volume.");
        }
        else
        {
            SetStatusMessage(L"Kinect Fusion Reset Reconstruction call failed.");
            goto FinishFrame;
        }
    }

    ////////////////////////////////////////////////////////
    // Integrate Depth Data into volume

    // Don't integrate depth data into the volume if tracking failed or we have paused capture
    if (!m_bTrackingFailed && !m_paramsCurrent.m_bPauseIntegration)
    {
        // Integrate the depth data into the volume from the calculated camera pose
        hr = m_pVolume->IntegrateFrame(
                m_pDepthFloatImage,
                m_paramsCurrent.m_cMaxIntegrationWeight,
                &m_worldToCameraTransform);

        if (FAILED(hr))
        {
            SetStatusMessage(L"Kinect Fusion IntegrateFrame call failed.");
            goto FinishFrame;
        }
    }

    {
        double currentTime = m_timer.AbsoluteTime();

        // Is another frame already waiting?
        if (WaitForSingleObject(m_hNextDepthFrameEvent, 0) == WAIT_TIMEOUT)
        {
            // No: We should have enough time to raycast.
            raycastFrame = true;
        }
        else
        {
            // Yes: Raycast only if we've exceeded the render interval.
            double renderIntervalSeconds = (0.001 * cRenderIntervalMilliseconds);
            raycastFrame = (currentTime - m_fMostRecentRaycastTime > renderIntervalSeconds);
        }

        if (raycastFrame)
        {
            m_fMostRecentRaycastTime = currentTime;
        }
    }

    if (raycastFrame)
    {
        ////////////////////////////////////////////////////////
        // CalculatePointCloud

        // Raycast even if camera tracking failed, to enable us to visualize what is 
        // happening with the system
        hr = m_pVolume->CalculatePointCloud(m_pPointCloud, &m_worldToCameraTransform);

        if (FAILED(hr))
        {
            SetStatusMessage(L"Kinect Fusion CalculatePointCloud call failed.");
            goto FinishFrame;
        }

        ////////////////////////////////////////////////////////
        // ShadePointCloud

        // Map X axis to blue channel, Y axis to green channel and Z axiz to red channel,
        // normalizing each to the range [0, 1].
        Matrix4 worldToBGRTransform = { 0.0f };
        worldToBGRTransform.M11 = m_paramsCurrent.m_reconstructionParams.voxelsPerMeter / m_paramsCurrent.m_reconstructionParams.voxelCountX;
        worldToBGRTransform.M22 = m_paramsCurrent.m_reconstructionParams.voxelsPerMeter / m_paramsCurrent.m_reconstructionParams.voxelCountY;
        worldToBGRTransform.M33 = m_paramsCurrent.m_reconstructionParams.voxelsPerMeter / m_paramsCurrent.m_reconstructionParams.voxelCountZ;
        worldToBGRTransform.M41 = 0.5f;
        worldToBGRTransform.M42 = 0.5f;
        worldToBGRTransform.M44 = 1.0f;

        hr = NuiFusionShadePointCloud(
            m_pPointCloud,
            &m_worldToCameraTransform,
            &worldToBGRTransform,
            m_pShadedSurface,
            m_pShadedSurfaceNormals);

        if (FAILED(hr))
        {
            SetStatusMessage(L"Kinect Fusion NuiFusionShadePointCloud call failed.");
            goto FinishFrame;
        }
    }

FinishFrame:

    EnterCriticalSection(&m_lockFrame);

    m_frame.m_bIntegrationResumed = m_bIntegrationResumed;
    m_bIntegrationResumed = false;

    ////////////////////////////////////////////////////////
    // Copy the images to their frame buffers

    if (depthAvailable)
    {
        StoreImageToFrameBuffer(m_pDepthFloatImage, m_frame.m_pDepthRGBX);
    }

    if (SUCCEEDED(hr))
    {
        if (raycastFrame)
        {
            if (m_paramsCurrent.m_bDisplaySurfaceNormals)
            {
                StoreImageToFrameBuffer(m_pShadedSurfaceNormals, m_frame.m_pReconstructionRGBX);
            }
            else
            {
                StoreImageToFrameBuffer(m_pShadedSurface, m_frame.m_pReconstructionRGBX);
            }
        }

        hr  = ColorResiduals();

        StoreImageToFrameBuffer(m_pShadedDeltaFromReference, m_frame.m_pTrackingDataRGBX);
    }

    ////////////////////////////////////////////////////////
    // Periodically Display Fps

    if (SUCCEEDED(hr))
    {
        // Update frame counter
        m_cFrameCounter++;

        // Display fps count approximately every cTimeDisplayInterval seconds
        double elapsed = m_timer.AbsoluteTime() - m_fFrameCounterStartTime;
        if (static_cast<int>(elapsed) >= cTimeDisplayInterval)
        {
            m_frame.m_fFramesPerSecond = 0;

            // Update status display
            if (!m_bTrackingFailed)
            {
                m_frame.m_fFramesPerSecond = static_cast<float>(m_cFrameCounter / elapsed);
            }

            m_cFrameCounter = 0;
            m_fFrameCounterStartTime = m_timer.AbsoluteTime();
        }
    }

    m_frame.SetStatusMessage(m_statusMessage);

    LeaveCriticalSection(&m_lockFrame);
}

/// <summary>
/// Store a Kinect Fusion image to a frame buffer.
/// Accepts Depth Float, and Color image types.
/// </summary>
/// <param name="imageFrame">The image frame to store.</param>
/// <param name="buffer">The frame buffer.</param>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::StoreImageToFrameBuffer(
    const NUI_FUSION_IMAGE_FRAME* imageFrame,
    BYTE* buffer)
{
    AssertOwnThread();

    HRESULT hr = S_OK;

    if (nullptr == imageFrame || nullptr == imageFrame->pFrameTexture || nullptr == buffer)
    {
        return E_INVALIDARG;
    }

    if (NUI_FUSION_IMAGE_TYPE_COLOR != imageFrame->imageType &&
        NUI_FUSION_IMAGE_TYPE_FLOAT != imageFrame->imageType)
    {
        return E_INVALIDARG;
    }

    if (0 == imageFrame->width || 0 == imageFrame->height)
    {
        return E_NOINTERFACE;
    }

    INuiFrameTexture *imageFrameTexture = imageFrame->pFrameTexture;
    NUI_LOCKED_RECT LockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    imageFrameTexture->LockRect(0, &LockedRect, nullptr, 0);

    // Make sure we've received valid data
    if (LockedRect.Pitch != 0)
    {
        const size_t destPixelCount =
            m_paramsCurrent.m_cDepthWidth * m_paramsCurrent.m_cDepthHeight;

        // Convert from floating point depth if required
        if (NUI_FUSION_IMAGE_TYPE_FLOAT == imageFrame->imageType)
        {
            BYTE * rgbrun = buffer;
            FLOAT * pBufferRun = reinterpret_cast<FLOAT *>(LockedRect.pBits);
            FLOAT * pBufferEnd = pBufferRun + destPixelCount;

            // Depth ranges set here for better visualization, and map to black at 0 and white at 4m
            FLOAT range = 4.0f;
            FLOAT minRange = 0.0f;

            while ( pBufferRun < pBufferEnd )
            {
                // discard the portion of the depth that contains only the player index
                FLOAT depth = *pBufferRun;

                // Note: Using conditionals in this loop could degrade performance.
                // Consider using a lookup table instead when writing production code.
                BYTE intensity = (depth >= minRange) ?
                    static_cast<BYTE>( (int)(((depth - minRange) / range) * 256.0f) % 256 ) :
                    0; // % 256 to enable it to wrap around after the max range

                // Write out blue byte
                *(rgbrun++) = intensity;

                // Write out green byte
                *(rgbrun++) = intensity;

                // Write out red byte
                *(rgbrun++) = intensity;

                // We're outputting BGR, the last byte in the 32 bits is unused so skip it
                // If we were outputting BGRA, we would write alpha here.
                ++rgbrun;

                // Increment our index into the Kinect's depth buffer
                ++pBufferRun;
            }
        }
        else	// already in 4 bytes per int (RGBA/BGRA) format
        {
            BYTE * pBuffer = (BYTE *)LockedRect.pBits;

            // Draw the data with Direct2D
            memcpy_s(
                buffer,
                destPixelCount * KinectFusionParams::BytesPerPixel,
                pBuffer,
                imageFrame->width * imageFrame->height * KinectFusionParams::BytesPerPixel);
        }
    }
    else
    {
        return E_NOINTERFACE;
    }

    // We're done with the texture so unlock it
    imageFrameTexture->UnlockRect(0);

    return hr;
}

/// <summary>
/// Reset the reconstruction camera pose and clear the volume on the next frame.
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::ResetReconstruction()
{
    AssertOtherThread();

    EnterCriticalSection(&m_lockParams);
    m_bResetReconstruction = true;
    LeaveCriticalSection(&m_lockParams);

    return S_OK;
}

/// <summary>
/// Reset the reconstruction camera pose and clear the volume.
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessor::InternalResetReconstruction()
{
    AssertOwnThread();

    if (nullptr == m_pVolume)
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    SetIdentityMatrix(m_worldToCameraTransform);

    // Translate the reconstruction volume location away from the world origin by an amount equal
    // to the minimum depth threshold. This ensures that some depth signal falls inside the volume.
    // If set false, the default world origin is set to the center of the front face of the 
    // volume, which has the effect of locating the volume directly in front of the initial camera
    // position with the +Z axis into the volume along the initial camera direction of view.
    if (m_paramsCurrent.m_bTranslateResetPoseByMinDepthThreshold)
    {
        Matrix4 worldToVolumeTransform = m_defaultWorldToVolumeTransform;

        // Translate the volume in the Z axis by the minDepthThreshold distance
        float minDist = (m_paramsCurrent.m_fMinDepthThreshold < m_paramsCurrent.m_fMaxDepthThreshold) ? m_paramsCurrent.m_fMinDepthThreshold : m_paramsCurrent.m_fMaxDepthThreshold;
        worldToVolumeTransform.M43 -= (minDist * m_paramsCurrent.m_reconstructionParams.voxelsPerMeter);

        hr = m_pVolume->ResetReconstruction(&m_worldToCameraTransform, &worldToVolumeTransform);
    }
    else
    {
        hr = m_pVolume->ResetReconstruction(&m_worldToCameraTransform, nullptr);
    }

    m_cLostFrameCounter = 0;
    m_cFrameCounter = 0;
    m_fFrameCounterStartTime = m_timer.AbsoluteTime();

    EnterCriticalSection(&m_lockFrame);
    m_frame.m_fFramesPerSecond = 0;
    LeaveCriticalSection(&m_lockFrame);

    if (SUCCEEDED(hr))
    {
        m_bTrackingFailed = false;
        m_paramsCurrent.m_bPauseIntegration = false;
        m_bIntegrationResumed = true;
    }

    return hr;
}


/// <summary>
/// Calculate a mesh for the current volume
/// </summary>
/// <param name="ppMesh">returns the new mesh</param>
HRESULT KinectFusionProcessor::CalculateMesh(INuiFusionMesh** ppMesh)
{
    AssertOtherThread();

    EnterCriticalSection(&m_lockVolume);

    HRESULT hr = E_FAIL;

    if (m_pVolume != nullptr)
    {
        hr = m_pVolume->CalculateMesh(1, ppMesh);

        // Set the frame counter to 0 to prevent a reset reconstruction call due to large frame 
        // timestamp change after meshing. Also reset frame time for fps counter.
        m_cFrameCounter = 0;
        m_fFrameCounterStartTime =  m_timer.AbsoluteTime();
    }

    LeaveCriticalSection(&m_lockVolume);

    return hr;
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
void KinectFusionProcessor::SetStatusMessage(WCHAR * szMessage)
{
    AssertOwnThread();

    StringCchCopy(m_statusMessage, ARRAYSIZE(m_statusMessage), szMessage);
}

/// <summary>
/// Notifies the UI window that a new frame is ready
/// </summary>
void KinectFusionProcessor::NotifyFrameReady()
{
    AssertOwnThread();

    if (m_hWnd != nullptr && m_msgFrameReady != WM_NULL)
    {
        PostMessage(m_hWnd, m_msgFrameReady, 0, 0);
    }
}

/// <summary>
/// Notifies the UI window to update, even though there is no new frame data
/// </summary>
void KinectFusionProcessor::NotifyEmptyFrame()
{
    AssertOwnThread();

    EnterCriticalSection(&m_lockFrame);
    m_frame.m_fFramesPerSecond = 0;
    m_frame.SetStatusMessage(m_statusMessage);
    LeaveCriticalSection(&m_lockFrame);

    NotifyFrameReady();
}
