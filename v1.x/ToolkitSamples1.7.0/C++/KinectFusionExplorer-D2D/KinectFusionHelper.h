//------------------------------------------------------------------------------
// <copyright file="KinectFusionHelper.h" company="Microsoft">
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

#include <NuiKinectFusionApi.h>

/// <summary>
/// Set Identity in a Matrix4
/// </summary>
/// <param name="mat">The matrix to set to identity</param>
void SetIdentityMatrix(Matrix4 &mat);

/// <summary>
/// Extract translation values from the 4x4 Matrix4 transformation in M41,M42,M43
/// </summary>
/// <param name="transform">The transform matrix.</param>
/// <param name="translation">Array of 3 floating point values for translation.</param>
void ExtractVector3Translation(const Matrix4 &transform, _Out_cap_c_(3) float *translation);

/// <summary>
/// Extract translation Vector3 from the 4x4 Matrix transformation in M41,M42,M43
/// </summary>
/// <param name="transform">The transform matrix.</param>
/// <returns>Returns a Vector3 containing the translation.</returns>
Vector3 ExtractVector3Translation(const Matrix4 &transform);

/// <summary>
/// Extract 3x3 rotation from the 4x4 Matrix and return in new Matrix4
/// </summary>
/// <param name="transform">The transform matrix.</param>
/// <returns>Returns a Matrix4 containing the rotation.</returns>
Matrix4 Extract3x3Rotation(const Matrix4 &transform);

/// <summary>
/// Extract 3x3 rotation matrix from the Matrix4 4x4 transformation:
/// Then convert to Euler angles.
/// </summary>
/// <param name="transform">The transform matrix.</param>
/// <param name="rotation">Array of 3 floating point values for euler angles.</param>
void ExtractRot2Euler(const Matrix4 &transform, _Out_cap_c_(3) float *rotation);

/// <summary>
/// Test whether the camera moved too far between sequential frames by looking at starting and end transformation matrix.
/// We assume that if the camera moves or rotates beyond a reasonable threshold, that we have lost track.
/// Note that on lower end machines, if the processing frame rate decreases below 30Hz, this limit will potentially have
/// to be increased as frames will be dropped and hence there will be a greater motion between successive frames.
/// </summary>
/// <param name="T_initial">The transform matrix from the previous frame.</param>
/// <param name="T_final">The transform matrix from the current frame.</param>
/// <param name="maxTrans">The maximum translation in meters we expect per x,y,z component between frames under normal motion.</param>
/// <param name="maxRotDegrees">The maximum rotation in degrees we expect about the x,y,z axes between frames under normal motion.</param>
/// <returns>true if camera transformation is greater than the threshold, otherwise false</returns>
bool CameraTransformFailed(const Matrix4 &T_initial, const Matrix4 &T_final, float maxTrans, float maxRotDegrees);

/// <summary>
/// Invert the 3x3 Rotation Matrix Component of a 4x4 matrix
/// </summary>
/// <param name="rot">The rotation matrix to invert.</param>
void InvertRotation(Matrix4 &rot);

/// <summary>
/// Negate the 3x3 Rotation Matrix Component of a 4x4 matrix
/// </summary>
/// <param name="rot">The rotation matrix to negate.</param>
void NegateRotation(Matrix4 &rot);

/// <summary>
/// Rotate a vector with the 3x3 Rotation Matrix Component of a 4x4 matrix
/// </summary>
/// <param name="vec">The Vector3 to rotate.</param>
/// <param name="rot">Rotation matrix.</param>
Vector3 RotateVector(const Vector3 &vec, const Matrix4 &rot);

/// <summary>
/// Invert Matrix4 Pose either from WorldToCameraTransform (view) matrix to CameraToWorldTransform pose matrix (world/SE3) or vice versa
/// </summary>
/// <param name="transform">The camera pose transform matrix.</param>
/// <returns>Returns a Matrix4 containing the inverted camera pose.</returns>
Matrix4 InvertMatrix4Pose(const Matrix4 &transform);

/// <summary>
/// Write ASCII Wavefront .OBJ mesh file
/// See http://en.wikipedia.org/wiki/Wavefront_.obj_file for .OBJ format
/// </summary>
/// <param name="mesh">The Kinect Fusion mesh object.</param>
/// <param name="lpOleFileName">The full path and filename of the file to save.</param>
/// <param name="flipYZ">Flag to determine whether the Y and Z values are flipped on save.</param>
/// <returns>indicates success or failure</returns>
HRESULT WriteAsciiObjMeshFile(INuiFusionMesh *mesh, LPOLESTR lpOleFileName, bool flipYZ = true);

/// <summary>
/// Write Binary .STL mesh file
/// see http://en.wikipedia.org/wiki/STL_(file_format) for STL format
/// </summary>
/// <param name="mesh">The Kinect Fusion mesh object.</param>
/// <param name="lpOleFileName">The full path and filename of the file to save.</param>
/// <param name="flipYZ">Flag to determine whether the Y and Z values are flipped on save.</param>
/// <returns>indicates success or failure</returns>
HRESULT WriteBinarySTLMeshFile(INuiFusionMesh *mesh, LPOLESTR lpOleFileName, bool flipYZ = true);

/// <summary>
/// Convert int to string
/// </summary>
/// <param name="theValue">The int value to convert.</param>
/// <returns>Returns a string containing the int value.</returns>
inline std::string to_string(int theValue)
{
    char buffer[65];

    errno_t err = _itoa_s(theValue, buffer, ARRAYSIZE(buffer), 10);

    if (0 != err)
    {
        return std::string("");
    }

    return std::string(buffer);
}

/// <summary>
/// Convert float to string
/// </summary>
/// <param name="theValue">The float value to convert.</param>
/// <returns>Returns a string containing the float value.</returns>
inline std::string to_string(float theValue)
{
    char buffer[_CVTBUFSIZE];

    errno_t err = _gcvt_s(buffer, _CVTBUFSIZE, theValue, 6);

    if (0 != err)
    {
        return std::string("");
    }

    return std::string(buffer);
}

/// <summary>
/// Clamp a value if outside two given thresholds
/// </summary>
/// <param name="x">The value to clamp.</param>
/// <param name="a">The minimum inclusive threshold.</param>
/// <param name="b">The maximum inclusive threshold.</param>
/// <returns>Returns the clamped value.</returns>
template <typename T>
inline T clamp(const T& x, const T& a, const T& b)
{
    if (x < a)
        return a;
    else if (x > b)
        return b;
    else
        return x;
}
