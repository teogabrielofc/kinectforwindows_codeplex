//------------------------------------------------------------------------------
// <copyright file="NuiAccelerometerStream.h" company="Microsoft">
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
#include "NuiAccelerometerViewer.h"

class NuiAccelerometerStream
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="pNuiSensor">The pointer to Nui sensor object</param>
    NuiAccelerometerStream(INuiSensor* pNuiSensor);

    /// <summary>
    /// Destructor
    /// </summary>
   ~NuiAccelerometerStream();

public:
    /// <summary>
    /// Attach stream viewer
    /// </summary>
    /// <param name="pViewer">The pointer to the viewer to attach</param>
    void SetStreamViewer(NuiAccelerometerViewer* pViewer);

    /// <summary>
    /// Get accelerometer reading
    /// </summary>
    void ProcessStream();

    /// <summary>
    /// Start processing stream
    /// </summary>
    /// <returns>Always returns S_OK</returns>
    HRESULT StartStream();

private:
    INuiSensor*             m_pNuiSensor;
    NuiAccelerometerViewer* m_pAccelerometerViewer;
};
