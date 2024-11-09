//------------------------------------------------------------------------------
// <copyright file="Camera.h" company="Microsoft">
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
    /// Handles window messages, used to process input
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Reset the camera state to initial values
    /// </summary>
    void Reset();

    /// <summary>
    /// Update the view matrix
    /// </summary>
    void Update();

    /// <summary>
    /// Get the camera's up vector
    /// </summary>
    /// <returns>camera's up vector</returns>
    XMVECTOR  GetUp() { return m_up; }

    /// <summary>
    /// Get the camera's right vector
    /// </summary>
    /// <returns>camera's right vector</returns>
    XMVECTOR  GetRight() { return m_right; }

    /// <summary>
    /// Get the camera's position vector
    /// </summary>
    /// <returns>camera's position vector</returns>
    XMVECTOR  GetEye() { return m_eye; }

private:
    float     m_rotationSpeed;
    float     m_movementSpeed;

    float     m_yaw;
    float     m_pitch;

    XMVECTOR  m_eye;
    XMVECTOR  m_at;
    XMVECTOR  m_up;
    XMVECTOR  m_forward;
    XMVECTOR  m_right;

    XMVECTOR  m_atBasis;
    XMVECTOR  m_upBasis;
};
