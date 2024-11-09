// -----------------------------------------------------------------------
// <copyright file="FaceTrackInfo.cs" company="Microsoft">
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

namespace Microsoft.Samples.Kinect.KinectFusionHeadScanning
{
    using System;
    using System.Collections.Generic;
    using Microsoft.Kinect.Toolkit.FaceTracking;

    /// <summary>
    /// Struct represent tracking results for a face
    /// </summary>
    public struct FaceTrackInfo
    {
        /// <summary>
        /// Indicate the tracking data is valid or not
        /// </summary>
        public bool TrackValid { get; set; }

        /// <summary>
        /// Face rectangle in video frame coordinates
        /// </summary>
        public Rect FaceRect { get; set; }

        /// <summary>
        /// Translation in X, Y, Z axes
        /// </summary>
        public Vector3DF Translation { get; set; }

        /// <summary>
        /// Rotation around X, Y, Z axes
        /// </summary>
        public Vector3DF Rotation { get; set; }

        /// <summary>
        /// Override the equality operator
        /// </summary>
        /// <returns>Returns true if equivalent, otherwise false</returns>
        public static bool operator ==(FaceTrackInfo face1, FaceTrackInfo face2)
        {
            return face1.Equals(face2);
        }

        /// <summary>
        /// Override the inequality operator
        /// </summary>
        /// <returns>Returns true if not equivalent, otherwise false</returns>
        public static bool operator !=(FaceTrackInfo face1, FaceTrackInfo face2)
        {
            return !face1.Equals(face2);
        }

        /// <summary>
        /// Override the GetHashCode method
        /// </summary>
        /// <returns>Returns hash code.</returns>
        public override int GetHashCode()
        {
            return TrackValid.GetHashCode() ^ FaceRect.GetHashCode() ^ Translation.GetHashCode() ^ Rotation.GetHashCode();
        }

        /// <summary>
        /// Override the Equals method
        /// </summary>
        /// <returns>Returns true if not equivalent, otherwise false</returns>
        public override bool Equals(object obj)
        {
            if (!(obj is FaceTrackInfo))
            {
                return false;
            }

            return this.Equals((FaceTrackInfo)obj);
        }

        /// <summary>
        /// Equals method
        /// </summary>
        /// <returns>Returns true if not equivalent, otherwise false</returns>
        public bool Equals(FaceTrackInfo other)
        {
            if (this.TrackValid != other.TrackValid || this.FaceRect != other.FaceRect
                || this.Rotation != other.Rotation || this.Translation != other.Translation)
            {
                return false;
            }

            return true;
        }
    }
}
