//------------------------------------------------------------------------------
// <copyright file="CoordinateMappingBasics.cpp" company="Microsoft">
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

#include "CoordinateMappingBasics.h"
#include "resource.h"

#include <Wincodec.h>
#include <assert.h>

#include <NuiSensorChooser.h>
#include <NuiSensorChooserUI.h>

#define WM_SENSORCHANGED WM_USER + 1

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    CCoordinateMappingBasics application;
    application.Run(hInstance, nCmdShow);
}

/// <summary>
/// Constructor
/// </summary>
CCoordinateMappingBasics::CCoordinateMappingBasics() :
    m_pD2DFactory(NULL),
    m_pDrawCoordinateMappingBasics(NULL),
    m_hNextDepthFrameEvent(INVALID_HANDLE_VALUE),
    m_hNextColorFrameEvent(INVALID_HANDLE_VALUE),
    m_pDepthStreamHandle(INVALID_HANDLE_VALUE),
    m_pColorStreamHandle(INVALID_HANDLE_VALUE),
    m_bNearMode(false),
    m_pNuiSensor(NULL),
    m_pSensorChooser(NULL),
    m_pSensorChooserUI(NULL)
{
    // get resolution as DWORDS, but store as LONGs to avoid casts later
    DWORD width = 0;
    DWORD height = 0;

    NuiImageResolutionToSize(cDepthResolution, width, height);
    m_depthWidth  = static_cast<LONG>(width);
    m_depthHeight = static_cast<LONG>(height);

    NuiImageResolutionToSize(cColorResolution, width, height);
    m_colorWidth  = static_cast<LONG>(width);
    m_colorHeight = static_cast<LONG>(height);

    m_colorToDepthDivisor = m_colorWidth/m_depthWidth;

    m_depthTimeStamp.QuadPart = 0;
    m_colorTimeStamp.QuadPart = 0;

    // create heap storage for depth pixel data in RGBX format
    m_depthD16 = new USHORT[m_depthWidth*m_depthHeight];
    m_colorCoordinates = new LONG[m_depthWidth*m_depthHeight*2];

    m_colorRGBX = new BYTE[m_colorWidth*m_colorHeight*cBytesPerPixel];
    m_backgroundRGBX = new BYTE[m_colorWidth*m_colorHeight*cBytesPerPixel];
    m_outputRGBX = new BYTE[m_colorWidth*m_colorHeight*cBytesPerPixel];

    // Create an event that will be signaled when depth data is available
    m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Create an event that will be signaled when color data is available
    m_hNextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

/// <summary>
/// Destructor
/// </summary>
CCoordinateMappingBasics::~CCoordinateMappingBasics()
{
    // clean up NSC sensor chooser and its UI control
    delete m_pSensorChooser;
    delete m_pSensorChooserUI;

    if (m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hNextDepthFrameEvent);
    }

    if (m_hNextColorFrameEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hNextColorFrameEvent);
    }

    // clean up Direct2D renderer
    delete m_pDrawCoordinateMappingBasics;
    m_pDrawCoordinateMappingBasics = NULL;

    // done with pixel data
    delete[] m_depthD16;
    delete[] m_colorCoordinates;

    delete[] m_colorRGBX;
    delete[] m_backgroundRGBX;
    delete[] m_outputRGBX;

    // clean up Direct2D
    SafeRelease(m_pD2DFactory);

    SafeRelease(m_pNuiSensor);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int CCoordinateMappingBasics::Run(HINSTANCE hInstance, int nCmdShow)
{
    MSG       msg = {0};
    WNDCLASS  wc;

    // Dialog custom window class
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.lpfnWndProc   = DefDlgProcW;
    wc.lpszClassName = L"CoordinateMappingBasicsAppDlgWndClass";

    if (!RegisterClassW(&wc))
    {
        return 0;
    }

    // Create main application window
    HWND hWndApp = CreateDialogParamW(
        hInstance,
        MAKEINTRESOURCE(IDD_APP),
        NULL,
        (DLGPROC)CCoordinateMappingBasics::MessageRouter, 
        reinterpret_cast<LPARAM>(this));

    // Set the init sensor status
    UpdateNscControlStatus();

    // Show window
    ShowWindow(hWndApp, nCmdShow);
    UpdateWindow(hWndApp);

    const int eventCount = 2;
    HANDLE hEvents[eventCount];

    LoadResourceImage(L"Background", L"Image", m_colorWidth*m_colorHeight*cBytesPerPixel, m_backgroundRGBX);

    // Main message loop
    while (WM_QUIT != msg.message)
    {
        hEvents[0] = m_hNextDepthFrameEvent;
        hEvents[1] = m_hNextColorFrameEvent;

        // Check to see if we have either a message (by passing in QS_ALLINPUT)
        // Or a Kinect event (hEvents)
        // Update() will check for Kinect events individually, in case more than one are signalled
        MsgWaitForMultipleObjects(eventCount, hEvents, FALSE, INFINITE, QS_ALLINPUT);

        // Explicitly check the Kinect frame event since MsgWaitForMultipleObjects
        // can return for other reasons even though it is signaled.
        Update();

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // If a dialog message will be taken care of by the dialog proc
            if ((hWndApp != NULL) && IsDialogMessageW(hWndApp, &msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

/// <summary>
/// This function will be called when Kinect device status changed
/// </summary>
void CALLBACK CCoordinateMappingBasics::StatusChangeCallback(HRESULT hrStatus, const OLECHAR* instancename, const OLECHAR* uniqueDeviceName, void* pUserData)
{
    HWND hWnd = reinterpret_cast<HWND>(pUserData);

    if (NULL != hWnd)
    {
        SendMessage(hWnd, WM_SENSORCHANGED, 0, 0);
    }
}

/// <summary>
/// Main processing function
/// </summary>
void CCoordinateMappingBasics::Update()
{
    if (NULL == m_pNuiSensor)
    {
        return;
    }

    bool needToDraw = false;

    if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextDepthFrameEvent, 0) )
    {
        // if we have received any valid new depth data we may need to draw
        if ( SUCCEEDED(ProcessDepth()) )
        {
            needToDraw = true;
        }
    }

    if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextColorFrameEvent, 0) )
    {
        // if we have received any valid new color data we may need to draw
        if ( SUCCEEDED(ProcessColor()) )
        {
            needToDraw = true;
        }
    }

    // Depth is 30 fps.  For any given combination of FPS, we should ensure we are within half a frame of the more frequent of the two.  
    // But depth is always the greater (or equal) of the two, so just use depth FPS.
    const int depthFps = 30;
    const int halfADepthFrameMs = (1000 / depthFps) / 2;

    // If we have not yet received any data for either color or depth since we started up, we shouldn't draw
    if (m_colorTimeStamp.QuadPart == 0 || m_depthTimeStamp.QuadPart == 0)
    {
        needToDraw = false;
    }

    // If the color frame is more than half a depth frame ahead of the depth frame we have,
    // then we should wait for another depth frame.  Otherwise, just go with what we have.
    if (m_colorTimeStamp.QuadPart - m_depthTimeStamp.QuadPart > halfADepthFrameMs)
    {
        needToDraw = false;
    }

    if (needToDraw)
    {
        int outputIndex = 0;
        LONG* pDest;
        LONG* pSrc;

        // loop over each row and column of the color
        for (LONG y = 0; y < m_colorHeight; ++y)
        {
            for (LONG x = 0; x < m_colorWidth; ++x)
            {
                // calculate index into depth array
                int depthIndex = x/m_colorToDepthDivisor + y/m_colorToDepthDivisor * m_depthWidth;

                USHORT depth  = m_depthD16[depthIndex];
                USHORT player = NuiDepthPixelToPlayerIndex(depth);

                // default setting source to copy from the background pixel
                pSrc  = (LONG *)m_backgroundRGBX + outputIndex;

                // if we're tracking a player for the current pixel, draw from the color camera
                if ( player > 0 )
                {
                    // retrieve the depth to color mapping for the current depth pixel
                    LONG colorInDepthX = m_colorCoordinates[depthIndex * 2];
                    LONG colorInDepthY = m_colorCoordinates[depthIndex * 2 + 1];

                    // make sure the depth pixel maps to a valid point in color space
                    if ( colorInDepthX >= 0 && colorInDepthX < m_colorWidth && colorInDepthY >= 0 && colorInDepthY < m_colorHeight )
                    {
                        // calculate index into color array
                        LONG colorIndex = colorInDepthX + colorInDepthY * m_colorWidth;

                        // set source for copy to the color pixel
                        pSrc  = (LONG *)m_colorRGBX + colorIndex;
                    }
                }

                // calculate output pixel location
                pDest = (LONG *)m_outputRGBX + outputIndex++;

                // write output
                *pDest = *pSrc;
            }
        }

        // Draw the data with Direct2D
        m_pDrawCoordinateMappingBasics->Draw(m_outputRGBX, m_colorWidth * m_colorHeight * cBytesPerPixel);
    }
}

