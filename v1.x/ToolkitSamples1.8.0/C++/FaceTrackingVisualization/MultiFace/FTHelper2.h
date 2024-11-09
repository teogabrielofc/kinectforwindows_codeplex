//------------------------------------------------------------------------------
// <copyright file="FTHelper2.h" company="Microsoft">
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
#include "KinectSensor.h"

struct FTHelperContext
{
    IFTFaceTracker*     m_pFaceTracker;
    IFTResult*          m_pFTResult;
    FT_VECTOR3D         m_hint3D[2];
    bool                m_LastTrackSucceeded;
    int                 m_CountUntilFailure;
    UINT                m_SkeletonId;
};

typedef void (*FTHelper2CallBack)(PVOID lpParam, UINT userId);
typedef void (*FTHelper2UserSelectCallBack)(PVOID lpParam, KinectSensor * pKinectSensor, UINT nbUsers, FTHelperContext* pUserContexts);

class FTHelper2
{
public:
    FTHelper2();
    ~FTHelper2();

    HRESULT Init(HWND hWnd, UINT nbUsers, FTHelper2CallBack callBack, PVOID callBackParam, FTHelper2UserSelectCallBack userSelectCallBack, PVOID userSelectCallBackParam,
        NUI_IMAGE_TYPE depthType, NUI_IMAGE_RESOLUTION depthRes, BOOL bNearMode, NUI_IMAGE_TYPE colorType, NUI_IMAGE_RESOLUTION colorRes, BOOL bSeatedSkeletonMode);
    HRESULT Stop();
    IFTResult* GetResult(UINT userId)       { return(m_UserContext[userId].m_pFTResult);}
    BOOL IsKinectPresent()                  { return(m_KinectSensorPresent);}
    IFTImage* GetColorImage()               { return(m_colorImage);}
    float GetXCenterFace()                  { return(m_XCenterFace);}
    float GetYCenterFace()                  { return(m_YCenterFace);}
    void SetDrawMask(BOOL drawMask)         { m_DrawMask = drawMask;}
    BOOL GetDrawMask()                      { return(m_DrawMask);}
    IFTFaceTracker* GetTracker(UINT userId) { return(m_UserContext[userId].m_pFaceTracker);}
    HRESULT GetCameraConfig(FT_CAMERA_CONFIG* cameraConfig);

private:
    KinectSensor                m_KinectSensor;
    BOOL                        m_KinectSensorPresent;
    UINT                        m_nbUsers;
    FTHelperContext*            m_UserContext;
    HWND                        m_hWnd;
    IFTImage*                   m_colorImage;
    IFTImage*                   m_depthImage;
    bool                        m_ApplicationIsRunning;
    FTHelper2CallBack           m_CallBack;
    LPVOID                      m_CallBackParam;
    FTHelper2UserSelectCallBack m_UserSelectCallBack;
    LPVOID                      m_UserSelectCallBackParam;
    float                       m_XCenterFace;
    float                       m_YCenterFace;
    HANDLE                      m_hFaceTrackingThread;
    BOOL                        m_DrawMask;
    NUI_IMAGE_TYPE              m_depthType;
    NUI_IMAGE_RESOLUTION        m_depthRes;
    BOOL                        m_bNearMode;
    NUI_IMAGE_TYPE              m_colorType;
    NUI_IMAGE_RESOLUTION        m_colorRes;
    BOOL						m_bSeatedSkeleton;


    BOOL SubmitFraceTrackingResult(IFTResult* pResult, UINT userId);
    void SetCenterOfImage(IFTResult* pResult);
    void CheckCameraInput();
    DWORD WINAPI FaceTrackingThread();
    static DWORD WINAPI FaceTrackingStaticThread(PVOID lpParam);
    static void SelectUserToTrack(KinectSensor * pKinectSensor, UINT nbUsers, FTHelperContext* pUserContexts);
};
