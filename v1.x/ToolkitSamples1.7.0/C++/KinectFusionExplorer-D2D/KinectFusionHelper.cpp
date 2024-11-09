//------------------------------------------------------------------------------
// <copyright file="KinectFusionHelper.cpp" company="Microsoft">
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

// System includes
#include "stdafx.h"

#define _USE_MATH_DEFINES
#include <math.h>

// Project includes
#include "KinectFusionHelper.h"

/// <summary>
/// Set Identity in a Matrix4
/// </summary>
/// <param name="mat">The matrix to set to identity</param>
void SetIdentityMatrix(Matrix4 &mat)
{
    mat.M11 = 1; mat.M12 = 0; mat.M13 = 0; mat.M14 = 0;
    mat.M21 = 0; mat.M22 = 1; mat.M23 = 0; mat.M24 = 0;
    mat.M31 = 0; mat.M32 = 0; mat.M33 = 1; mat.M34 = 0;
    mat.M41 = 0; mat.M42 = 0; mat.M43 = 0; mat.M44 = 1;
}

/// <summary>
/// Extract translation Vector3 from the Matrix4 4x4 transformation in M41,M42,M43
/// </summary>
/// <param name="transform">The transform matrix.</param>
/// <param name="translation">Array of 3 floating point values for translation.</param>
void ExtractVector3Translation(const Matrix4 &transform, _Out_cap_c_(3) float *translation)
{
    translation[0] = transform.M41;
    translation[1] = transform.M42;
    translation[2] = transform.M43;
}

/// <summary>
/// Extract translation Vector3 from the 4x4 Matrix in M41,M42,M43
/// </summary>
/// <param name="transform">The transform matrix.</param>
/// <returns>Returns a Vector3 containing the translation.</returns>
Vector3 ExtractVector3Translation(const Matrix4 &transform)
{
    Vector3 translation;
    translation.x = transform.M41;
    translation.y = transform.M42;
    translation.z = transform.M43;
    return translation;
}

/// <summary>
/// Extract 3x3 rotation from the 4x4 Matrix and return in new Matrix4
/// </summary>
/// <param name="transform">The transform matrix.</param>
/// <returns>Returns a Matrix4 containing the rotation.</returns>
Matrix4 Extract3x3Rotation(const Matrix4 &transform)
{
    Matrix4 rotation;

    rotation.M11 = transform.M11;
    rotation.M12 = transform.M12;
    rotation.M13 = transform.M13;
    rotation.M14 = 0;

    rotation.M21 = transform.M21;
    rotation.M22 = transform.M22;
    rotation.M23 = transform.M23;
    rotation.M24 = 0;

    rotation.M31 = transform.M31;
    rotation.M32 = transform.M32;
    rotation.M33 = transform.M33;
    rotation.M34 = 0;

    rotation.M41 = 0;
    rotation.M42 = 0;
    rotation.M43 = 0;
    rotation.M44 = 1;

    return rotation;
}