/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CCoordinateMappingBasics::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCoordinateMappingBasics* pThis = NULL;
    
    if (WM_INITDIALOG == uMsg)
    {
        pThis = reinterpret_cast<CCoordinateMappingBasics*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<CCoordinateMappingBasics*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->DlgProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CCoordinateMappingBasics::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Bind application window handle
            m_hWnd = hWnd;

            // Create NuiSensorChooser UI control
            RECT rc;
            GetClientRect(m_hWnd, &rc);

            POINT ptCenterTop;
            ptCenterTop.x = (rc.right - rc.left)/2;
            ptCenterTop.y = 0;

            // Create the sensor chooser UI control to show sensor status
            m_pSensorChooserUI = new NuiSensorChooserUI(m_hWnd, IDC_SENSORCHOOSER, ptCenterTop);

            // Set the sensor status callback
            NuiSetDeviceStatusCallback(StatusChangeCallback, reinterpret_cast<void*>(m_hWnd));
            // Init the sensor chooser to find a valid sensor
            m_pSensorChooser = new NuiSensorChooser();

            // Init Direct2D
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

            // Create and initialize a new Direct2D image renderer (take a look at ImageRenderer.h)
            // We'll use this to draw the data we receive from the Kinect to the screen
            m_pDrawCoordinateMappingBasics = new ImageRenderer();
            HRESULT hr = m_pDrawCoordinateMappingBasics->Initialize(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), m_pD2DFactory, m_colorWidth, m_colorHeight, m_colorWidth * sizeof(long));
            if (FAILED(hr))
            {
                SetStatusMessage(L"Failed to initialize the Direct2D draw device.");
            }

            // Look for a connected Kinect, and create it if found
            CreateFirstConnected();
        }
        break;

        // If the titlebar X is clicked, destroy app
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            // Quit the main message pump
            PostQuitMessage(0);
            break;

        // Handle button press
        case WM_COMMAND:
            // If it was for the near mode control and a clicked event, change near mode
            if (IDC_CHECK_NEARMODE == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
            {
                // Toggle out internal state for near mode
                m_bNearMode = !m_bNearMode;

                if (NULL != m_pNuiSensor)
                {
                    // Set near mode based on our internal state
                    m_pNuiSensor->NuiImageStreamSetImageFrameFlags(m_pDepthStreamHandle, m_bNearMode ? NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE : 0);
                }
            }
            break;

        case WM_NOTIFY:
        {
            const NMHDR* pNMHeader = reinterpret_cast<const NMHDR*>(lParam);
            if (pNMHeader->code == NSCN_REFRESH && pNMHeader->idFrom == IDC_SENSORCHOOSER)
            {
                // Handle refresh notification sent from NSC UI control
                DWORD dwChangeFlags;

                HRESULT hr = m_pSensorChooser->TryResolveConflict(&dwChangeFlags);
                if (SUCCEEDED(hr))
                {
                    UpdateSensorAndStatus(dwChangeFlags);
                }
            }

            return TRUE;
        }
        break;

        case WM_SENSORCHANGED:
        {
            if (NULL != m_pSensorChooser)
            {
                // Handle sensor status change event
                DWORD dwChangeFlags = 0;

                HRESULT hr = m_pSensorChooser->HandleNuiStatusChanged(&dwChangeFlags);
                if (SUCCEEDED(hr))
                {
                    UpdateSensorAndStatus(dwChangeFlags);
                }
            }
        }
        break;
    }

    return FALSE;
}

