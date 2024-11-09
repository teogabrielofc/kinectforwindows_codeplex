Imports Microsoft.Kinect
Imports System.Threading
Imports System.IO
Imports System.Globalization
Imports System.ComponentModel

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

Namespace Microsoft.Samples.Kinect.AudioBasics

    ''' <summary>
    ''' Interaction logic for MainWindow.xaml.
    ''' </summary>
    Partial Public Class MainWindow
        Inherits Window

        ''' <summary>
        ''' Number of milliseconds between each read of audio data from the stream.
        ''' </summary>
        Private Const AudioPollingInterval As Integer = 50

        ''' <summary>
        ''' Number of samples captured from Kinect audio stream each millisecond.
        ''' </summary>
        Private Const SamplesPerMillisecond As Integer = 16

        ''' <summary>
        ''' Number of bytes in each Kinect audio stream sample.
        ''' </summary>
        Private Const BytesPerSample As Integer = 2

        ''' <summary>
        ''' Number of audio samples represented by each column of pixels in wave bitmap.
        ''' </summary>
        Private Const SamplesPerColumn As Integer = 40

        ''' <summary>
        ''' Width of bitmap that stores audio stream energy data ready for visualization.
        ''' </summary>
        Private Const EnergyBitmapWidth As Integer = 780

        ''' <summary>
        ''' Height of bitmap that stores audio stream energy data ready for visualization.
        ''' </summary>
        Private Const EnergyBitmapHeight As Integer = 195

        ''' <summary>
        ''' Bitmap that contains constructed visualization for audio stream energy, ready to
        ''' be displayed. It is a 2-color bitmap with white as background color and blue as
        ''' foreground color.
        ''' </summary>
        Private ReadOnly energyBitmap As WriteableBitmap

        ''' <summary>
        ''' Rectangle representing the entire energy bitmap area. Used when drawing background
        ''' for energy visualization.
        ''' </summary>
        Private ReadOnly fullEnergyRect As New Int32Rect(0, 0, EnergyBitmapWidth, EnergyBitmapHeight)

        ''' <summary>
        ''' Array of background-color pixels corresponding to an area equal to the size of whole energy bitmap.
        ''' </summary>
        Private ReadOnly backgroundPixels(EnergyBitmapWidth * EnergyBitmapHeight - 1) As Byte

        ''' <summary>
        ''' Buffer used to hold audio data read from audio stream.
        ''' </summary>
        Private ReadOnly audioBuffer(AudioPollingInterval * SamplesPerMillisecond * BytesPerSample - 1) As Byte

        ''' <summary>
        ''' Buffer used to store audio stream energy data as we read audio.
        ''' 
        ''' We store 25% more energy values than we strictly need for visualization to allow for a smoother
        ''' stream animation effect, since rendering happens on a different schedule with respect to audio
        ''' capture.
        ''' </summary>
        Private ReadOnly energy(CUInt(EnergyBitmapWidth * 1.25) - 1) As Double

        ''' <summary>
        ''' Active Kinect sensor.
        ''' </summary>
        Private sensor As KinectSensor

        ''' <summary>
        ''' Stream of audio being captured by Kinect sensor.
        ''' </summary>
        Private audioStream As Stream

        ''' <summary>
        ''' <code>true</code> if audio is currently being read from Kinect stream, <code>false</code> otherwise.
        ''' </summary>
        Private reading As Boolean

        ''' <summary>
        ''' Thread that is reading audio from Kinect stream.
        ''' </summary>
        Private readingThread As Thread

        ''' <summary>
        ''' Array of foreground-color pixels corresponding to a line as long as the energy bitmap is tall.
        ''' This gets re-used while constructing the energy visualization.
        ''' </summary>
        Private foregroundPixels() As Byte

        ''' <summary>
        ''' Sum of squares of audio samples being accumulated to compute the next energy value.
        ''' </summary>
        Private accumulatedSquareSum As Double

        ''' <summary>
        ''' Number of audio samples accumulated so far to compute the next energy value.
        ''' </summary>
        Private accumulatedSampleCount As Integer

        ''' <summary>
        ''' Index of next element available in audio energy buffer.
        ''' </summary>
        Private energyIndex As Integer

        ''' <summary>
        ''' Number of newly calculated audio stream energy values that have not yet been
        ''' displayed.
        ''' </summary>
        Private newEnergyAvailable As Integer

        ''' <summary>
        ''' Error between time slice we wanted to display and time slice that we ended up
        ''' displaying, given that we have to display in integer pixels.
        ''' </summary>
        Private energyError As Double

        ''' <summary>
        ''' Last time energy visualization was rendered to screen.
        ''' </summary>
        Private lastEnergyRefreshTime? As Date

        ''' <summary>
        ''' Index of first energy element that has never (yet) been displayed to screen.
        ''' </summary>
        Private energyRefreshIndex As Integer

        ''' <summary>
        ''' Initializes a new instance of the MainWindow class.
        ''' </summary>
        Public Sub New()
            InitializeComponent()
            Dim foregroundColor As Color = DirectCast(Me.FindResource("KinectPurpleColor"), Color)
            energyBitmap = New WriteableBitmap(EnergyBitmapWidth, EnergyBitmapHeight, 96, 96, PixelFormats.Indexed1, New BitmapPalette(New List(Of Color) From {Colors.White, foregroundColor}))
        End Sub

        ''' <summary>
        ''' Execute initialization tasks.
        ''' </summary>
        ''' <param name="sender">object sending the event.</param>
        ''' <param name="e">event arguments.</param>
        Private Sub WindowLoaded(ByVal sender As Object, ByVal e As RoutedEventArgs)
            ' Look through all sensors and start the first connected one.
            ' This requires that a Kinect is connected at the time of app startup.
            ' To make your app robust against plug/unplug, it is recommended to use KinectSensorChooser provided in Microsoft.Kinect.Toolkit (See components in Toolkit Browser).
            For Each potentialSensor In KinectSensor.KinectSensors
                If potentialSensor.Status = KinectStatus.Connected Then
                    Me.sensor = potentialSensor
                    Exit For
                End If
            Next potentialSensor

            If Nothing IsNot Me.sensor Then
                Try
                    ' Start the sensor!
                    Me.sensor.Start()
                Catch e1 As IOException
                    ' Some other application is streaming from the same Kinect sensor
                    Me.sensor = Nothing
                End Try
            End If

            If Nothing Is Me.sensor Then
                Me.statusBarText.Text = My.Resources.NoKinectReady
                Return
            End If

            ' Initialize foreground pixels
            Me.foregroundPixels = New Byte(EnergyBitmapHeight - 1) {}
            For i As Integer = 0 To Me.foregroundPixels.Length - 1
                Me.foregroundPixels(i) = &HFF
            Next i

            Me.waveDisplay.Source = Me.energyBitmap

            AddHandler CompositionTarget.Rendering, AddressOf UpdateEnergy

            AddHandler Me.sensor.AudioSource.BeamAngleChanged, AddressOf AudioSourceBeamChanged
            AddHandler Me.sensor.AudioSource.SoundSourceAngleChanged, AddressOf AudioSourceSoundSourceAngleChanged

            ' Start streaming audio!
            Me.audioStream = Me.sensor.AudioSource.Start()

            ' Use a separate thread for capturing audio because audio stream read operations
            ' will block, and we don't want to block main UI thread.
            Me.reading = True
            Me.readingThread = New Thread(AddressOf AudioReadingThread)
            Me.readingThread.Start()
        End Sub

        ''' <summary>
        ''' Execute uninitialization tasks.
        ''' </summary>
        ''' <param name="sender">object sending the event.</param>
        ''' <param name="e">event arguments.</param>
        Private Sub WindowClosing(ByVal sender As Object, ByVal e As CancelEventArgs)
            ' Tell audio reading thread to stop and wait for it to finish.
            Me.reading = False
            If Nothing IsNot readingThread Then
                readingThread.Join()
            End If

            If Nothing IsNot Me.sensor Then
                RemoveHandler CompositionTarget.Rendering, AddressOf UpdateEnergy

                RemoveHandler Me.sensor.AudioSource.BeamAngleChanged, AddressOf AudioSourceBeamChanged
                RemoveHandler Me.sensor.AudioSource.SoundSourceAngleChanged, AddressOf AudioSourceSoundSourceAngleChanged
                Me.sensor.AudioSource.Stop()

                Me.sensor.Stop()
                Me.sensor = Nothing
            End If
        End Sub

        ''' <summary>
        ''' Handles event triggered when audio beam angle changes.
        ''' </summary>
        ''' <param name="sender">object sending the event.</param>
        ''' <param name="e">event arguments.</param>
        Private Sub AudioSourceBeamChanged(ByVal sender As Object, ByVal e As BeamAngleChangedEventArgs)
            beamRotation.Angle = -e.Angle

            beamAngleText.Text = String.Format(CultureInfo.CurrentCulture, My.Resources.BeamAngle, e.Angle.ToString("0", CultureInfo.CurrentCulture))
        End Sub

        ''' <summary>
        ''' Handles event triggered when sound source angle changes.
        ''' </summary>
        ''' <param name="sender">object sending the event.</param>
        ''' <param name="e">event arguments.</param>
        Private Sub AudioSourceSoundSourceAngleChanged(ByVal sender As Object, ByVal e As SoundSourceAngleChangedEventArgs)
            ' Maximum possible confidence corresponds to this gradient width
            Const MinGradientWidth As Double = 0.04

            ' Set width of mark based on confidence.
            ' A confidence of 0 would give us a gradient that fills whole area diffusely.
            ' A confidence of 1 would give us the narrowest allowed gradient width.
            Dim halfWidth As Double = Math.Max((1 - e.ConfidenceLevel), MinGradientWidth) / 2

            ' Update the gradient representing sound source position to reflect confidence
            Me.sourceGsPre.Offset = Math.Max(Me.sourceGsMain.Offset - halfWidth, 0)
            Me.sourceGsPost.Offset = Math.Min(Me.sourceGsMain.Offset + halfWidth, 1)

            ' Rotate gradient to match angle
            sourceRotation.Angle = -e.Angle

            sourceAngleText.Text = String.Format(CultureInfo.CurrentCulture, My.Resources.SourceAngle, e.Angle.ToString("0", CultureInfo.CurrentCulture))
            sourceConfidenceText.Text = String.Format(CultureInfo.CurrentCulture, My.Resources.SourceConfidence, e.ConfidenceLevel.ToString("0.00", CultureInfo.CurrentCulture))
        End Sub

        ''' <summary>
        ''' Handles polling audio stream and updating visualization every tick.
        ''' </summary>
        Private Sub AudioReadingThread()
            ' Bottom portion of computed energy signal that will be discarded as noise.
            ' Only portion of signal above noise floor will be displayed.
            Const EnergyNoiseFloor As Double = 0.2

            Do While Me.reading
                Dim readCount As Integer = audioStream.Read(audioBuffer, 0, audioBuffer.Length)

                ' Calculate energy corresponding to captured audio in the dispatcher
                ' (UI Thread) context, so that rendering code doesn't need to
                ' perform additional synchronization.
                Dispatcher.BeginInvoke(New Action(Sub()
                                                      ' compute the sum of squares of audio samples that will get accumulated
                                                      ' into a single energy value.
                                                      ' Each energy value will represent the logarithm of the mean of the
                                                      ' sum of squares of a group of audio samples.
                                                      ' Renormalize signal above noise floor to [0,1] range.
                                                      For i As Integer = 0 To readCount - 1 Step 2
                                                          Dim audioSample As Integer = BitConverter.ToInt16(audioBuffer, i)
                                                          Me.accumulatedSquareSum += audioSample * audioSample
                                                          Me.accumulatedSampleCount += 1
                                                          If Me.accumulatedSampleCount < SamplesPerColumn Then
                                                              Continue For
                                                          End If
                                                          Dim meanSquare As Double = Me.accumulatedSquareSum / SamplesPerColumn
                                                          Dim amplitude As Double = Math.Log(meanSquare) / Math.Log(Integer.MaxValue)
                                                          Me.energy(Me.energyIndex) = Math.Max(0, amplitude - EnergyNoiseFloor) / (1 - EnergyNoiseFloor)
                                                          Me.energyIndex = (Me.energyIndex + 1) Mod Me.energy.Length
                                                          Me.accumulatedSquareSum = 0
                                                          Me.accumulatedSampleCount = 0
                                                          Me.newEnergyAvailable += 1
                                                      Next i
                                                  End Sub))
            Loop
        End Sub

        ''' <summary>
        ''' Handles rendering energy visualization into a bitmap.
        ''' </summary>
        ''' <param name="sender">object sending the event.</param>
        ''' <param name="e">event arguments.</param>
        Private Sub UpdateEnergy(ByVal sender As Object, ByVal e As EventArgs)
            ' Calculate how many energy samples we need to advance since the last update in order to
            ' have a smooth animation effect
            Dim now As Date = Date.UtcNow
            Dim previousRefreshTime? As Date = Me.lastEnergyRefreshTime
            Me.lastEnergyRefreshTime = now

            ' No need to refresh if there is no new energy available to render
            If Me.newEnergyAvailable <= 0 Then
                Return
            End If

            If previousRefreshTime IsNot Nothing Then
                Dim energyToAdvance As Double = Me.energyError + (((now.Subtract(previousRefreshTime.Value)).TotalMilliseconds * SamplesPerMillisecond) / SamplesPerColumn)
                Dim energySamplesToAdvance As Integer = Math.Min(Me.newEnergyAvailable, CInt(Fix(Math.Round(energyToAdvance))))
                Me.energyError = energyToAdvance - energySamplesToAdvance
                Me.energyRefreshIndex = (Me.energyRefreshIndex + energySamplesToAdvance) Mod Me.energy.Length
                Me.newEnergyAvailable -= energySamplesToAdvance
            End If

            ' clear background of energy visualization area
            Me.energyBitmap.WritePixels(fullEnergyRect, Me.backgroundPixels, EnergyBitmapWidth, 0)

            ' Draw each energy sample as a centered vertical bar, where the length of each bar is
            ' proportional to the amount of energy it represents.
            ' Time advances from left to right, with current time represented by the rightmost bar.
            Dim baseIndex As Integer = (Me.energyRefreshIndex + Me.energy.Length - EnergyBitmapWidth) Mod Me.energy.Length
            For i As Integer = 0 To EnergyBitmapWidth - 1
                Const HalfImageHeight As Integer = EnergyBitmapHeight \ 2

                ' Each bar has a minimum height of 1 (to get a steady signal down the middle) and a maximum height
                ' equal to the bitmap height.
                Dim barHeight As Integer = CInt(Math.Max(1.0, (Me.energy((baseIndex + i) Mod Me.energy.Length) * EnergyBitmapHeight)))

                ' Center bar vertically on image
                Dim barRect = New Int32Rect(i, HalfImageHeight - (barHeight \ 2), 1, barHeight)

                ' Draw bar in foreground color
                Me.energyBitmap.WritePixels(barRect, foregroundPixels, 1, 0)
            Next i
        End Sub
    End Class
End Namespace
