//------------------------------------------------------------------------------
// <copyright file="MainWindow.xaml.cs" company="Microsoft">
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
    using System.Globalization;
    using System.IO;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using Microsoft.Kinect;
    using Microsoft.Kinect.Toolkit;

    /// <summary>
    /// Interaction logic for MainWindow.xaml.
    /// </summary>
    public partial class MainWindow : Window, IDisposable
    {
        /// <summary>
        /// Format we will use for the depth stream.
        /// </summary>
        private const DepthImageFormat DepthFormat = DepthImageFormat.Resolution320x240Fps30;

        /// <summary>
        /// Format we will use for the color stream.
        /// </summary>
        private const ColorImageFormat ColorFormat = ColorImageFormat.RgbResolution640x480Fps30;

        /// <summary>
        /// Maximum number of users that can be tracked simultaneously.
        /// </summary>
        private const int MaxUsers = 6;

        /// <summary>
        /// Skeleton IDs chosen for full tracking.
        /// </summary>
        private readonly int[] trackingIds =
        {
            TrackableUser.InvalidTrackingId,
            TrackableUser.InvalidTrackingId
        };

        /// <summary>
        /// Colors to use for each user index.
        /// </summary>
        private readonly uint[] userColors =
        {
            0xff000000, // black (background)
            0xffff0000, // red
            0xffff00ff, // magenta
            0xff0000ff, // blue
            0xff00ffff, // cyan
            0xff00ff00, // green
            0xffffff00, // yellow
            0xff000000  // black (unused)
        };

        /// <summary>
        /// Objects representing users being tracked for background removal.
        /// </summary>
        private TrackableUser[] trackableUsers = new TrackableUser[MaxUsers];

        /// <summary>
        /// Active Kinect sensor.
        /// </summary>
        private KinectSensorChooser sensorChooser;

        /// <summary>
        /// Buffer for most recently received depth frame.
        /// </summary>
        private DepthImagePixel[] depthData;

        /// <summary>
        /// Skeletons most recently received from the sensor.
        /// </summary>
        private Skeleton[] skeletons;

        /// <summary>
        /// Bitmap for the user view inset.
        /// </summary>
        private WriteableBitmap userViewBitmap;

        /// <summary>
        /// Index of the next user to choose for skeleton tracking.
        /// </summary>
        private int nextUserIndex = 0;

        /// <summary>
        /// Track whether Dispose has been called.
        /// </summary>
        private bool disposed;

        /// <summary>
        /// Initializes a new instance of the <see cref="MainWindow"/> class.
        /// </summary>
        public MainWindow()
        {
            this.InitializeComponent();

            // Initialize the sensor chooser and UI.
            this.sensorChooser = new KinectSensorChooser();
            this.sensorChooserUi.KinectSensorChooser = this.sensorChooser;
            this.sensorChooser.KinectChanged += this.SensorChooserOnKinectChanged;
            this.sensorChooser.Start();

            // Create one Image control per trackable user.
            for (int i = 0; i < MaxUsers; ++i)
            {
                Image image = new Image();
                this.MaskedColorImages.Children.Add(image);
                this.trackableUsers[i] = new TrackableUser(image);
            }
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="MainWindow"/> class.
        /// This will run only if the Dispose method does not get called.
        /// </summary>
        ~MainWindow()
        {
            this.Dispose(false);
        }

        /// <summary>
        /// Disposes all objects associated with the MainWindow.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);

            // This object will be cleaned up by the Dispose method.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Disposes all objects associated with the MainWindow.
        /// </summary>
        /// <param name="disposing">Whether the function was called from Dispose.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                foreach (var user in this.trackableUsers)
                {
                    user.Dispose();
                }

                this.disposed = true;
            }
        }

        /// <summary>
        /// Execute shutdown tasks.
        /// </summary>
        /// <param name="sender">Object sending the event.</param>
        /// <param name="e">Event arguments.</param>
        private void WindowClosing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            this.sensorChooser.Stop();
            this.sensorChooser = null;
        }

        /// <summary>
        /// Event handler for Kinect sensor's AllFramesReady event.
        /// </summary>
        /// <param name="sender">Object sending the event.</param>
        /// <param name="e">Event arguments.</param>
        private void SensorAllFramesReady(object sender, AllFramesReadyEventArgs e)
        {
            // Already shutting down, or lingering events from previous sensor: do nothing here.
            if (null == this.sensorChooser ||
                null == this.sensorChooser.Kinect ||
                this.sensorChooser.Kinect != sender)
            {
                return;
            }

            try
            {
                using (var depthFrame = e.OpenDepthImageFrame())
                {
                    if (null != depthFrame)
                    {
                        // Update the user view with the new depth data.
                        this.UpdateUserView(depthFrame);
                    }
                }

                using (var skeletonFrame = e.OpenSkeletonFrame())
                {
                    if (null != skeletonFrame)
                    {
                        // Save skeleton data for subsequent lookup of tracking IDs.
                        skeletonFrame.CopySkeletonDataTo(this.skeletons);

                        this.UpdateChosenSkeletons();
                    }
                }
            }
            catch (InvalidOperationException)
            {
                // Ignore the exception. 
            }
        }

        /// <summary>
        /// Updates the user view inset window, which displays user masks as reported in the depth
        /// image.
        /// </summary>
        /// <param name="depthFrame">New depth frame from the sensor.</param>
        private void UpdateUserView(DepthImageFrame depthFrame)
        {
            if (null == this.depthData || this.depthData.Length != depthFrame.PixelDataLength)
            {
                // If necessary, allocate new buffer for the depth data.
                this.depthData = new DepthImagePixel[depthFrame.PixelDataLength];
            }

            // Store the depth data.
            depthFrame.CopyDepthImagePixelDataTo(this.depthData);

            int width = depthFrame.Width;
            int height = depthFrame.Height;

            if (null == this.userViewBitmap ||
                this.userViewBitmap.PixelWidth != width ||
                this.userViewBitmap.PixelHeight != height)
            {
                // If necessary, allocate new bitmap in BGRA format.
                // Set it as the source of the UserView Image control.
                this.userViewBitmap = new WriteableBitmap(
                    width,
                    height,
                    96.0,
                    96.0,
                    PixelFormats.Bgra32,
                    null);

                this.UserView.Source = this.userViewBitmap;
            }

            // Write the per-user colors into the user view bitmap, one pixel at a time.
            this.userViewBitmap.Lock();
            
            unsafe
            {
                uint* userViewBits = (uint*)this.userViewBitmap.BackBuffer;
                fixed (uint* userColors = &this.userColors[0])
                {
                    // Walk through each pixel in the depth data.
                    fixed (DepthImagePixel* depthData = &this.depthData[0])
                    {
                        DepthImagePixel* depthPixel = depthData;
                        DepthImagePixel* depthPixelEnd = depthPixel + this.depthData.Length;
                        while (depthPixel < depthPixelEnd)
                        {
                            // Lookup a pixel color based on the player index.
                            // Store the color in the user view bitmap's buffer.
                            *(userViewBits++) = *(userColors + (depthPixel++)->PlayerIndex);
                        }
                    }
                }
            }

            this.userViewBitmap.AddDirtyRect(new Int32Rect(0, 0, width, height));
            this.userViewBitmap.Unlock();
        }

        /// <summary>
        /// Called when the KinectSensorChooser gets a new sensor.
        /// </summary>
        /// <param name="sender">Sender of the event.</param>
        /// <param name="args">Event arguments.</param>
        private void SensorChooserOnKinectChanged(object sender, KinectChangedEventArgs args)
        {
            if (null != args.OldSensor)
            {
                try
                {
                    // Shut down the old sensor.
                    args.OldSensor.AllFramesReady -= this.SensorAllFramesReady;
                    args.OldSensor.DepthStream.Disable();
                    args.OldSensor.ColorStream.Disable();
                    args.OldSensor.SkeletonStream.Disable();
                }
                catch (InvalidOperationException)
                {
                    // KinectSensor might enter an invalid state while enabling/disabling streams
                    // or stream features. E.g.: sensor might be abruptly unplugged.
                }
            }

            if (null != args.NewSensor)
            {
                try
                {
                    // Initialize the new sensor.
                    args.NewSensor.DepthStream.Enable(DepthFormat);
                    args.NewSensor.ColorStream.Enable(ColorFormat);
                    args.NewSensor.SkeletonStream.Enable();
                    args.NewSensor.SkeletonStream.AppChoosesSkeletons = true;
                    args.NewSensor.SkeletonStream.EnableTrackingInNearRange = true;

                    // Allocate space for the skeleton data we'll receive
                    if (null == this.skeletons)
                    {
                        this.skeletons =
                            new Skeleton[args.NewSensor.SkeletonStream.FrameSkeletonArrayLength];
                    }

                    // Add an event handler to be called whenever there is new depth frame data
                    args.NewSensor.AllFramesReady += this.SensorAllFramesReady;

                    try
                    {
                        args.NewSensor.DepthStream.Range =
                            this.checkBoxNearMode.IsChecked.GetValueOrDefault()
                            ? DepthRange.Near
                            : DepthRange.Default;
                    }
                    catch (InvalidOperationException)
                    {
                        // If near mode not supported, reset back to default mode.
                        args.NewSensor.DepthStream.Range = DepthRange.Default;
                    }

                    this.statusBarText.Text = Properties.Resources.Instructions;
                }
                catch (InvalidOperationException)
                {
                    // KinectSensor might enter an invalid state while enabling/disabling streams
                    // or stream features. E.g.: sensor might be abruptly unplugged.
                }
            }

            // Notify each TrackableUser object that the sensor has changed.
            foreach (var user in this.trackableUsers)
            {
                user.OnKinectSensorChanged(args.OldSensor, args.NewSensor);
            }
        }

        /// <summary>
        /// Handles the user clicking on the screenshot button.
        /// </summary>
        /// <param name="sender">Object sending the event.</param>
        /// <param name="e">Event arguments.</param>
        private void ButtonScreenshotClick(object sender, RoutedEventArgs e)
        {
            if (null == this.sensorChooser || null == this.sensorChooser.Kinect)
            {
                this.statusBarText.Text = Properties.Resources.ConnectDeviceFirst;
                return;
            }

            BitmapSource source = this.Backdrop.Source as BitmapSource;
            if (null == source)
            {
                return;
            }

            int colorWidth = source.PixelWidth;
            int colorHeight = source.PixelHeight;

            // Create a render target that we'll render our controls to.
            var renderBitmap = new RenderTargetBitmap(
                colorWidth,
                colorHeight,
                96.0,
                96.0,
                PixelFormats.Pbgra32);

            var dv = new DrawingVisual();
            using (var dc = dv.RenderOpen())
            {
                // Render the backdrop.
                var backdropBrush = new VisualBrush(Backdrop);
                dc.DrawRectangle(
                    backdropBrush,
                    null,
                    new Rect(new Point(), new Size(colorWidth, colorHeight)));

                // Render the foreground.
                var colorBrush = new VisualBrush(MaskedColorImages);
                dc.DrawRectangle(
                    colorBrush,
                    null,
                    new Rect(new Point(), new Size(colorWidth, colorHeight)));
            }

            renderBitmap.Render(dv);
    
            // Create a bitmap encoder that knows how to save a .png file.
            BitmapEncoder encoder = new PngBitmapEncoder();

            // Create frame from the writable bitmap and add to encoder.
            encoder.Frames.Add(BitmapFrame.Create(renderBitmap));

            // Construct path for the file to be written.
            var time = DateTime.Now.ToString(
                "hh'-'mm'-'ss",
                CultureInfo.CurrentUICulture.DateTimeFormat);
            var myPhotos = Environment.GetFolderPath(Environment.SpecialFolder.MyPictures);
            var path = Path.Combine(myPhotos, "KinectSnapshot-" + time + ".png");

            // Write the new file to disk.
            try
            {
                using (var fs = new FileStream(path, FileMode.Create))
                {
                    encoder.Save(fs);
                }

                this.statusBarText.Text = string.Format(
                    CultureInfo.InvariantCulture,
                    Properties.Resources.ScreenshotWriteSuccess,
                    path);
            }
            catch (IOException)
            {
                this.statusBarText.Text = string.Format(
                    CultureInfo.InvariantCulture,
                    Properties.Resources.ScreenshotWriteFailed,
                    path);
            }
        }
        
        /// <summary>
        /// Handles the checking or unchecking of the near mode check box.
        /// </summary>
        /// <param name="sender">Object sending the event.</param>
        /// <param name="e">Event arguments.</param>
        private void CheckBoxNearModeChanged(object sender, RoutedEventArgs e)
        {
            if (null == this.sensorChooser || null == this.sensorChooser.Kinect)
            {
                return;
            }

            // will not function on non-Kinect for Windows devices
            try
            {
                this.sensorChooser.Kinect.DepthStream.Range =
                    this.checkBoxNearMode.IsChecked.GetValueOrDefault()
                    ? DepthRange.Near
                    : DepthRange.Default;
            }
            catch (InvalidOperationException)
            {
                // Ignore the exception.
            }
        }

        /// <summary>
        /// Handles a left button press in the UserView control.
        /// </summary>
        /// <param name="sender">Object sending the event.</param>
        /// <param name="e">Event arguments.</param>
        private void UserViewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            // Determine which pixel in the depth image was clicked.
            Point p = e.GetPosition(this.UserView);
            int depthX = (int)(p.X * this.userViewBitmap.PixelWidth / this.UserView.ActualWidth);
            int depthY = (int)(p.Y * this.userViewBitmap.PixelHeight / this.UserView.ActualHeight);
            int pixelIndex = (depthY * this.userViewBitmap.PixelWidth) + depthX;
            if (pixelIndex >= 0 && pixelIndex < this.depthData.Length)
            {
                // Find the player index in the depth image. If non-zero, toggle background removal
                // for the corresponding user.
                short playerIndex = this.depthData[pixelIndex].PlayerIndex;
                if (playerIndex > 0)
                {
                    // playerIndex is 1-based, skeletons array is 0-based, so subtract 1.
                    this.ToggleUserTracking(this.skeletons[playerIndex - 1].TrackingId);
                }
            }
        }

        /// <summary>
        /// Toggle background removal tracking on/off for a particular user's tracking ID.
        /// </summary>
        /// <param name="trackingId">The tracking ID for the user to toggle.</param>
        /// <remarks>
        /// If the maximum number of users is already being tracked, this method will have the
        /// side-effect of turning tracking off for the user that was tracked earliest, and
        /// replacing that user with the one represented by the trackingId parameter.
        /// </remarks>
        private void ToggleUserTracking(int trackingId)
        {
            if (TrackableUser.InvalidTrackingId != trackingId)
            {
                DateTime minTimestamp = DateTime.MaxValue;
                TrackableUser trackedUser = null;
                TrackableUser staleUser = null;

                // Attempt to find a TrackableUser with a matching TrackingId.
                foreach (var user in this.trackableUsers)
                {
                    if (user.TrackingId == trackingId)
                    {
                        // Yes, this TrackableUser has a matching TrackingId.
                        trackedUser = user;
                    }

                    // Find the "stale" user (the trackable user with the earliest timestamp).
                    if (user.Timestamp < minTimestamp)
                    {
                        staleUser = user;
                        minTimestamp = user.Timestamp;
                    }
                }

                if (null != trackedUser)
                {
                    // User is being tracked: toggle to not tracked.
                    trackedUser.TrackingId = TrackableUser.InvalidTrackingId;
                }
                else
                {
                    // User is not currently being tracked: start tracking, by reusing
                    // the "stale" trackable user.
                    staleUser.TrackingId = trackingId;
                }
            }
        }

        /// <summary>
        /// Instructs the skeleton stream to track specific skeletons.
        /// </summary>
        private void UpdateChosenSkeletons()
        {
            KinectSensor sensor = this.sensorChooser.Kinect;
            if (null != sensor)
            {
                // Choose which of the users will be tracked in the next frame.
                int trackedUserCount = 0;
                for (int i = 0; i < MaxUsers && trackedUserCount < this.trackingIds.Length; ++i)
                {
                    // Get the trackable user for consideration.
                    var trackableUser = this.trackableUsers[this.nextUserIndex];
                    if (trackableUser.IsTracked)
                    {
                        // If this user is currently being tracked, copy its TrackingId to the
                        // array of chosen users.
                        this.trackingIds[trackedUserCount++] = trackableUser.TrackingId;
                    }

                    // Update the index for the next user to be considered.
                    this.nextUserIndex = (this.nextUserIndex + 1) % MaxUsers;
                }

                // Fill any unused slots with InvalidTrackingId.
                for (int i = trackedUserCount; i < this.trackingIds.Length; ++i)
                {
                    this.trackingIds[i] = TrackableUser.InvalidTrackingId;
                }

                // Pass the chosen tracking IDs to the skeleton stream.
                sensor.SkeletonStream.ChooseSkeletons(this.trackingIds[0], this.trackingIds[1]);
            }
        }
    }
}
