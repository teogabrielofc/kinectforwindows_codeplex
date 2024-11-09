//------------------------------------------------------------------------------
// <copyright file="OrbitCamera.h" company="Microsoft">
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

#include <windows.h>
#include <xnamath.h>

class CCamera
{
public:
    XMMATRIX View;

    /// <summary>
    /// Constructor
    /// </summary>
    CCamera();

    /// <summary>
    /// Reset the camera state to initial values
    /// </summary>
    void Reset();

    /// <summary>
    /// Update the view matrix
    /// </summary>
    void Update();

    /// <summary>
    /// Move camera into position
    /// </summary>
    int UpdatePosition();

    /// <summary>
    /// Sets the R value of the camera.
    /// R value represents the distance of the camera from the center
    /// </summary>
    void SetRadius(float r);

    /// <summary>
    /// Sets the Theta value of the camera from around the depth center
    /// Theta represents the angle (in radians) of the camera around the 
    /// center in the x-y plane (circling around players)
    /// </summary>
    void SetTheta(float theta);

    /// <summary>
    /// Sets the Phi value of the camera
    /// Phi represents angle (in radians) of the camera around the center 
    /// in the y-z plane (over the top and below players)
    /// </summary>
    void SetPhi(float phi);

    /// <summary>
    /// Get the camera's up vector
    /// </summary>
    /// <returns>camera's up vector</returns>
    XMVECTOR  GetUp() { return m_up; }

    /// <summary>
    /// Get the camera's position vector
    /// </summary>
    /// <returns>camera's position vector</returns>
    XMVECTOR  GetEye() { return m_eye; }

    /// <summary>
    /// Sets the center depth of the rendered image
    /// </summary>
    void SetCenterDepth(float depth);

private:
    float r;
    float theta;
    float phi;

    XMVECTOR  m_eye;
    XMVECTOR  m_at;
    XMVECTOR  m_up;
};