/// <summary>
/// Extract 3x3 rotation matrix from the Matrix4 4x4 transformation:
/// Then convert to Euler angles.
/// </summary>
/// <param name="transform">The transform matrix.</param>
/// <param name="rotation">Array of 3 floating point values for euler angles.</param>
void ExtractRot2Euler(const Matrix4 &transform, _Out_cap_c_(3) float *rotation)
{
    float phi = atan2f(transform.M23, transform.M33);
    float theta = asinf(-transform.M13);
    float psi = atan2f(transform.M12, transform.M11);

    rotation[0] = phi;	// This is rotation about x,y,z, or pitch, yaw, roll respectively
    rotation[1] = theta;
    rotation[2] = psi;
}

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
bool CameraTransformFailed(const Matrix4 &T_initial, const Matrix4 &T_final, float maxTrans, float maxRotDegrees)
{
    // Check if the transform is too far out to be reasonable 
    float deltaTrans = maxTrans;
    float angDeg = maxRotDegrees;
    float deltaRot = (angDeg * (float)M_PI) / 180.0f;

    // Calculate the deltas
    float eulerInitial[3];
    float eulerFinal[3];

    ExtractRot2Euler(T_initial, eulerInitial);
    ExtractRot2Euler(T_final, eulerFinal);

    float transInitial[3];
    float transFinal[3];

    ExtractVector3Translation(T_initial, transInitial);
    ExtractVector3Translation(T_final, transFinal);

    bool failRot = false;
    bool failTrans = false;

    float rDeltas[3];
    float tDeltas[3];

	static const float pi = static_cast<float>(M_PI);

    for (int i = 0; i < 3; i++)
    {
        // Handle when one angle is near PI, and the other is near -PI.
        if (eulerInitial[i] >= (pi - deltaRot) && eulerFinal[i] < (deltaRot - pi))
        {
            eulerInitial[i] -= pi * 2;
        }
        else if (eulerFinal[i] >= (pi - deltaRot) && eulerInitial[i] < (deltaRot - pi))
        {
            eulerFinal[i] -= pi * 2;
        }

        rDeltas[i] = eulerInitial[i] - eulerFinal[i];
        tDeltas[i] = transInitial[i] - transFinal[i];

        if (fabs(rDeltas[i]) > deltaRot)
        {
            failRot = true;
            break;
        }
        if (fabs(tDeltas[i]) > deltaTrans)
        {
            failTrans = true;
            break;
        }
    }

    return failRot || failTrans;
}

/// <summary>
/// Invert/Transpose the 3x3 Rotation Matrix Component of a 4x4 matrix
/// </summary>
/// <param name="rot">The rotation matrix to invert.</param>
void InvertRotation(Matrix4 &rot)
{
    // Invert equivalent to a transpose for 3x3 rotation rotrices when orthogonal
    float tmp = rot.M12;
    rot.M12 = rot.M21;
    rot.M21 = tmp;

    tmp = rot.M13;
    rot.M13 = rot.M31;
    rot.M31 = tmp;

    tmp = rot.M23;
    rot.M23 = rot.M32;
    rot.M32 = tmp;
}

/// <summary>
/// Negate the 3x3 Rotation Matrix Component of a 4x4 matrix
/// </summary>
/// <param name="rot">The rotation matrix to negate.</param>
void NegateRotation(Matrix4 &rot)
{
    rot.M11 = -rot.M11;
    rot.M12 = -rot.M12;
    rot.M13 = -rot.M13;

    rot.M21 = -rot.M21;
    rot.M22 = -rot.M22;
    rot.M23 = -rot.M23;

    rot.M31 = -rot.M31;
    rot.M32 = -rot.M32;
    rot.M33 = -rot.M33;
}

/// <summary>
/// Rotate a vector with the 3x3 Rotation Matrix Component of a 4x4 matrix
/// </summary>
/// <param name="vec">The Vector3 to rotate.</param>
/// <param name="rot">Rotation matrix.</param>
Vector3 RotateVector(const Vector3 &vec, const Matrix4 & rot)
{
    // we only use the rotation component here
    Vector3 result;

    result.x = (rot.M11 * vec.x) + (rot.M12 * vec.y) + (rot.M13 * vec.z);
    result.y = (rot.M21 * vec.x) + (rot.M22 * vec.y) + (rot.M23 * vec.z);
    result.z = (rot.M31 * vec.x) + (rot.M32 * vec.y) + (rot.M33 * vec.z);

    return result;
}
/// <summary>
/// Invert Matrix4 Pose either from WorldToCameraTransform (view) matrix to CameraToWorldTransform camera pose matrix (world/SE3) or vice versa
/// </summary>
/// <param name="transform">The camera pose transform matrix.</param>
/// <returns>Returns a Matrix4 containing the inverted camera pose.</returns>
Matrix4 InvertMatrix4Pose(const Matrix4 &transform)
{
    // Given the SE3 world transform transform T = [R|t], the inverse view transform matrix is simply:
    // T^-1 = [R^T | -R^T . t ]
    // This also works the opposite way to get the world transform, given the view transform matrix.
    Matrix4 rotation = Extract3x3Rotation(transform);

    Matrix4 invRotation = rotation;
    InvertRotation(invRotation);  // invert(transpose) 3x3 rotation

    Matrix4 negRotation = rotation;
    NegateRotation(negRotation);  // negate 3x3 rotation

    Vector3 translation = ExtractVector3Translation(transform);
    Vector3 invTranslation = RotateVector(translation, negRotation);

    // Add the translation back in
    invRotation.M41 = invTranslation.x;
    invRotation.M42 = invTranslation.y;
    invRotation.M43 = invTranslation.z;

    return invRotation;
}

