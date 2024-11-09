To ensure this code works with your solution, do the follow:

Step 1:
Add a copy of the "Microsoft.Kinect.Toolkit" project to your solution. Directory can be accessed using this path:
%KINECT_TOOLKIT_DIRR%\Samples\C#

Step 2:
Add a copy of the "Microsoft.Kinect.Toolkit.FaceTracking" project to your solution:
%KINECT_TOOLKIT_DIRR%\Samples\C#


Step 3:
Modify the FaceTrackFrame.cs file in the Microsoft.Kinect.Toolkit.FaceTracking project. 
Add the following function after the private void InternalDispose() { ... } function:
- FaceTrackFrame.cs

    // populates an array for the ShapePoints
    public PointF[] GetShapePoints()
    {
        // get the 2D tracked shapes
        IntPtr pointsPtr = IntPtr.Zero;
        uint pointCount = 0;
        this.faceTrackingResultPtr.Get2DShapePoints(out pointsPtr, out pointCount);
        if (pointCount == 0)
        {
            return null;
        }

        // create our array to hold the points
        PointF[] shapePoints = new PointF[pointCount];

        int sizeInBytes = Marshal.SizeOf(typeof(PointF));
        for (int i = 0; i < pointCount; i++)
        {
            IntPtr faceShapePointsPtr;
            if (IntPtr.Size == 8)
            {
                // 64bit
                faceShapePointsPtr = new IntPtr(pointsPtr.ToInt64() + (i * sizeInBytes));
            }
            else
            {
                // 32bit
                faceShapePointsPtr = new IntPtr(pointsPtr.ToInt32() + (i * sizeInBytes));
            }

            // copy the data
            shapePoints[i] = (PointF)Marshal.PtrToStructure(faceShapePointsPtr, typeof(PointF));
        }

        return shapePoints;
    }
