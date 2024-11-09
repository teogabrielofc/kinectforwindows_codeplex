Imports Microsoft.Kinect
Imports System.IO
Imports System.Globalization

'------------------------------------------------------------------------------
' <copyright file="MainWindow.xaml.cs" company="Microsoft">
' 	 
'	 Copyright 2013 Microsoft Corporation 
' 	 
'	Licensed under the Apache License, Version 2.0 (the "License"); 
'	you may not use this file except in compliance with the License.
'	You may obtain a copy of the License at
' 	 
'		 http://www.apache.org/licenses/LICENSE-2.0 
' 	 
'	Unless required by applicable law or agreed to in writing, software 
'	distributed under the License is distributed on an "AS IS" BASIS,
'	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
'	See the License for the specific language governing permissions and 
'	limitations under the License. 
' 	 
' </copyright>
'------------------------------------------------------------------------------

Namespace Microsoft.Samples.Kinect.ColorBasics

	''' <summary>
	''' Interaction logic for MainWindow.xaml
	''' </summary>
	Partial Public Class MainWindow
		Inherits Window

		''' <summary>
		''' Active Kinect sensor
		''' </summary>
		Private sensor As KinectSensor

		''' <summary>
		''' Bitmap that will hold color information
		''' </summary>
		Private colorBitmap As WriteableBitmap

		''' <summary>
		''' Intermediate storage for the color data received from the camera
		''' </summary>
		Private colorPixels() As Byte

		''' <summary>
		''' Initializes a new instance of the MainWindow class.
		''' </summary>
		Public Sub New()
			InitializeComponent()
		End Sub

		''' <summary>
		''' Execute startup tasks
		''' </summary>
		''' <param name="sender">object sending the event</param>
		''' <param name="e">event arguments</param>
		Private Sub WindowLoaded(ByVal sender As Object, ByVal e As RoutedEventArgs)
			' Look through all sensors and start the first connected one.
			' This requires that a Kinect is connected at the time of app startup.
			' To make your app robust against plug/unplug, it is recommended to use KinectSensorChooser.
			For Each potentialSensor In KinectSensor.KinectSensors
				If potentialSensor.Status = KinectStatus.Connected Then
					Me.sensor = potentialSensor
					Exit For
				End If
			Next potentialSensor

			If Nothing IsNot Me.sensor Then
				' Turn on the color stream to receive color frames
				Me.sensor.ColorStream.Enable(ColorImageFormat.RgbResolution640x480Fps30)

				' Allocate space to put the pixels we'll receive
				Me.colorPixels = New Byte(Me.sensor.ColorStream.FramePixelDataLength - 1){}

				' This is the bitmap we'll display on-screen
				Me.colorBitmap = New WriteableBitmap(Me.sensor.ColorStream.FrameWidth, Me.sensor.ColorStream.FrameHeight, 96.0, 96.0, PixelFormats.Bgr32, Nothing)

				' Set the image we display to point to the bitmap where we'll put the image data
				Me.Image.Source = Me.colorBitmap

				' Add an event handler to be called whenever there is new color frame data
				AddHandler Me.sensor.ColorFrameReady, AddressOf SensorColorFrameReady

				' Start the sensor!
				Try
					Me.sensor.Start()
				Catch e1 As IOException
					Me.sensor = Nothing
				End Try
			End If

			If Nothing Is Me.sensor Then
				Me.statusBarText.Text = "No ready Kinect found!"
			End If
		End Sub

		''' <summary>
		''' Execute shutdown tasks
		''' </summary>
		''' <param name="sender">object sending the event</param>
		''' <param name="e">event arguments</param>
		Private Sub WindowClosing(ByVal sender As Object, ByVal e As System.ComponentModel.CancelEventArgs)
			If Nothing IsNot Me.sensor Then
				Me.sensor.Stop()
			End If
		End Sub

		''' <summary>
		''' Event handler for Kinect sensor's ColorFrameReady event
		''' </summary>
		''' <param name="sender">object sending the event</param>
		''' <param name="e">event arguments</param>
		Private Sub SensorColorFrameReady(ByVal sender As Object, ByVal e As ColorImageFrameReadyEventArgs)
			Using colorFrame As ColorImageFrame = e.OpenColorImageFrame()
				If colorFrame IsNot Nothing Then
					' Copy the pixel data from the image to a temporary array
					colorFrame.CopyPixelDataTo(Me.colorPixels)

					' Write the pixel data into our bitmap
					Me.colorBitmap.WritePixels(New Int32Rect(0, 0, Me.colorBitmap.PixelWidth, Me.colorBitmap.PixelHeight), Me.colorPixels, Me.colorBitmap.PixelWidth * Len(New Integer), 0)
				End If
			End Using
		End Sub

		''' <summary>
		''' Handles the user clicking on the screenshot button
		''' </summary>
		''' <param name="sender">object sending the event</param>
		''' <param name="e">event arguments</param>
		Private Sub ButtonScreenshotClick(ByVal sender As Object, ByVal e As RoutedEventArgs)
			If Nothing Is Me.sensor Then
				Me.statusBarText.Text = "Connect a device first!"
				Return
			End If

			' create a png bitmap encoder which knows how to save a .png file
			Dim encoder As BitmapEncoder = New PngBitmapEncoder()

			' create frame from the writable bitmap and add to encoder
			encoder.Frames.Add(BitmapFrame.Create(Me.colorBitmap))

			Dim time As String = Date.Now.ToString("hh'-'mm'-'ss", CultureInfo.CurrentUICulture.DateTimeFormat)

			Dim myPhotos As String = Environment.GetFolderPath(Environment.SpecialFolder.MyPictures)

			Dim path As String = System.IO.Path.Combine(myPhotos, "KinectSnapshot-" & time & ".png")

			' write the new file to disk
			Try
				Using fs As New FileStream(path, FileMode.Create)
					encoder.Save(fs)
				End Using

				Me.statusBarText.Text = "Screenshot saved to " & path
			Catch e1 As Exception
				Me.statusBarText.Text = "Failed to write screenshot to " & path
			End Try
		End Sub
	End Class
End Namespace
