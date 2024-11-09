//-----------------------------------------------------------------------------
// <copyright file="FrameRateTracker.h" company="Microsoft">
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
//-----------------------------------------------------------------------------

#pragma once

#include <ctime>
#include <algorithm>
#include <windows.h>

class FrameRateTracker {
public:
    // Functions:
    /// <summary>
    /// Constructor
    /// </summary>
    FrameRateTracker();

    /// <summary>
    /// Call once per frame to update the frame rate
    /// </summary>
    void Tick();

    /// <summary>
    /// Get the current frame rate
    /// </summary>
    /// <returns>The current frame rate</returns>
    const int CurrentFPS();

private:
    // Variables
    // The clock tick from the last time the fps was calculated
    clock_t m_previousClock;

    // The current frame count
    DWORD m_frameCount;

    // The previous frame count
    DWORD m_previousFrameCount;

    // The current frame rate
    int m_fps;
};
