//------------------------------------------------------------------------------
// <copyright file="KinectSettings.h" company="Microsoft">
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

#include "NuiStreamViewer.h"
#include "NuiColorStream.h"
#include "NuiDepthStream.h"
#include "NuiSkeletonStream.h"
#include "CameraSettingsViewer.h"

class KinectSettings
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="pNuiSensor">The pointer to NUI sensor instance</param>
    /// <param name="pPrimaryView">The pointer to primary viewer instance</param>
    /// <param name="pSecondaryView">The pointer to secondary viewer instance</param>
    /// <param name="pColorStream">The pointer to color stream object instance</param>
    /// <param name="pDepthStream">The pointer to depth stream object instance</param>
    /// <param name="pSkeletonStream">The pointer to skeleton stream object instance</param>
    KinectSettings(INuiSensor* pNuiSensor, NuiStreamViewer* pPrimaryView, NuiStreamViewer* pSecondarView, NuiColorStream* pColorStream, NuiDepthStream* pDepthStream, NuiSkeletonStream* pSkeletonStream, CameraSettingsViewer* pColorSettingsView, CameraSettingsViewer* pExposureSettingsView);

    /// <summary>
    /// Destructor
    /// </summary>
   ~KinectSettings();

public:
    /// <summary>
    /// Process Kinect window menu commands
    /// </summary>
    /// <param name="commanId">ID of the menu item</param>
    /// <param name="param">Parameter passed in along with the commmand ID</param>
    /// <param name="previouslyChecked">Check status of menu item before command is issued</param>
    void ProcessMenuCommand(WORD commandId, WORD param, bool previouslyChecked);

private:
    INuiSensor*              m_pNuiSensor;
    // Stream viewers
    NuiStreamViewer*         m_pPrimaryView;
    NuiStreamViewer*         m_pSecondaryView;

    // Streams
    NuiColorStream*          m_pColorStream;
    NuiDepthStream*          m_pDepthStream;
    NuiSkeletonStream*       m_pSkeletonStream;

    // Camera settings
    CameraSettingsViewer*     m_pColorSettingsView;
    CameraSettingsViewer*     m_pExposureSettingsView;
};