/// <summary>
/// Create the first connected Kinect found
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT CCoordinateMappingBasics::CreateFirstConnected()
{
    // Get the Kinect and specify that we'll be using depth
    HRESULT hr = m_pSensorChooser->GetSensor(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_COLOR, &m_pNuiSensor);

    if (SUCCEEDED(hr) && NULL != m_pNuiSensor)
    {
        // Open a depth image stream to receive depth frames
        hr = m_pNuiSensor->NuiImageStreamOpen(
            NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
            cDepthResolution,
            0,
            2,
            m_hNextDepthFrameEvent,
            &m_pDepthStreamHandle);

        // Open a color image stream to receive depth frames
        hr = m_pNuiSensor->NuiImageStreamOpen(
            NUI_IMAGE_TYPE_COLOR,
            cColorResolution,
            0,
            2,
            m_hNextColorFrameEvent,
            &m_pColorStreamHandle);
    }
    else
    {
        // Reset all the event to nonsignaled state
        ResetEvent(m_hNextDepthFrameEvent);
        ResetEvent(m_hNextColorFrameEvent);
    }

    if (NULL == m_pNuiSensor || FAILED(hr))
    {
        SetStatusMessage(L"No ready Kinect found!");
        return E_FAIL;
    }
    else
    {
        SetStatusMessage(L"Kinect found!");
    }

    m_pNuiSensor->NuiSkeletonTrackingDisable();

    return hr;
}

