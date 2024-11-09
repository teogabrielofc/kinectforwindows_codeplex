//------------------------------------------------------------------------------
// <copyright file="Camera.cpp" company="Microsoft">
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

#include "Camera.h"

/// <summary>
/// Constructor
/// </summary>
CCamera::CCamera()
{
    Reset();
}

/// <summary>
/// Handles window messages, used to process input
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CCamera::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(hWnd);

    switch(uMsg)
    {
        case WM_KEYDOWN:
        {
            int nKey = static_cast<int>(wParam);

            if (nKey == 'Q' || nKey == VK_LEFT)
            {
                m_yaw -= m_rotationSpeed;
            }
            else if (nKey == 'E' || nKey == VK_RIGHT)
            {
                m_yaw += m_rotationSpeed;
            }
            else if (nKey == 'R' || nKey == VK_UP)
            {
                m_pitch -= m_rotationSpeed;
            }
            else if (nKey == 'F' || nKey == VK_DOWN)
            {
                m_pitch += m_rotationSpeed;
            }
            else if (nKey == 'A')
            {
               m_eye -= m_right * m_movementSpeed;
            }
            else if (nKey == 'D')
            {
               m_eye += m_right * m_movementSpeed;
            }
            else if (nKey == 'S')
            {
                m_eye -= m_forward * m_movementSpeed;
            }
            else if (nKey == 'W')
            {
                 m_eye += m_forward * m_movementSpeed;
            }
            else if (nKey == VK_SPACE)
            {
                Reset();
            }

            break;
        }
    }

    return 0;
}

/// <summary>
/// Reset the camera state to initial values
/// </summary>
void CCamera::Reset()
{
    m_rotationSpeed = .0125f;
    m_movementSpeed = .03f;

    View       = XMMatrixIdentity();

    m_eye      = XMVectorSet(0.f, 0.f, -1.5f, 0.f);
    m_at       = XMVectorSet(0.f, 0.f,  10.f, 0.f);
    m_up       = XMVectorSet(0.f, 1.f,   0.f, 0.f);
    m_forward  = XMVectorSet(0.f, 0.f,   1.f, 0.f);
    m_right    = XMVectorSet(1.f, 0.f,   0.f, 0.f);

    m_atBasis  = XMVectorSet(0.f, 0.f,   1.f, 0.f);
    m_upBasis  = XMVectorSet(0.f, 1.f,   0.f, 0.f);

    m_yaw      = 0.f;
    m_pitch    = 0.f;
}

/// <summary>
/// Update the view matrix
/// </summary>
void CCamera::Update()
{
    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);

    m_at      = XMVector4Transform(m_atBasis, rotation);
    m_up      = XMVector4Transform(m_upBasis, rotation);
    m_forward = XMVector4Normalize(m_at);
    
    m_right   = XMVector3Cross(m_up, m_forward);
    m_right   = XMVector4Normalize(m_right);

    //take into account player position so they're always looking forward
    m_at += m_eye;

    View = XMMatrixLookAtLH(m_eye, m_at, m_up);
}
