//-----------------------------------------------------------------------------
// <copyright file="FrameRateTracker.cpp" company="Microsoft">
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

#include "FrameRateTracker.h"

// Functions:
/// <summary>
/// Constructor
/// </summary>
FrameRateTracker::FrameRateTracker(): m_frameCount(0), m_previousFrameCount(0), m_fps(0)
{
    m_previousClock = clock();
}

/// <summary>
/// Call once per frame to update the frame rate tracker's internal history
/// about how long it took to render the current frame.
/// </summary>
void FrameRateTracker::Tick() {
    m_frameCount++;

    // Calculate how long it took to render the current frame
    clock_t currentClock = clock();
    clock_t milliseconds = (currentClock - m_previousClock) / (CLOCKS_PER_SEC / 1000);

    // Update the frame rate every 1 second
    if (milliseconds >= 1000)
    {
        m_fps = (UINT)((double)(m_frameCount - m_previousFrameCount) * 1000 / milliseconds);
        m_previousClock = currentClock;
        m_previousFrameCount = m_frameCount;
    }
}

/// <summary>
/// Get the current frame rate
/// </summary>
/// <returns>The current frame rate</returns>
const int FrameRateTracker::CurrentFPS() {
    return m_fps;
}
