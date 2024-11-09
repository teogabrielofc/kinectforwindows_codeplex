//------------------------------------------------------------------------------
// <copyright file="CameraColorSettingsViewer.h" company="Microsoft">
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

#include "CameraSettingsViewer.h"

class CameraColorSettingsViewer : public CameraSettingsViewer
{
public:

    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="pParent">Pointer to the parent viewer.</param>
    CameraColorSettingsViewer(const NuiViewer* pParent);

protected:

    /// <summary>
    /// Set range of all the track bar controls
    /// </summary>
    void SetTrackBarsRange();

    /// <summary>
    /// Reset the sensor parameters
    /// </summary>
    void ResetSensorParameters();

    /// <summary>
    /// Update the chilid controls based on the setting values
    /// </summary>
    void UpdateControl();

    /// <summary>
    /// Enable/Disable some child controls based on "auto white balance" option
    /// </summary>
    void UpdateControlEnableStatus();

    /// <summary>
    /// Handle the WM_COMMAND message
    /// </summary>
    /// <param name="controlId">Id of the control that triggers the message.</param>
    void OnCommand(UINT controlId);

    /// <summary>
    /// Handle the WM_HSCROLL message
    /// </summary>
    /// <param name="controlId">Id of the control that triggers the message.</param>
    void OnScroll(UINT controlId);

    /// <summary>
    /// Set auto white balance
    /// </summary>
    /// <param name="autoWhiteBalanceEnabled">Indicate whether enable auto white balance option.</param>
    void SetAutoWhiteBalance(BOOL autoWhiteBalanceEnabled);
};
