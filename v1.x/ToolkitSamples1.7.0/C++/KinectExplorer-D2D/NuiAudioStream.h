//------------------------------------------------------------------------------
// <copyright file="NuiAudioStream.h" company="Microsoft">
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
#include "NuiAudioViewer.h"
#include "StaticMediaBuffer.h"

class NuiAudioStream
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="pNuiSensor">The pointer to Nui sensor object</param>
    NuiAudioStream(INuiSensor* pNuiSensor);

    /// <summary>
    /// Destructor
    /// </summary>
   ~NuiAudioStream();

public:
    /// <summary>
    /// Attach stream viewer
    /// </summary>
    /// <param name="pViewer">The pointer to the viewer to attach</param>
    void SetStreamViewer(NuiAudioViewer* pViewer);

    /// <summary>
    /// Start processing stream
    /// </summary>
    /// <returns>Indicates success or failure</returns>
    HRESULT StartStream();
    
    /// <summary>
    /// Get the audio readings from the stream
    /// </summary>
    void ProcessStream();

private:
    INuiSensor*         m_pNuiSensor;
    INuiAudioBeam*      m_pNuiAudioSource;
    IMediaObject*       m_pDMO;
    IPropertyStore*     m_pPropertyStore;
    NuiAudioViewer*     m_pAudioViewer;
    CStaticMediaBuffer  m_captureBuffer;
};
