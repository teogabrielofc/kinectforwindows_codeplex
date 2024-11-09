//------------------------------------------------------------------------------
// <copyright file="TrackableUser.cs" company="Microsoft">
//
//      Copyright 2013 Microsoft Corporation 
//
//      Licensed under the Apache License, Version 2.0 (the "License"); 
//      you may not use this file except in compliance with the License.
//      You may obtain a copy of the License at
//
//          http://www.apache.org/licenses/LICENSE-2.0 
//
//      Unless required by applicable law or agreed to in writing, software 
//      distributed under the License is distributed on an "AS IS" BASIS,
//      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
//      See the License for the specific language governing permissions and 
//      limitations under the License. 
//
// </copyright>
//------------------------------------------------------------------------------

namespace Microsoft.Samples.Kinect.BackgroundRemovalMultiUser
{
    using System;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using Microsoft.Kinect;
    using Microsoft.Kinect.Toolkit.BackgroundRemoval;

    /// <summary>
    /// Represents one trackable user for which background-removal will be performed.
    /// </summary>
    internal class TrackableUser : IDisposable
    {
        /// <summary>
        /// Invalid skeleton tracking ID.
        /// </summary>
        public const int InvalidTrackingId = 0;

        /// <summary>
        /// Backing store for the TrackingId property.
        /// </summary>
        private int trackingId = InvalidTrackingId;

        /// <summary>
        /// The Kinect sensor currently in use.
        /// </summary>
        private KinectSensor sensor;

        /// <summary>
        /// The BackgroundRemovedColorStream associated with this user.
        /// </summary>
        private BackgroundRemovedColorStream backgroundRemovedColorStream;

        /// <summary>
        /// Array into which new skeletons from the sensor will be copied.
        /// </summary>
        private Skeleton[] skeletonsNew;

        /// <summary>
        /// Array of skeletons in which this user was fully tracked.
        /// </summary>
        private Skeleton[] skeletonsTracked;

        /// <summary>
        /// The Image control that has the ForegroundBitmap as its source.
        /// </summary>
        private Image imageControl;

        /// <summary>
        /// Track whether Dispose has been called.
        /// </summary>
        private bool disposed;

        /// <summary>
        /// Initializes a new instance of the <see cref="TrackableUser"/> class.
        /// </summary>
        /// <param name="imageControl">The image control in which this user's background-removed
        /// image will be displayed.</param>
        public TrackableUser(Image imageControl)
        {
            this.imageControl = imageControl;
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="TrackableUser"/> class.
        /// This will run only if the Dispose method does not get called.
        /// </summary>
        ~TrackableUser()
        {
            this.Dispose(false);
        }

        /// <summary>
        /// Gets a value indicating whether a user is currently being tracked by this object.
        /// </summary>
        public bool IsTracked
        {
            get
            {
                return InvalidTrackingId != this.TrackingId;
            }
        }

        /// <summary>
        /// Gets or sets the tracking ID corresponding to this object. The value may be invalid
        /// (i.e., zero), indicating that no user is currently being tracked by this object.
        /// </summary>
        public int TrackingId
        {
            get
            {
                return this.trackingId;
            }

            set
            {
                if (value != this.trackingId)
                {
                    if (null != this.backgroundRemovedColorStream)
                    {
                        if (InvalidTrackingId != value)
                        {
                            this.backgroundRemovedColorStream.SetTrackedPlayer(value);
                            this.Timestamp = DateTime.UtcNow;
                        }
                        else
                        {
                            // Hide the last frame that was received for this user.
                            this.imageControl.Visibility = Visibility.Hidden;
                            this.Timestamp = DateTime.MinValue;
                        }
                    }

                    this.trackingId = value;
                }
            }
        }

        /// <summary>
        /// Gets the time that the TrackingId was set to a valid value. If the TrackingId is
        /// not valid (i.e., zero), the Timestamp will be DateTime.MinValue.
        /// </summary>
        public DateTime Timestamp { get; private set; }

        /// <summary>
        /// Frees all resources associated with the trackable user.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);

            // This object will be cleaned up by the Dispose method.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Notification of changes to available sensors.
        /// </summary>
        /// <param name="oldSensor">Previous sensor.</param>
        /// <param name="newSensor">New sensor.</param>
        public void OnKinectSensorChanged(KinectSensor oldSensor, KinectSensor newSensor)
        {
            if (null != oldSensor)
            {
                // Remove sensor frame event handler.
                oldSensor.AllFramesReady -= this.SensorAllFramesReady;

                // Tear down the BackgroundRemovedColorStream for this user.
                this.backgroundRemovedColorStream.BackgroundRemovedFrameReady -=
                    this.BackgroundRemovedFrameReadyHandler;
                this.backgroundRemovedColorStream.Dispose();
                this.backgroundRemovedColorStream = null;
                this.TrackingId = InvalidTrackingId;
            }

            this.sensor = newSensor;

            if (null != newSensor)
            {
                // Setup a new BackgroundRemovedColorStream for this user.
                this.backgroundRemovedColorStream = new BackgroundRemovedColorStream(newSensor);
                this.backgroundRemovedColorStream.BackgroundRemovedFrameReady +=
                    this.BackgroundRemovedFrameReadyHandler;
                this.backgroundRemovedColorStream.Enable(
                    newSensor.ColorStream.Format,
                    newSensor.DepthStream.Format);

                // Add an event handler to be called when there is new frame data from the sensor.
                newSensor.AllFramesReady += this.SensorAllFramesReady;
            }
        }

        /// <summary>
        /// Frees all resources associated with the trackable user.
        /// </summary>
        /// <param name="disposing">Whether the function was called from Dispose.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                if (null != this.backgroundRemovedColorStream)
                {
                    this.backgroundRemovedColorStream.Dispose();
                }

                this.disposed = true;
            }
        }

