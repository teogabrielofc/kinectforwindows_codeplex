// --------------------------------------------------------------------------------------------------------------------
// <copyright file="FaceTrackingViewer.xaml.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace FaceTrackingBasics
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Media;
    using Microsoft.Kinect;
    using Microsoft.Kinect.Toolkit.FaceTracking;
    
    using Point = System.Windows.Point;

    /// <summary>
    /// Class that uses the Face Tracking SDK to display a face mask for
    /// tracked skeletons
    /// </summary>
    public partial class FaceTrackingViewer : UserControl, IDisposable
    {
        public static readonly DependencyProperty KinectProperty = DependencyProperty.Register(
            "Kinect",
            typeof(KinectSensor),
            typeof(FaceTrackingViewer),
            new PropertyMetadata(
                null, (o, args) => ((FaceTrackingViewer)o).OnSensorChanged((KinectSensor)args.OldValue, (KinectSensor)args.NewValue)));

        private const uint MaxMissedFrames = 100;

        private readonly Dictionary<int, SkeletonFaceTracker> trackedSkeletons = new Dictionary<int, SkeletonFaceTracker>();

        private byte[] colorImage;

        private ColorImageFormat colorImageFormat = ColorImageFormat.Undefined;

        private short[] depthImage;

        private DepthImageFormat depthImageFormat = DepthImageFormat.Undefined;

        private bool disposed;

        private Skeleton[] skeletonData;

        private bool draw3DMesh;

        private bool drawShapePoints;

        private DrawFeaturePoint drawFeaturePoints;

        private Grid grid;

        public FaceTrackingViewer()
        {
            this.InitializeComponent();

            // add grid to the layout
            this.grid = new Grid();
            this.grid.Background = Brushes.Transparent;
            this.Content = this.grid;
        }

        ~FaceTrackingViewer()
        {
            this.Dispose(false);
        }

        public KinectSensor Kinect
        {
            get
            {
                return (KinectSensor)this.GetValue(KinectProperty);
            }

            set
            {
                this.SetValue(KinectProperty, value);
            }
        }

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                this.ResetFaceTracking();

                this.disposed = true;
            }
        }

        protected override void OnRender(DrawingContext drawingContext)
        {
            base.OnRender(drawingContext);
            foreach (SkeletonFaceTracker faceInformation in this.trackedSkeletons.Values)
            {
                faceInformation.DrawFaceModel(drawingContext);
            }
        }

        private void OnAllFramesReady(object sender, AllFramesReadyEventArgs allFramesReadyEventArgs)
        {
            ColorImageFrame colorImageFrame = null;
            DepthImageFrame depthImageFrame = null;
            SkeletonFrame skeletonFrame = null;

            try
            {
                colorImageFrame = allFramesReadyEventArgs.OpenColorImageFrame();
                depthImageFrame = allFramesReadyEventArgs.OpenDepthImageFrame();
                skeletonFrame = allFramesReadyEventArgs.OpenSkeletonFrame();

                if (colorImageFrame == null || depthImageFrame == null || skeletonFrame == null)
                {
                    return;
                }

                // Check for image format changes.  The FaceTracker doesn't
                // deal with that so we need to reset.
                if (this.depthImageFormat != depthImageFrame.Format)
                {
                    this.ResetFaceTracking();
                    this.depthImage = null;
                    this.depthImageFormat = depthImageFrame.Format;
                }

                if (this.colorImageFormat != colorImageFrame.Format)
                {
                    this.ResetFaceTracking();
                    this.colorImage = null;
                    this.colorImageFormat = colorImageFrame.Format;
                }

                // Create any buffers to store copies of the data we work with
                if (this.depthImage == null)
                {
                    this.depthImage = new short[depthImageFrame.PixelDataLength];
                }

                if (this.colorImage == null)
                {
                    this.colorImage = new byte[colorImageFrame.PixelDataLength];
                }

                // Get the skeleton information
                if (this.skeletonData == null || this.skeletonData.Length != skeletonFrame.SkeletonArrayLength)
                {
                    this.skeletonData = new Skeleton[skeletonFrame.SkeletonArrayLength];
                }

                colorImageFrame.CopyPixelDataTo(this.colorImage);
                depthImageFrame.CopyPixelDataTo(this.depthImage);
                skeletonFrame.CopySkeletonDataTo(this.skeletonData);

                // Update the list of trackers and the trackers with the current frame information
                foreach (Skeleton skeleton in this.skeletonData)
                {
                    if (skeleton.TrackingState == SkeletonTrackingState.Tracked
                        || skeleton.TrackingState == SkeletonTrackingState.PositionOnly)
                    {
                        // We want keep a record of any skeleton, tracked or untracked.
                        if (!this.trackedSkeletons.ContainsKey(skeleton.TrackingId))
                        {
                            // create a new canvas for each tracker
                            Canvas canvas = new Canvas();
                            canvas.Background = Brushes.Transparent;
                            this.grid.Children.Add(canvas);

                            this.trackedSkeletons.Add(skeleton.TrackingId, new SkeletonFaceTracker(canvas));
                        }

                        // Give each tracker the upated frame.
                        SkeletonFaceTracker skeletonFaceTracker;
                        if (this.trackedSkeletons.TryGetValue(skeleton.TrackingId, out skeletonFaceTracker))
                        {
                            skeletonFaceTracker.OnFrameReady(this.Kinect, colorImageFormat, colorImage, depthImageFormat, depthImage, skeleton);
                            skeletonFaceTracker.LastTrackedFrame = skeletonFrame.FrameNumber;

                            skeletonFaceTracker.DrawFaceMesh = this.draw3DMesh;
                            skeletonFaceTracker.DrawShapePoints = this.drawShapePoints;
                            skeletonFaceTracker.DrawFeaturePoints = this.drawFeaturePoints;

                        }
                    }
                }

                this.RemoveOldTrackers(skeletonFrame.FrameNumber);

                this.InvalidateVisual();
            }
            finally
            {
                if (colorImageFrame != null)
                {
                    colorImageFrame.Dispose();
                }

                if (depthImageFrame != null)
                {
                    depthImageFrame.Dispose();
                }

                if (skeletonFrame != null)
                {
                    skeletonFrame.Dispose();
                }
            }
        }

        private void OnSensorChanged(KinectSensor oldSensor, KinectSensor newSensor)
        {
            if (oldSensor != null)
            {
                oldSensor.AllFramesReady -= this.OnAllFramesReady;
                this.ResetFaceTracking();
            }

            if (newSensor != null)
            {
                newSensor.AllFramesReady += this.OnAllFramesReady;
            }
        }

        /// <summary>
        /// Clear out any trackers for skeletons we haven't heard from for a while
        /// </summary>
        private void RemoveOldTrackers(int currentFrameNumber)
        {
            var trackersToRemove = new List<int>();

            foreach (var tracker in this.trackedSkeletons)
            {
                uint missedFrames = (uint)currentFrameNumber - (uint)tracker.Value.LastTrackedFrame;
                if (missedFrames > MaxMissedFrames)
                {
                    // There have been too many frames since we last saw this skeleton
                    trackersToRemove.Add(tracker.Key);
                }
            }

            foreach (int trackingId in trackersToRemove)
            {
                this.RemoveTracker(trackingId);
            }
        }

        private void RemoveTracker(int trackingId)
        {
            this.trackedSkeletons[trackingId].Dispose();
            this.trackedSkeletons.Remove(trackingId);
        }

        private void ResetFaceTracking()
        {
            foreach (int trackingId in new List<int>(this.trackedSkeletons.Keys))
            {
                this.RemoveTracker(trackingId);
            }
        }

        public void DrawMesh(bool drawMesh)
        {
            this.draw3DMesh = drawMesh;

            foreach (SkeletonFaceTracker faceInformation in this.trackedSkeletons.Values)
            {
                faceInformation.DrawFaceMesh = drawMesh;
            }

        }

        public void DrawShapePoints(bool drawShapePoints)
        {
            this.drawShapePoints = drawShapePoints;

            foreach (SkeletonFaceTracker faceInformation in this.trackedSkeletons.Values)
            {
                faceInformation.DrawShapePoints = drawShapePoints;
            }
        }

        public void DrawFeaturePoints(DrawFeaturePoint drawFeature)
        {
            this.drawFeaturePoints = drawFeature;

            foreach (SkeletonFaceTracker faceInformation in this.trackedSkeletons.Values)
            {
                faceInformation.DrawFeaturePoints = drawFeature;
            }
        }

        private class SkeletonFaceTracker : IDisposable
        {
            private static FaceTriangle[] faceTriangles;

            private EnumIndexableCollection<FeaturePoint, PointF> facePoints;

            private FaceTracker faceTracker;

            private bool lastFaceTrackSucceeded;

            private SkeletonTrackingState skeletonTrackingState;

            public int LastTrackedFrame { get; set; }

            // properties to toggle rendering 3D mesh, shape points and feature points
            public bool DrawFaceMesh { get; set; }

            public bool DrawShapePoints { get; set; }

            public DrawFeaturePoint DrawFeaturePoints { get; set; }

            // defined array for the feature points
            private Array featurePoints;
            private DrawFeaturePoint lastDrawFeaturePoints;

            // array for Points to be used in shape points rendering
            private PointF[] shapePoints;

            // map to hold the label controls for the overlay
            private Dictionary<string, Label> labelControls;

            // canvas control for new text rendering
            private Canvas Canvas;

            // canvas is passed in for every instance
            public SkeletonFaceTracker(Canvas canvas)
            {
                this.Canvas = canvas;
            }

            public void Dispose()
            {
                if (this.faceTracker != null)
                {
                    this.faceTracker.Dispose();
                    this.faceTracker = null;
                }
            }

            public void DrawFaceModel(DrawingContext drawingContext)
            {
                if (!this.lastFaceTrackSucceeded || this.skeletonTrackingState != SkeletonTrackingState.Tracked)
                {
                    return;
                }

                // only draw if selected
                if (this.DrawFaceMesh && this.facePoints != null)
                {
                    var faceModelPts = new List<Point>();
                    var faceModel = new List<FaceModelTriangle>();

                    for (int i = 0; i < this.facePoints.Count; i++)
                    {
                        faceModelPts.Add(new Point(this.facePoints[i].X + 0.5f, this.facePoints[i].Y + 0.5f));
                    }

                    foreach (var t in faceTriangles)
                    {
                        var triangle = new FaceModelTriangle();
                        triangle.P1 = faceModelPts[t.First];
                        triangle.P2 = faceModelPts[t.Second];
                        triangle.P3 = faceModelPts[t.Third];
                        faceModel.Add(triangle);
                    }

                    var faceModelGroup = new GeometryGroup();
                    for (int i = 0; i < faceModel.Count; i++)
                    {
                        var faceTriangle = new GeometryGroup();
                        faceTriangle.Children.Add(new LineGeometry(faceModel[i].P1, faceModel[i].P2));
                        faceTriangle.Children.Add(new LineGeometry(faceModel[i].P2, faceModel[i].P3));
                        faceTriangle.Children.Add(new LineGeometry(faceModel[i].P3, faceModel[i].P1));
                        faceModelGroup.Children.Add(faceTriangle);
                    }
                    drawingContext.DrawGeometry(Brushes.LightYellow, new Pen(Brushes.LightYellow, 1.0), faceModelGroup);
                }
            }

            /// <summary>
            /// Updates the face tracking information for this skeleton
            /// </summary>
            internal void OnFrameReady(KinectSensor kinectSensor, ColorImageFormat colorImageFormat, byte[] colorImage, DepthImageFormat depthImageFormat, short[] depthImage, Skeleton skeletonOfInterest)
            {
                this.skeletonTrackingState = skeletonOfInterest.TrackingState;

                if (this.skeletonTrackingState != SkeletonTrackingState.Tracked)
                {
                    // nothing to do with an untracked skeleton.
                    return;
                }

                if (this.faceTracker == null)
                {
                    try
                    {
                        this.faceTracker = new FaceTracker(kinectSensor);
                    }
                    catch (InvalidOperationException)
                    {
                        // During some shutdown scenarios the FaceTracker
                        // is unable to be instantiated.  Catch that exception
                        // and don't track a face.
                        Debug.WriteLine("AllFramesReady - creating a new FaceTracker threw an InvalidOperationException");
                        this.faceTracker = null;
                    }
                }

                if (this.faceTracker != null)
                {
                    FaceTrackFrame frame = this.faceTracker.Track(
                        colorImageFormat, colorImage, depthImageFormat, depthImage, skeletonOfInterest);

                    this.lastFaceTrackSucceeded = frame.TrackSuccessful;
                    if (this.lastFaceTrackSucceeded)
                    {
                        if (faceTriangles == null)
                        {
                            // only need to get this once.  It doesn't change.
                            faceTriangles = frame.GetTriangles();
                        }

                        if (this.DrawFaceMesh || this.DrawFeaturePoints != DrawFeaturePoint.None)
                        {
                            this.facePoints = frame.GetProjected3DShape();
                        }

                        // get the shape points array
                        if (this.DrawShapePoints)
                        {
                            // see the !!!README.txt file to add the function 
                            // to your toolkit project
                            this.shapePoints = frame.GetShapePoints();
                        }
                    }

                    // draw/remove the components
                    SetFeaturePointsLocations();
                    SetShapePointsLocations();
                }
            }

            private struct FaceModelTriangle
            {
                public Point P1;
                public Point P2;
                public Point P3;
            }

            private Label FindTextControl(string key)
            {
                if (this.labelControls == null)
                {
                    this.labelControls = new Dictionary<string, Label>();
                }

                Label text = null;
                if (this.labelControls.ContainsKey(key))
                {
                    text = this.labelControls[key];
                }

                return text;
            }

            private void UpdateTextControls(string key, string prefix, Brush color, int x, int y)
            {
                Label label = FindTextControl(prefix + "_" + key);

                if (label == null)
                {
                    label = new Label()
                    {
                        Name = prefix + "_" + key,
                        Content = key,
                        FontFamily = new FontFamily("Arial Bold"),
                        FontSize = 5,
                        Background = Brushes.Transparent,
                        Foreground = color,
                    };

                    this.labelControls.Add(label.Name, label);
                }

                if (label != null)
                {
                    // be sure it was added to the canvas
                    if (!this.Canvas.Children.Contains(label))
                    {
                        this.Canvas.Children.Add(label);
                    }

                    // move it to the correct location
                    label.RenderTransform = new TranslateTransform(x - (label.ActualWidth / 2), y - (label.ActualHeight / 2));
                }
            }

            private void RemoveAllFromCanvas(string key)
            {
                for (int i = this.Canvas.Children.Count - 1; i >= 0; i--)
                {
                    if (this.Canvas.Children[i].GetType() == typeof(Label))
                    {
                        Label txtblock = (Label)this.Canvas.Children[i];
                        if (txtblock.Name.StartsWith(key))
                        {
                            this.Canvas.Children.Remove(txtblock);
                        }
                    }
                }
            }

            private void SetShapePointsLocations()
            {
                if (!this.lastFaceTrackSucceeded || !this.DrawShapePoints)
                {
                    if (this.Canvas.Children.Count > 0)
                    {
                        RemoveAllFromCanvas("Shape_");
                    }
                    return;
                }

                int count = 0;

                if (this.DrawShapePoints && (this.shapePoints != null))
                {
                    // create or update the text controls on the canvas/table
                    foreach (PointF v2d in this.shapePoints)
                    {
                        UpdateTextControls(
                            count.ToString(),
                            "Shape", Brushes.Yellow,
                            (int)v2d.X, (int)v2d.Y);

                        count++;
                    }
                }

            }

            private void SetFeaturePointsLocations()
            {
                // populate the array of feature point names
                if (this.featurePoints == null)
                {
                    this.featurePoints = Enum.GetValues(typeof(FeaturePoint));
                }

                if (!this.lastFaceTrackSucceeded || this.lastDrawFeaturePoints != this.DrawFeaturePoints)
                {
                    if (this.Canvas.Children.Count > 0)
                    {
                        RemoveAllFromCanvas("FacePoint_");
                    }

                    this.lastDrawFeaturePoints = this.DrawFeaturePoints;

                    return;
                }

                if ((this.DrawFeaturePoints != DrawFeaturePoint.None) && (this.featurePoints != null))
                {
                    foreach (FeaturePoint fp in this.featurePoints)
                    {
                        UpdateTextControls(
                            (this.DrawFeaturePoints == DrawFeaturePoint.ByValue) ? ((int)fp).ToString() : fp.ToString(),
                            "FacePoint", Brushes.LimeGreen,
                            (int)this.facePoints[fp].X, (int)this.facePoints[fp].Y);
                    }
                }
            }
        }
    }
}