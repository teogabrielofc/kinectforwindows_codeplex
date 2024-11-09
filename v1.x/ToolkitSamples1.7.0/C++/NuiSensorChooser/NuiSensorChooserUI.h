//------------------------------------------------------------------------------
// <copyright file="NuiSensorChooserUI.h" company="Microsoft">
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

#include <Windows.h>

//
// The NSC control contains a subclassed icon image control to show current sensor status.
// It also uses a popup control to show more details about current sensor status when the mouse
// is hovering over it.
//

#define NSCN_REFRESH        1

class NuiSensorChooserUI
{
public:

    NuiSensorChooserUI(HWND hParent, UINT controlId, const POINT& ptLeftTop);

public:
    /// <summary>
    /// This method will update the corresponding children controls to reflect the sensor status change.
    /// </summary>
    /// <param name="dwStatus"> The current status of the sensor. </param>
    void UpdateSensorStatus(const DWORD dwStatus);

private:

    class NscIconControl* m_pControl;
};