        /// <summary>
        /// Copies the data from a skeleton frame into a skeleton array.
        /// </summary>
        /// <param name="skeletonFrame">The skeleton frame containing the data to be copied.</param>
        private void CopyDataFromSkeletonFrame(SkeletonFrame skeletonFrame)
        {
            // Allocate space for the skeleton data we'll receive.
            if (null == this.skeletonsNew)
            {
                this.skeletonsNew = new Skeleton[skeletonFrame.SkeletonArrayLength];
            }

            // Copy the skeleton data.
            skeletonFrame.CopySkeletonDataTo(this.skeletonsNew);
        }

        /// <summary>
        /// Updates the array containing the most recent skeleton data in which this user was
        /// fully tracked.
        /// </summary>
        /// <returns>True if the user is still present, false otherwise.</returns>
        private bool UpdateTrackedSkeletonsArray()
        {
            // Determine if this user is still present in the scene.
            bool isUserPresent = false;
            foreach (var skeleton in this.skeletonsNew)
            {
                if (skeleton.TrackingId == this.TrackingId)
                {
                    isUserPresent = true;
                    if (skeleton.TrackingState == SkeletonTrackingState.Tracked)
                    {
                        // User is fully tracked: save the new array of skeletons,
                        // and recycle the old saved array for reuse next time.
                        var temp = this.skeletonsTracked;
                        this.skeletonsTracked = this.skeletonsNew;
                        this.skeletonsNew = temp;
                    }

                    break;
                }
            }

            if (!isUserPresent)
            {
                // User has disappeared; stop trying to track.
                this.TrackingId = TrackableUser.InvalidTrackingId;
            }

            return isUserPresent;
        }

        /// <summary>
        /// Event handler for Kinect sensor's AllFramesReady event.
        /// </summary>
        /// <param name="sender">Object sending the event.</param>
        /// <param name="e">Event arguments.</param>
        private void SensorAllFramesReady(object sender, AllFramesReadyEventArgs e)
        {
            // Already shutting down, or lingering events from previous sensor: do nothing here.
            if (null == this.sensor || this.sensor != sender)
            {
                return;
            }

            try
            {
                if (this.IsTracked)
                {
                    using (var depthFrame = e.OpenDepthImageFrame())
                    {
                        if (null != depthFrame)
                        {
                            // Process depth data for background removal.
                            this.backgroundRemovedColorStream.ProcessDepth(
                                depthFrame.GetRawPixelData(),
                                depthFrame.Timestamp);
                        }
                    }

                    using (var colorFrame = e.OpenColorImageFrame())
                    {
                        if (null != colorFrame)
                        {
                            // Process color data for background removal.
                            this.backgroundRemovedColorStream.ProcessColor(
                                colorFrame.GetRawPixelData(),
                                colorFrame.Timestamp);
                        }
                    }

                    using (var skeletonFrame = e.OpenSkeletonFrame())
                    {
                        if (null != skeletonFrame)
                        {
                            // Save skeleton frame data for subsequent processing.
                            this.CopyDataFromSkeletonFrame(skeletonFrame);

                            // Locate the most recent data in which this user was fully tracked.
                            bool isUserPresent = this.UpdateTrackedSkeletonsArray();

                            // If we have an array in which this user is fully tracked,
                            // process the skeleton data for background removal.
                            if (isUserPresent && null != this.skeletonsTracked)
                            {
                                this.backgroundRemovedColorStream.ProcessSkeleton(
                                    this.skeletonsTracked,
                                    skeletonFrame.Timestamp);
                            }
                        }
                    }
                }
            }
            catch (InvalidOperationException)
            {
                // Ignore the exception. 
            }
        }

        /// <summary>
        /// Handle a new background-removed color frame. The frame obtained from the stream
        /// is in BGRA format.
        /// </summary>
        /// <param name="sender">Object sending the event.</param>
        /// <param name="e">Event arguments.</param>
        private void BackgroundRemovedFrameReadyHandler(
            object sender,
            BackgroundRemovedColorFrameReadyEventArgs e)
        {
            using (var backgroundRemovedFrame = e.OpenBackgroundRemovedColorFrame())
            {
                if (null != backgroundRemovedFrame && this.IsTracked)
                {
                    int width = backgroundRemovedFrame.Width;
                    int height = backgroundRemovedFrame.Height;

                    WriteableBitmap foregroundBitmap = this.imageControl.Source as WriteableBitmap;

                    // If necessary, allocate new bitmap. Set it as the source of the Image control.
                    if (null == foregroundBitmap ||
                        foregroundBitmap.PixelWidth != width ||
                        foregroundBitmap.PixelHeight != height)
                    {
                        foregroundBitmap = new WriteableBitmap(
                            width,
                            height,
                            96.0,
                            96.0,
                            PixelFormats.Bgra32,
                            null);

                        this.imageControl.Source = foregroundBitmap;
                    }

                    // Write the pixel data into our bitmap.
                    foregroundBitmap.WritePixels(
                        new Int32Rect(0, 0, width, height),
                        backgroundRemovedFrame.GetRawPixelData(),
                        width * sizeof(uint),
                        0);

                    // A frame has been delivered; ensure that it is visible.
                    this.imageControl.Visibility = Visibility.Visible;
                }
            }
        }
    }
}
