//------------------------------------------------------------------------------
// <copyright file="DrawDevice.h" company="Microsoft">
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

// Manages the drawing of bitmap data

#pragma once

#include <d2d1.h>

class DrawDevice
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    DrawDevice();

    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~DrawDevice();

    /// <summary>
    /// Set the window to draw to as well as the video format
    /// Implied bits per pixel is 32
    /// </summary>
    /// <param name="hWnd">window to draw to</param>
    /// <param name="pD2DFactory">already created D2D factory object</param>
    /// <param name="sourceWidth">width (in pixels) of image data to be drawn</param>
    /// <param name="sourceHeight">height (in pixels) of image data to be drawn</param>
    /// <param name="sourceStride">length (in bytes) of a single scanline</param>
    /// <returns>true if successful, false otherwise</returns>
    bool Initialize( HWND hwnd, ID2D1Factory * pD2DFactory, int sourceWidth, int sourceHeight, int sourceStride );

    /// <summary>
    /// Draws a 32 bit per pixel image of previously specified width, height, and stride to the associated hwnd
    /// </summary>
    /// <param name="pImage">image data in RGBX format</param>
    /// <param name="cbImage">size of image data in bytes</param>
    /// <returns>true if successful, false otherwise</returns>
    bool Draw( BYTE * pImage, unsigned long cbImage );

private:
    HWND                     m_hWnd;

    // Format information
    UINT                     m_sourceHeight;
    UINT                     m_sourceWidth;
    LONG                     m_sourceStride;

     // Direct2D 
    ID2D1Factory *           m_pD2DFactory;
    ID2D1HwndRenderTarget *  m_pRenderTarget;
    ID2D1Bitmap *            m_pBitmap;

    /// <summary>
    /// Ensure necessary Direct2d resources are created
    /// </summary>
    /// <returns>S_OK if successful, otherwise an error code</returns>
    HRESULT EnsureResources( );

    /// <summary>
    /// Dispose of Direct2d resources 
    /// </summary>
    void DiscardResources( );
};
