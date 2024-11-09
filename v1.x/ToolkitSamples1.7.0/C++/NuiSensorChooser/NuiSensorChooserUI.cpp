//------------------------------------------------------------------------------
// <copyright file="NuiSensorChooserUI.cpp" company="Microsoft">
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

#include "NuiSensorChooserUI.h"
#include "NuiSensorChooserUIPrivate.h"

NuiSensorChooserUI::NuiSensorChooserUI(
    HWND hParent,
    UINT controlId,
    const POINT& ptCenterTop
    )
{
    m_pControl = new NscIconControl(hParent, controlId, ptCenterTop);
}

/// <summary>
/// This method will update its children controls to reflect the sensor status change.
/// </summary>
/// <param name="dwStatus"> The current sensor status. </param>
void NuiSensorChooserUI::UpdateSensorStatus(const DWORD dwStatus)
{
    m_pControl->UpdateSensorStatus(dwStatus);
}
