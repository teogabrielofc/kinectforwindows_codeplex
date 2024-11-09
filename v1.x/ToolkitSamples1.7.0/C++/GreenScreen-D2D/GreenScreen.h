//------------------------------------------------------------------------------
// <copyright file="GreenScreen.h" company="Microsoft">
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

#pragma once

#include "resource.h"
#include "NuiApi.h"
#include "ImageRenderer.h"

#include <NuiSensorChooser.h>
#include "NuiSensorChooserUI.h"

class CGreenScreen
{
    static const int        cBytesPerPixel    = 4;

    static const NUI_IMAGE_RESOLUTION cDepthResolution = NUI_IMAGE_RESOLUTION_320x240;
    
    // green screen background will also be scaled to this resolution
    static const NUI_IMAGE_RESOLUTION cColorResolution = NUI_IMAGE_RESOLUTION_640x480;

    static const int        cStatusMessageMaxLen = MAX_PATH*2;

public:
    /// <summary>
    /// Constructor
    /// </summary>
    CGreenScreen();

    /// <summary>
    /// Destructor
    /// </summary>
    ~CGreenScreen();

    /// <summary>
    /// Handles window messages, passes most to the class instance to handle
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Handle windows messages for a class instance
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    LRESULT CALLBACK        DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Creates the main window and begins processing
    /// </summary>
    /// <param name="hInstance"></param>
    /// <param name="nCmdShow"></param>
    int                     Run(HINSTANCE hInstance, int nCmdShow);

    /// <summary>
    /// Called on Kinect device status changed. It will update the sensor chooser UI control
    /// based on the new sensor status. It may also updates the sensor instance if needed
    /// </summary>
    static void CALLBACK StatusChangeCallback(HRESULT hrStatus, const OLECHAR* instancename, const OLECHAR* uniqueDeviceName, void* pUserData);

private:
    HWND                    m_hWnd;

    bool                    m_bNearMode;

    // Current Kinect
    INuiSensor*             m_pNuiSensor;

    // Direct2D
    ImageRenderer*          m_pDrawGreenScreen;
    ID2D1Factory*           m_pD2DFactory;
    
    HANDLE                  m_pDepthStreamHandle;
    HANDLE                  m_hNextDepthFrameEvent;

    HANDLE                  m_pColorStreamHandle;
    HANDLE                  m_hNextColorFrameEvent;

    LONG                    m_depthWidth;
    LONG                    m_depthHeight;

    LONG                    m_colorWidth;
    LONG                    m_colorHeight;

    LONG                    m_colorToDepthDivisor;

    USHORT*                 m_depthD16;
    BYTE*                   m_colorRGBX;
    BYTE*                   m_backgroundRGBX;
    BYTE*                   m_outputRGBX;
    LONG*                   m_colorCoordinates;

    LARGE_INTEGER           m_depthTimeStamp;
    LARGE_INTEGER           m_colorTimeStamp;

    NuiSensorChooser*       m_pSensorChooser;
    NuiSensorChooserUI*     m_pSensorChooserUI;

    /// <summary>
    /// Load an image from a resource into a buffer
    /// </summary>
    /// <param name="resourceName">name of image resource to load</param>
    /// <param name="resourceType">type of resource to load</param>
    /// <param name="cOutputBuffer">size of output buffer, in bytes</param>
    /// <param name="outputBuffer">buffer that will hold the loaded image</param>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                 LoadResourceImage(PCWSTR resourceName, PCWSTR resourceType, DWORD cOutputBuffer, BYTE* outputBuffer);

    /// <summary>
    /// Main processing function
    /// </summary>
    void                    Update();

    /// <summary>
    /// Create the first connected Kinect found 
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                 CreateFirstConnected();

    /// <summary>
    /// Handle new depth data
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                 ProcessDepth();

    /// <summary>
    /// Handle new color data
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                 ProcessColor();

    /// <summary>
    /// Set the status bar message
    /// </summary>
    /// <param name="szMessage">message to display</param>
    void                    SetStatusMessage(WCHAR* szMessage);

    /// <summary>
    /// Update the sensor and status based on the input changed flags
    /// </summary>
    /// <param name="changedFlags">The device change flags</param>
    void UpdateSensorAndStatus(DWORD changedFlags);

    /// <summary>
    /// Update the Nui Sensor Chooser UI control status
    /// </summary>
    void UpdateNscControlStatus();
};
