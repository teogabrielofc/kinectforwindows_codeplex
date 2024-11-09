//------------------------------------------------------------------------------
// <copyright file="NuiStream.h" company="Microsoft">
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
#include "NuiStreamViewer.h"
#include "Utility.h"

class NuiStream
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="pNuiSensor">The pointer to Nui sensor device instance</param>
    NuiStream(INuiSensor * pNuiSensor);

    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~NuiStream();

public:
    /// <summary>
    /// Attach viewer object to stream object
    /// </summary>
    /// <param name="pStreamViewer">The pointer to viewer object to attach</param>
    /// <returns>Previously attached viewer object. If none, returns nullptr</returns>
    virtual NuiStreamViewer* SetStreamViewer(NuiStreamViewer* pStreamViewer);

    /// <summary>
    /// Subclass should override this method to process the next incoming
    /// stream frame when stream event is set.
    /// </summary>
    virtual void ProcessStreamFrame() = 0;

    /// <summary>
    /// Pause the stream
    /// </summary>
    /// <param name="pause">Pause or resume the stream</param>
    virtual void PauseStream(bool pause);

    /// <summary>
    /// Subclass should override this method to start the stream processing.
    /// </summary>
    virtual HRESULT StartStream() = 0;

public:
    /// <summary>
    /// Get stream frame ready event handle
    /// </summary>
    /// <returns>Handle to event</returns>
    HANDLE GetFrameReadyEvent();

protected:
    NuiStreamViewer*    m_pStreamViewer;
    INuiSensor*         m_pNuiSensor;

    bool                m_paused;
    HANDLE              m_hStreamHandle;
    HANDLE              m_hFrameReadyEvent;
};
