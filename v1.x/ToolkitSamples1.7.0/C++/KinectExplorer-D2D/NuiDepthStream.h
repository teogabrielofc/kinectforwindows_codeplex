//------------------------------------------------------------------------------
// <copyright file="NuiDepthStream.h" company="Microsoft">
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

#include "NuiStream.h"
#include "NuiImageBuffer.h"

class NuiDepthStream : public NuiStream
{
public:
    /// <summary>
    /// Constructor
    /// <summary>
    /// <param name="pNuiSensor">The pointer to NUI sensor device instance</param>
    NuiDepthStream(INuiSensor* pNuiSensor);

    /// <summary>
    /// Destructor
    /// </summary>
   ~NuiDepthStream();

public:
    /// <summary>
    /// Attach viewer object to stream object
    /// </summary>
    /// <param name="pStreamViewer">The pointer to viewer object to attach</param>
    /// <returns>Previously attached viewer object. If none, returns nullptr</returns>
    virtual NuiStreamViewer* SetStreamViewer(NuiStreamViewer* pStreamViewer);

    /// <summary>
    /// Start stream processing.
    /// </summary>
    /// <returns>Indicate success or failure.</returns>
    virtual HRESULT StartStream();

    /// <summary>
    /// Open stream with a certain image resolution.
    /// </summary>
    /// <param name="resolution">Frame image resolution</param>
    /// <returns>Indicates success or failure.</returns>
    HRESULT OpenStream(NUI_IMAGE_RESOLUTION resolution);

    /// <summary>
    /// Process an incoming stream frame
    /// </summary>
    virtual void ProcessStreamFrame();

    /// <summary>
    /// Set and reset near mode
    /// </summary>
    /// <param name="nearMode">True to enable near mode. False to disable</param>
    void SetNearMode(bool nearMode);

    /// <summary>
    /// Set depth treatment
    /// </summary>
    /// <param name="treatment">Depth treatment mode to set</param>
    void SetDepthTreatment(DEPTH_TREATMENT treatment);

private:
    /// <summary>
    /// Retrieve depth data from stream frame
    /// </summary>
    void ProcessDepth();

private:
    bool            m_nearMode;
    NUI_IMAGE_TYPE  m_imageType;
    NuiImageBuffer  m_imageBuffer;
    DEPTH_TREATMENT m_depthTreatment;
};
