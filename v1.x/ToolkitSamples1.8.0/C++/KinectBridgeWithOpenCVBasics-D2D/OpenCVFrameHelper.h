//-----------------------------------------------------------------------------
// <copyright file="OpenCVFrameHelper.h" company="Microsoft">
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
//-----------------------------------------------------------------------------

#pragma once
#include "KinectHelper.h"

// Suppress warnings that come from compiling OpenCV code since we have no control over it
#pragma warning(push)
#pragma warning(disable : 6294 6031)
#include <opencv2/core/core.hpp>
#pragma warning(pop)

using namespace cv;
namespace Microsoft {
    namespace KinectBridge {
        class OpenCVFrameHelper : public KinectHelper<Mat> {
        public:
            // Functions:
            /// <summary>
            /// Constructor
            /// </summary>
            OpenCVFrameHelper() : KinectHelper<Mat>() {}

            /// <summary>
            /// Destructor
            /// </summary>
            ~OpenCVFrameHelper() {}

            // Constants
            // Mat type for each usage
            static const int COLOR_TYPE = CV_8UC4;
            static const int DEPTH_TYPE = CV_16U;
            static const int DEPTH_RGB_TYPE = CV_8UC4;

        protected:
            // Functions:
            /// <summary>
            /// Converts from Kinect color frame data into a RGB OpenCV image matrix
            /// </summary>
            /// <param name="pImage">pointer in which to return the OpenCV image matrix</param>
            /// <returns>S_OK if successful, an error code otherwise</returns>
            HRESULT GetColorData(Mat* pImage) const override;

            /// <summary>
            /// Converts from Kinect depth frame data into a OpenCV matrix
            /// </summary>
            /// <param name="pImage">pointer in which to return the OpenCV matrix</param>
            /// <returns>S_OK if successful, an error code otherwise</returns>
            HRESULT GetDepthData(Mat* pImage) const override;

            /// <summary>
            /// Converts from Kinect depth frame data into a ARGB OpenCV image matrix
            /// </summary>
            /// <param name="pImage">pointer in which to return the OpenCV image matrix</param>
            /// <returns>S_OK if successful, an error code otherwise</returns>
            HRESULT GetDepthDataAsArgb(Mat* pImage) const override;

            /// <summary>
            /// Verify image is of the given resolution
            /// </summary>
            /// <param name="pImage">pointer to image to verify</param>
            /// <param name="resolution">resolution of image</param>
            /// <returns>S_OK if image matches given width and height, an error code otherwise</returns>
            HRESULT VerifySize(const Mat* pImage, NUI_IMAGE_RESOLUTION resolution) const override;
        };
    }
}