/// <summary>
/// Write ASCII Wavefront .OBJ file
/// See http://en.wikipedia.org/wiki/Wavefront_.obj_file for .OBJ format
/// </summary>
/// <param name="mesh">The Kinect Fusion mesh object.</param>
/// <param name="lpOleFileName">The full path and filename of the file to save.</param>
/// <param name="flipYZ">Flag to determine whether the Y and Z values are flipped on save.</param>
/// <returns>indicates success or failure</returns>
HRESULT WriteAsciiObjMeshFile(INuiFusionMesh *mesh, LPOLESTR lpOleFileName, bool flipYZ)
{
    HRESULT hr = S_OK;

    if (NULL == mesh)
    {
        return E_INVALIDARG;
    }

    int numVertices = mesh->VertexCount();
    int numTriangleIndices = mesh->TriangleVertexIndexCount();
    int numTriangles = numVertices / 3;

    if (0 == numVertices || 0 == numTriangleIndices || 0 != numVertices % 3 || numVertices != numTriangleIndices)
    {
        return E_INVALIDARG;
    }

    const Vector3 *vertices = NULL;
    hr = mesh->GetVertices(&vertices);
    if (FAILED(hr))
    {
        return hr;
    }

    const Vector3 *normals = NULL;
    hr = mesh->GetNormals(&normals);
    if (FAILED(hr))
    {
        return hr;
    }

    const int *triangleIndices = NULL;
    hr = mesh->GetTriangleIndices(&triangleIndices);
    if (FAILED(hr))
    {
        return hr;
    }

    // Open File
    USES_CONVERSION;
    char* pszFileName = NULL;

    try
    {
        pszFileName = OLE2A(lpOleFileName);
    }
    catch (...)
    {
        return E_INVALIDARG;
    }

    FILE *meshFile = NULL;
    errno_t err = fopen_s(&meshFile, pszFileName, "wt");

    // Could not open file for writing - return
    if (0 != err || NULL == meshFile)
    {
        return E_ACCESSDENIED;
    }

    // Write the header line
    std::string header = "#\r\n# OBJ file created by Microsoft Kinect Fusion\r\n#\r\n";
    fwrite(header.c_str(), sizeof(char), header.length(), meshFile);

    // Sequentially write the 3 vertices of the triangle, for each triangle
    for (int t=0; t < numTriangles; ++t)
    {
        for (int v=0; v<3; v++)
        {
            Vector3 vertex = vertices[(t*3) + v];

            if (flipYZ)
            {
                vertex.y = -vertex.y;
                vertex.z = -vertex.z;
            }

            std::string vertexString = "v " + to_string(vertex.x) + " " + to_string(vertex.y) + " "+ to_string(vertex.z) + "\r\n" ;
            fwrite(vertexString.c_str(), sizeof(char), vertexString.length(), meshFile);
        }
    }

    // Sequentially write the 3 normals of the triangle, for each triangle
    for (int t=0; t < numTriangles; ++t)
    {
        for (int n=0; n<3; n++)
        {
            Vector3 normal = normals[(t*3) + n];

            if (flipYZ)
            {
                normal.y = -normal.y;
                normal.z = -normal.z;
            }

            std::string normalString = "vn " + to_string(normal.x) + " " + to_string(normal.y) + " "+ to_string(normal.z) + "\r\n" ;
            fwrite(normalString.c_str(), sizeof(char), normalString.length(), meshFile);
        }
    }

    // Sequentially write the 3 vertex indices of the triangle face, for each triangle
    // Note this is typically 1-indexed in an OBJ file when using absolute referencing!
    for (int t=0; t < numTriangles; ++t)
    {
        int baseIndex = (t*3)+1;    // Add 1 for the 1-based indexing

        std::string faceString = "f " + to_string(baseIndex) + "//" + to_string(baseIndex) + " " + to_string(baseIndex+1) + "//" + to_string(baseIndex+1) + " " + to_string(baseIndex+2) + "//" + to_string(baseIndex+2) + "\r\n" ;
        fwrite(faceString.c_str(), sizeof(char), faceString.length(), meshFile);
    }

    // Note: we do not have texcoords to store, if we did, we would put the index of the texcoords between the vertex and normal indices (i.e. between the two slashes //) in the string above
    fflush(meshFile);
    fclose(meshFile);

    return hr;
}

