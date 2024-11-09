//------------------------------------------------------------------------------
// <copyright file="KinectSensor.h" company="Microsoft">
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

#include <FaceTrackLib.h>
#include <NuiApi.h>

class KinectSensor
{
public:
    KinectSensor();
    ~KinectSensor();

    HRESULT Init(NUI_IMAGE_TYPE depthType, NUI_IMAGE_RESOLUTION depthRes, BOOL bNearMode, BOOL bFallbackToDefault, NUI_IMAGE_TYPE colorType, NUI_IMAGE_RESOLUTION colorRes, BOOL bSeatedSkeletonMode);
    void Release();

    HRESULT     GetVideoConfiguration(FT_CAMERA_CONFIG* videoConfig);
    HRESULT     GetDepthConfiguration(FT_CAMERA_CONFIG* depthConfig);

    IFTImage*   GetVideoBuffer(){ return(m_VideoBuffer); };
    IFTImage*   GetDepthBuffer(){ return(m_DepthBuffer); };
    float       GetZoomFactor() { return(m_ZoomFactor); };
    POINT*      GetViewOffSet() { return(&m_ViewOffset); };
    HRESULT     GetClosestHint(FT_VECTOR3D* pHint3D);

    bool        IsTracked(UINT skeletonId) { return(m_SkeletonTracked[skeletonId]);};
    FT_VECTOR3D NeckPoint(UINT skeletonId) { return(m_NeckPoint[skeletonId]);};
    FT_VECTOR3D HeadPoint(UINT skeletonId) { return(m_HeadPoint[skeletonId]);};

private:
    IFTImage*   m_VideoBuffer;
    IFTImage*   m_DepthBuffer;
    FT_VECTOR3D m_NeckPoint[NUI_SKELETON_COUNT];
    FT_VECTOR3D m_HeadPoint[NUI_SKELETON_COUNT];
    bool        m_SkeletonTracked[NUI_SKELETON_COUNT];
    FLOAT       m_ZoomFactor;   // video frame zoom factor (it is 1.0f if there is no zoom)
    POINT       m_ViewOffset;   // Offset of the view from the top left corner.

    HANDLE      m_hNextDepthFrameEvent;
    HANDLE      m_hNextVideoFrameEvent;
    HANDLE      m_hNextSkeletonEvent;
    HANDLE      m_pDepthStreamHandle;
    HANDLE      m_pVideoStreamHandle;
    HANDLE      m_hThNuiProcess;
    HANDLE      m_hEvNuiProcessStop;

    bool        m_bNuiInitialized; 
    int         m_FramesTotal;
    int         m_SkeletonTotal;
    
    static DWORD WINAPI ProcessThread(PVOID pParam);
    void GotVideoAlert();
    void GotDepthAlert();
    void GotSkeletonAlert();
};
