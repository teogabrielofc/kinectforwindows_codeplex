//------------------------------------------------------------------------------
// <copyright file="NuiTiltAngleViewer.h" company="Microsoft">
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

#include <NuiApi.h>
#include "Utility.h"
#include "NuiViewer.h"

class NuiTiltAngleViewer : public NuiViewer
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="pParent">The pointer to parent window</param>
    /// <param name="pNuiSensor">The pointer to Nui sensor device instance</param>
    NuiTiltAngleViewer(const NuiViewer* pParent, INuiSensor* pNuiSensor);

    /// <summary>
    /// Destructor
    /// </summary>
   ~NuiTiltAngleViewer();

private:
    /// <summary>
    /// Dispatch window message to message handlers.
    /// </summary>
    /// <param name="hWnd">Handle to window</param>
    /// <param name="uMsg">Message type</param>
    /// <param name="wParam">Extra message parameter</param>
    /// <param name="lParam">Extra message parameter</param>
    /// <returns>
    /// If message is handled, non-zero is returned. Otherwise FALSE is returned and message is passed to default dialog procedure
    /// </returns>
    virtual LRESULT DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Returns the ID of the dialog
    /// </summary>
    /// <returns>ID of dialog</returns>
    virtual UINT GetDlgId();

    /// <summary>
    /// Handler for WM_INITDIALOG message
    /// </summary>
    /// <param name="hWnd">The handle to the window which receives the message</param>
    void OnInit(HWND hWnd);

    /// <summary>
    /// Handler for WM_COMMAND message
    /// </summary>
    /// <param name="hWnd">The handle to the window which receives the message</param>
    /// <param name = "wParam">Command parameter</param>
    void OnCommand(HWND hWnd, WPARAM wParam);

    /// <summary>
    /// Handler for slider control scroll event
    /// </summary>
    /// <param name="hWnd">The handle to the window which receives the message</param>
    /// <param name="wParam">HIWORD specifies the current position of the slider if LOWORD is SB_THUMBPOSIION or SB_THUMBTRACK. Otherwise not used</param>
    /// <param name="lParam">If the message is sent by a scroll bar, this parameter is the handle to the scroll bar control. If the message is not sent by a scroll bar, this parameter is NULL</param>
    void OnSliderScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Start a new thread to run the elevation task
    /// </summary>
    void StartElevationThread();

    /// <summary>
    /// The thread procedure in which the elevation task runs
    /// </summary>
    /// <param name="pThis">The pointer to NuiTiltAngleViewer instance</param>
    static DWORD WINAPI ThreadProc(NuiTiltAngleViewer* pThis);

    /// <summary>
    /// Release all the resources
    /// </summary>
    void CleanUp();

private:
    INuiSensor* m_pNuiSensor;
    LONG m_tiltAngle;

    HANDLE m_hSetTiltAngleEvent;
    HANDLE m_hExitThreadEvent;

    // Handle to the elevation task thread
    HANDLE m_hElevationTaskThread;
};
