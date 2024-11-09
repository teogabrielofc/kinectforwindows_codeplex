//------------------------------------------------------------------------------
// <copyright file="AudioPanel.h" company="Microsoft">
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

// Direct2D Header Files
#include <d2d1.h>

/// <summary>
/// Manages the drawing of audio data in audio panel that includes beam angle and
/// sound source angle gauges, and an oscilloscope visualization of audio data.
/// </summary>
/// <remarks>
/// Note that all panel elements are laid out directly in an {X,Y} coordinate space
/// where X and Y are both in [0.0,1.0] interval, and whole panel is later re-scaled
/// to fit available area via a scaling transform.
/// </remarks>
class AudioPanel
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    AudioPanel();

    /// <summary>
    /// Destructor
    /// </summary>
    virtual ~AudioPanel();

    /// <summary>
    /// Set the window to draw to as well as the amount of energy to expect to have to display.
    /// </summary>
    /// <param name="hWnd">window to draw to.</param>
    /// <param name="pD2DFactory">already created D2D factory object.</param>
    /// <param name="energyToDisplay">Number of energy samples to display at any given time.</param>
    /// <returns>S_OK on success, otherwise failure code.</returns>
    HRESULT Initialize(const HWND hwnd, ID2D1Factory* pD2DFactory, UINT energyToDisplay);

    /// <summary>
    /// Draws audio panel.
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code.</returns>
    HRESULT Draw();

    /// <summary>
    /// Update the beam angle being displayed in panel.
    /// </summary>
    /// <param name="beamAngle">new beam angle to display.</param>
    void SetBeam(const float & beamAngle);

    /// <summary>
    /// Update the sound source angle and confidence being displayed in panel.
    /// </summary>
    /// <param name="soundSourceAngle">new sound source angle to display.</param>
    /// <param name="soundSourceConfidence">new sound source confidence to display.</param>
    void SetSoundSource(const float & soundSourceAngle, const float & soundSourceConfidence);

    /// <summary>
    /// Update the audio energy being displayed in panel.
    /// </summary>
    /// <param name="pEnergy">new energy buffer to render.</param>
    /// <param name="energyLength">length of energy buffer to render.</param>
    void UpdateEnergy(const float *pEnergy, const UINT energyLength);

private:
    // Main application window
    HWND                        m_hWnd;

    // Direct2D objects
    ID2D1Factory*               m_pD2DFactory;
    ID2D1HwndRenderTarget*      m_pRenderTarget;
    D2D_MATRIX_3X2_F            m_RenderTargetTransform;
    UINT                        m_energyDisplayWidth;
    UINT                        m_energyDisplayHeight;
    BYTE*                       m_pEnergyBackground;
    UINT                        m_energyBackgroundStride;
    BYTE*                       m_pEnergyForeground;
    UINT                        m_energyForegroundStride;
    ID2D1Bitmap*                m_pEnergyDisplay;
    D2D1_RECT_F                 m_energyDisplayPosition;
    ID2D1PathGeometry*          m_pBeamGauge;
    ID2D1RadialGradientBrush*   m_pBeamGaugeFill;
    ID2D1PathGeometry*          m_pBeamNeedle;
    ID2D1LinearGradientBrush*   m_pBeamNeedleFill;
    D2D_MATRIX_3X2_F            m_BeamNeedleTransform;
    ID2D1PathGeometry*          m_pSourceGauge;
    ID2D1LinearGradientBrush*   m_pSourceGaugeFill;
    D2D_MATRIX_3X2_F            m_SourceGaugeTransform;
    ID2D1PathGeometry*          m_pPanelOutline;
    ID2D1SolidColorBrush*       m_pPanelOutlineStroke;

    /// <summary>
    /// Dispose of Direct2d resources.
    /// </summary>
    void DiscardResources( );

    /// <summary>
    /// Ensure necessary Direct2d resources are created.
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code.</returns>
    HRESULT EnsureResources();

    /// <summary>
    /// Create oscilloscope display for audio energy data.
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code.</returns>
    HRESULT CreateEnergyDisplay();

    /// <summary>
    /// Create gauge used to display beam angle.
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code.</returns>
    HRESULT CreateBeamGauge();

    /// <summary>
    /// Create gauge needle used to display beam angle.
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code.</returns>
    HRESULT CreateBeamGaugeNeedle();

    /// <summary>
    /// Create gauge (with position cloud) used to display sound source angle.
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code.</returns>
    HRESULT CreateSourceGauge();

    /// <summary>
    /// Create gradient used to represent sound source confidence, with the
    /// specified width.
    /// </summary>
    /// <param name="width">
    /// Width of gradient, specified in [0.0,1.0] interval.
    /// </param>
    /// <returns>S_OK on success, otherwise failure code.</returns>
    HRESULT CreateSourceGaugeFill(const float & width);

    /// <summary>
    /// Create outline that frames both gauges and energy display into a cohesive panel.
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code.</returns>
    HRESULT CreatePanelOutline();
};
