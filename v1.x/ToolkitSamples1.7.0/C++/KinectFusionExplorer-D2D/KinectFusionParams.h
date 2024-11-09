//------------------------------------------------------------------------------
// <copyright file="KinectFusionParams.h" company="Microsoft">
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
#include <NuiKinectFusionApi.h>

/// <summary>
/// Parameters to control the behavior of the KinectFusionProcessor.
/// </summary>
struct KinectFusionParams
{
    // Number of bytes per pixel (applies to both depth float and int-per-pixel raycast images)
    static const int            BytesPerPixel = 4;

    /// <summary>
    /// Constructor
    /// </summary>
    KinectFusionParams() :
        m_bPauseIntegration(false),
        m_bNearMode(true),
        m_imageResolution(NUI_IMAGE_RESOLUTION_640x480),
        m_bAutoResetReconstructionWhenLost(false),
        m_fMinDepthThreshold(NUI_FUSION_DEFAULT_MINIMUM_DEPTH),
        m_fMaxDepthThreshold(NUI_FUSION_DEFAULT_MAXIMUM_DEPTH),
        m_bMirrorDepthFrame(false),
        m_cMaxIntegrationWeight(NUI_FUSION_DEFAULT_INTEGRATION_WEIGHT),
        m_bDisplaySurfaceNormals(false),
        m_bTranslateResetPoseByMinDepthThreshold(true)
    {
        // Get the depth frame size from the NUI_IMAGE_RESOLUTION enum.
        // You can use NUI_IMAGE_RESOLUTION_640x480 or NUI_IMAGE_RESOLUTION_320x240 in this sample.
        // Smaller resolutions will be faster in per-frame computations, but show less detail in reconstructions.
        DWORD width = 0, height = 0;
        NuiImageResolutionToSize(m_imageResolution, width, height);
        m_cDepthWidth = width;
        m_cDepthHeight = height;
        m_cImageSize = m_cDepthWidth*m_cDepthHeight;

        // Define a cubic Kinect Fusion reconstruction volume, with the sensor at the center of the
        // front face and the volume directly in front of sensor.
        m_reconstructionParams.voxelsPerMeter = 256;    // 1000mm / 256vpm = ~3.9mm/voxel
        m_reconstructionParams.voxelCountX = 512;       // 512 / 256vpm = 2m wide reconstruction
        m_reconstructionParams.voxelCountY = 384;       // Memory = 512*384*512 * 4bytes per voxel
        m_reconstructionParams.voxelCountZ = 512;       // This will require a GPU with at least 512MB

        // This parameter sets whether GPU or CPU processing is used. Note that the CPU will likely be 
        // too slow for real-time processing.
        m_processorType = NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_AMP;

        // If GPU processing is selected, we can choose the index of the device we would like to
        // use for processing by setting this zero-based index parameter. Note that setting -1 will cause
        // automatic selection of the most suitable device (specifically the DirectX11 compatible device 
        // with largest memory), which is useful in systems with multiple GPUs when only one reconstruction
        // volume is required. Note that the automatic choice will not load balance across multiple 
        // GPUs, hence users should manually select GPU indices when multiple reconstruction volumes 
        // are required, each on a separate device.
        m_deviceIndex = -1;    // automatically choose device index for processing
    }

    /// <summary>
    /// Indicates whether the current reconstruction volume is different than the one in the params.
    /// </summary>
    bool VolumeChanged(const KinectFusionParams& params)
    {
        return
            m_reconstructionParams.voxelCountX != params.m_reconstructionParams.voxelCountX ||
            m_reconstructionParams.voxelCountY != params.m_reconstructionParams.voxelCountY ||
            m_reconstructionParams.voxelCountZ != params.m_reconstructionParams.voxelCountZ ||
            m_reconstructionParams.voxelsPerMeter != params.m_reconstructionParams.voxelsPerMeter ||
            m_processorType != params.m_processorType ||
            m_deviceIndex != params.m_deviceIndex;
    }

    /// <summary>
    /// Reconstruction Initialization parameters
    /// </summary>
    int                         m_deviceIndex;
    NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE m_processorType;

    /// <summary>
    /// Parameter to pause integration of new frames
    /// </summary>
    bool                        m_bPauseIntegration;

    /// <summary>
    /// Parameter to select the sensor's near mode
    /// </summary>
    bool                        m_bNearMode;

    /// <summary>
    /// Image Resolution and size
    /// </summary>
    NUI_IMAGE_RESOLUTION        m_imageResolution;
    int                         m_cDepthWidth;
    int                         m_cDepthHeight;
    int                         m_cImageSize;

    /// <summary>
    /// The Kinect Fusion Volume Parameters
    /// </summary>
    NUI_FUSION_RECONSTRUCTION_PARAMETERS m_reconstructionParams;

    /// <summary>
    /// Parameter to enable automatic reset of the reconstruction when camera tracking is lost.
    /// Set to true in the constructor to enable auto reset on cResetOnNumberOfLostFrames
    /// number of lost frames, or false to never automatically reset on loss of camera tracking.
    /// </summary>
    bool                        m_bAutoResetReconstructionWhenLost;

    /// <summary>
    /// Processing parameters
    /// </summary>
    float                       m_fMinDepthThreshold;
    float                       m_fMaxDepthThreshold;
    bool                        m_bMirrorDepthFrame;
    unsigned short              m_cMaxIntegrationWeight;
    bool                        m_bDisplaySurfaceNormals;

    /// <summary>
    /// Parameter to translate the reconstruction based on the minimum depth setting.
    /// When set to false, the reconstruction volume +Z axis starts at the camera lens and extends
    /// into the scene. Setting this true in the constructor will move the volume forward along +Z
    /// away from the camera by the minimum depth threshold to enable capture of very small
    /// reconstruction volumes by setting a non-identity camera transformation in the
    /// ResetReconstruction call.
    /// Small volumes may work better when shifted, as the Kinect hardware has a minimum sensing
    /// limit of ~0.35m, inside which no valid depth is returned, hence it is difficult to
    /// initialize and track robustly when the majority of a small volume is inside this distance.
    /// </summary>
    bool                        m_bTranslateResetPoseByMinDepthThreshold;
};
