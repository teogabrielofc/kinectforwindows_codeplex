//------------------------------------------------------------------------------
// <copyright file="NuiAccelerometerViewer.h" company="Microsoft">
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

#include "Utility.h"
#include "NuiViewer.h"

class KinectWindow;

class NuiAccelerometerViewer : public NuiViewer
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="pParent">The pointer to parent window</param>
    NuiAccelerometerViewer(const NuiViewer* pParent);

    /// <summary>
    /// Destructor
    /// </summary>
   ~NuiAccelerometerViewer();

public:
    /// <summary>
    /// Set accelerometer readings to display
    /// </summary>
    /// <param name="x">Acceleration component on x-axis</param>
    /// <param name="y">Acceleration component on y-axis</param>
    /// <param name="z">Acceleration component on z-axis</param>
    void SetAccelerometerReadings(FLOAT x, FLOAT y, FLOAT z);

private:
    /// <summary>
    /// Returns the id of the dialog
    /// </summary>
    /// <returns>Id of dialog</returns>
    virtual UINT GetDlgId();

    /// <sumamry>
    /// Process message or send it to coreponding handler
    /// </summary>
    /// <param name="hWnd">The handle to the window which receives the message</param>
    /// <param name="uMsg">Message identifier</param>
    /// <param name="wParam">Additional message information</param>
    /// <param name="lParam">Additional message information</param>
    /// <returns>Result of message process. Depends on message type</returns>
    virtual LRESULT DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    // Acceleration readings on x, y, z axis
    FLOAT m_accelerationX;
    FLOAT m_accelerationY;
    FLOAT m_accelerationZ;
};