/// <summary>
/// Write Binary .STL file
/// see http://en.wikipedia.org/wiki/STL_(file_format) for STL format
/// </summary>
/// <param name="mesh">The Kinect Fusion mesh object.</param>
/// <param name="lpOleFileName">The full path and filename of the file to save.</param>
/// <param name="flipYZ">Flag to determine whether the Y and Z values are flipped on save.</param>
/// <returns>indicates success or failure</returns>
HRESULT WriteBinarySTLMeshFile(INuiFusionMesh *mesh, LPOLESTR lpOleFileName, bool flipYZ)
{
    HRESULT hr = S_OK;

    if (NULL == mesh)
    {
        return E_INVALIDARG;
    }

    int numVertices = mesh->VertexCount();
    int numTriangleIndices = mesh->TriangleVertexIndexCount();
    int numTriangles = numVertices / 3;

    if (0 == numVertices || 0 == numTriangleIndices || 0 != numVertices % 3 || numVertices != numTriangleIndices)
    {
        return E_INVALIDARG;
    }

    const Vector3 *vertices = NULL;
    hr = mesh->GetVertices(&vertices);
    if (FAILED(hr))
    {
        return hr;
    }

    const Vector3 *normals = NULL;
    hr = mesh->GetNormals(&normals);
    if (FAILED(hr))
    {
        return hr;
    }

    const int *triangleIndices = NULL;
    hr = mesh->GetTriangleIndices(&triangleIndices);
    if (FAILED(hr))
    {
        return hr;
    }

    // Open File
    USES_CONVERSION;
    char* pszFileName = NULL;

    try
    {
        pszFileName = OLE2A(lpOleFileName);
    }
    catch (...)
    {
        return E_INVALIDARG;
    }

    FILE *meshFile = NULL;
    errno_t err = fopen_s(&meshFile, pszFileName, "wb");

    // Could not open file for writing - return
    if (0 != err || NULL == meshFile)
    {
        return E_ACCESSDENIED;
    }

    // Write the header line
    const unsigned char header[80] = {0};   // initialize all values to 0
    fwrite(&header, sizeof(unsigned char), ARRAYSIZE(header), meshFile);

    // Write number of triangles
    fwrite(&numTriangles, sizeof(int), 1, meshFile);

    // Sequentially write the normal, 3 vertices of the triangle and attribute, for each triangle
    for (int t=0; t < numTriangles; ++t)
    {
        Vector3 normal = normals[t*3];

        if (flipYZ)
        {
            normal.y = -normal.y;
            normal.z = -normal.z;
        }

        // Write normal
        fwrite(&normal, sizeof(float), 3, meshFile);

        // Write vertices
        for (int v=0; v<3; v++)
        {
            Vector3 vertex = vertices[(t*3) + v];

            if (flipYZ)
            {
                vertex.y = -vertex.y;
                vertex.z = -vertex.z;
            }

            fwrite(&vertex, sizeof(float), 3, meshFile);
        }

        unsigned short attribute = 0;
        fwrite(&attribute, sizeof(unsigned short), 1, meshFile);
    }

    fflush(meshFile);
    fclose(meshFile);

    return hr;
}

