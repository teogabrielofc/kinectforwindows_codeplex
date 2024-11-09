//------------------------------------------------------------------------------
// <copyright file="OrbitCamera.cpp" company="Microsoft">
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

#include "OrbitCamera.h"

/// <summary>
/// Constructor
/// </summary>
CCamera::CCamera()
{
    Reset();
}

/// <summary>
/// Move camera into position
/// </summary>
int CCamera::UpdatePosition()
{
	m_eye = XMVectorSet(r * sinf(theta) * cosf(phi), r * sinf(phi), -r * cosf(theta) * cosf(phi), 0.0F);

	return 0;
}

/// <summary>
/// Reset the camera state to initial values
/// </summary>
void CCamera::Reset()
{
    View       = XMMatrixIdentity();

    m_eye      = XMVectorSet(0.f, 0.f, -0.3f, 0.f);
    m_at       = XMVectorSet(0.f, 0.f,  1.0f, 0.f);
    m_up       = XMVectorSet(0.f, 1.f,   0.f, 0.f);
}

/// <summary>
/// Update the view matrix
/// </summary>
void CCamera::Update()
{
    View = XMMatrixLookAtLH(m_eye + m_at, m_at, m_up);
}

/// <summary>
/// Sets the center depth of the rendered image
/// </summary>
void CCamera::SetCenterDepth(float depth)
{
	m_at = XMVectorSet(0.0f, 0.0f, depth, 0.0f);
}

/// <summary>
/// Sets the R value of the camera from the depth center
/// R value represents the distance of the camera from the players
/// </summary>
void CCamera::SetRadius(float r)
{
	this->r = r;
	UpdatePosition();
}

/// <summary>
/// Sets the Theta value of the camera from around the depth center
/// Theta represents the angle (in radians) of the camera around the 
/// center in the x-y plane (circling around players)
/// </summary>
void CCamera::SetTheta(float theta)
{
	this->theta = theta;
	UpdatePosition();
}

/// <summary>
/// Sets the Phi value of the camera
/// Phi represents angle (in radians) of the camera around the center 
/// in the y-z plane (over the top and below players)
/// </summary>
void CCamera::SetPhi(float phi)
{
	this->phi = phi;
	UpdatePosition();
}