/// <summary>
/// Handle new depth data
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT CCoordinateMappingBasics::ProcessDepth()
{
    HRESULT hr = S_OK;
    NUI_IMAGE_FRAME imageFrame;

    // Attempt to get the depth frame
    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
    if (FAILED(hr))
    {
        return hr;
    }

    m_depthTimeStamp = imageFrame.liTimeStamp;

    INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect(0, &LockedRect, NULL, 0);

    // Make sure we've received valid data
    if (LockedRect.Pitch != 0)
    {
        memcpy(m_depthD16, LockedRect.pBits, LockedRect.size);
    }

    // We're done with the texture so unlock it
    pTexture->UnlockRect(0);

    // Release the frame
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);

    // Get of x, y coordinates for color in depth space
    // This will allow us to later compensate for the differences in location, angle, etc between the depth and color cameras
    m_pNuiSensor->NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(
        cColorResolution,
        cDepthResolution,
        m_depthWidth*m_depthHeight,
        m_depthD16,
        m_depthWidth*m_depthHeight*2,
        m_colorCoordinates
        );

    return hr;
}

/// <summary>
/// Handle new color data
/// </summary>
/// <returns>S_OK for success or error code</returns>
HRESULT CCoordinateMappingBasics::ProcessColor()
{
    HRESULT hr = S_OK;
    NUI_IMAGE_FRAME imageFrame;

    // Attempt to get the depth frame
    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pColorStreamHandle, 0, &imageFrame);
    if (FAILED(hr))
    {
        return hr;
    }

    m_colorTimeStamp = imageFrame.liTimeStamp;

    INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect(0, &LockedRect, NULL, 0);

    // Make sure we've received valid data
    if (LockedRect.Pitch != 0)
    {
        memcpy(m_colorRGBX, LockedRect.pBits, LockedRect.size);
    }

    // We're done with the texture so unlock it
    pTexture->UnlockRect(0);

    // Release the frame
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_pColorStreamHandle, &imageFrame);

    return hr;
}

