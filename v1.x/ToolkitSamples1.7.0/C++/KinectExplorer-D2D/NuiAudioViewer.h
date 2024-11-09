//------------------------------------------------------------------------------
// <copyright file="NuiAudioViewer.h" company="Microsoft">
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

class NuiAudioViewer : public NuiViewer
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="pParent">The pointer to parent window</param>
    NuiAudioViewer(const NuiViewer* pParent);

    /// <summary>
    /// Destructor
    /// </summary>
   ~NuiAudioViewer();

public:
    /// <summary>
    /// Set and update audio readings to display
    /// </summary>
    /// <param name="beamAngle">Beam angle reading</param>
    /// <param name="sourceAngle">Source angle reading</param>
    /// <param name="sourceConfidence">Source confidence reading</param>
    void SetAudioReadings(double beamAngle, double sourceAngle, double sourceConfidence);

private:
    /// <summary>
    /// Returns the ID of the dialog
    /// </summary>
    /// <returns>ID of dialog</returns>
    virtual UINT GetDlgId();

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

private:
    double  m_beamAngle;
    double  m_sourceAngle;
    double  m_sourceConfidence;
};