/// <summary>
/// Load an image from a resource into a buffer
/// </summary>
/// <param name="resourceName">name of image resource to load</param>
/// <param name="resourceType">type of resource to load</param>
/// <param name="cOutputBuffer">size of output buffer, in bytes</param>
/// <param name="outputBuffer">buffer that will hold the loaded image</param>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT CCoordinateMappingBasics::LoadResourceImage(
    PCWSTR resourceName,
    PCWSTR resourceType,
    DWORD cOutputBuffer,
    BYTE* outputBuffer
    )
{
    HRESULT hr = S_OK;

    IWICImagingFactory* pIWICFactory = NULL;
    IWICBitmapDecoder* pDecoder = NULL;
    IWICBitmapFrameDecode* pSource = NULL;
    IWICStream* pStream = NULL;
    IWICFormatConverter* pConverter = NULL;
    IWICBitmapScaler* pScaler = NULL;

    HRSRC imageResHandle = NULL;
    HGLOBAL imageResDataHandle = NULL;
    void *pImageFile = NULL;
    DWORD imageFileSize = 0;

    hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&pIWICFactory);
    if ( FAILED(hr) ) return hr;

    // Locate the resource.
    imageResHandle = FindResourceW(HINST_THISCOMPONENT, resourceName, resourceType);
    hr = imageResHandle ? S_OK : E_FAIL;

    if (SUCCEEDED(hr))
    {
        // Load the resource.
        imageResDataHandle = LoadResource(HINST_THISCOMPONENT, imageResHandle);
        hr = imageResDataHandle ? S_OK : E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        // Lock it to get a system memory pointer.
        pImageFile = LockResource(imageResDataHandle);
        hr = pImageFile ? S_OK : E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        // Calculate the size.
        imageFileSize = SizeofResource(HINST_THISCOMPONENT, imageResHandle);
        hr = imageFileSize ? S_OK : E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        // Create a WIC stream to map onto the memory.
        hr = pIWICFactory->CreateStream(&pStream);
    }

    if (SUCCEEDED(hr))
    {
        // Initialize the stream with the memory pointer and size.
        hr = pStream->InitializeFromMemory(
            reinterpret_cast<BYTE*>(pImageFile),
            imageFileSize
            );
    }

    if (SUCCEEDED(hr))
    {
        // Create a decoder for the stream.
        hr = pIWICFactory->CreateDecoderFromStream(
            pStream,
            NULL,
            WICDecodeMetadataCacheOnLoad,
            &pDecoder
            );
    }

    if (SUCCEEDED(hr))
    {
        // Create the initial frame.
        hr = pDecoder->GetFrame(0, &pSource);
    }

    if (SUCCEEDED(hr))
    {
        // Convert the image format to 32bppPBGRA
        // (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
        hr = pIWICFactory->CreateFormatConverter(&pConverter);
    }

    if (SUCCEEDED(hr))
    {
        hr = pIWICFactory->CreateBitmapScaler(&pScaler);
    }

    if (SUCCEEDED(hr))
    {
        hr = pScaler->Initialize(
            pSource,
            m_colorWidth,
            m_colorHeight,
            WICBitmapInterpolationModeCubic
            );
    }

    if (SUCCEEDED(hr))
    {
        hr = pConverter->Initialize(
            pScaler,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            NULL,
            0.f,
            WICBitmapPaletteTypeMedianCut
            );
    }

    UINT width = 0;
    UINT height = 0;
    if (SUCCEEDED(hr))
    {
        hr = pConverter->GetSize(&width, &height);
    }

    // make sure the output buffer is large enough
    if (SUCCEEDED(hr))
    {
        if ( width*height*cBytesPerPixel > cOutputBuffer )
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pConverter->CopyPixels(NULL, width*cBytesPerPixel, cOutputBuffer, outputBuffer);
    }

    SafeRelease(pScaler);
    SafeRelease(pConverter);
    SafeRelease(pSource);
    SafeRelease(pDecoder);
    SafeRelease(pStream);
    SafeRelease(pIWICFactory);

    return hr;
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
void CCoordinateMappingBasics::SetStatusMessage(WCHAR * szMessage)
{
    SendDlgItemMessageW(m_hWnd, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szMessage);
}

/// <summary>
/// Update the sensor and status based on the input changeg flags
/// </summary>
void CCoordinateMappingBasics::UpdateSensorAndStatus(DWORD changedFlags)
{
    switch(changedFlags)
    {
    case NUISENSORCHOOSER_SENSOR_CHANGED_FLAG:
        {
            // Free the previous sensor and try to get a new one
            SafeRelease(m_pNuiSensor);
            CreateFirstConnected();
        }

    case NUISENSORCHOOSER_STATUS_CHANGED_FLAG:
        UpdateNscControlStatus();
        break;
    }
}

/// <summary>
/// Update the Nui Sensor Chooser UI control status
/// </summary>
void CCoordinateMappingBasics::UpdateNscControlStatus()
{
    assert(m_pSensorChooser != NULL);
    assert(m_pSensorChooserUI != NULL);

    DWORD dwStatus;
    HRESULT hr = m_pSensorChooser->GetStatus(&dwStatus);

    if (SUCCEEDED(hr))
    {
        m_pSensorChooserUI->UpdateSensorStatus(dwStatus);
    }
}
