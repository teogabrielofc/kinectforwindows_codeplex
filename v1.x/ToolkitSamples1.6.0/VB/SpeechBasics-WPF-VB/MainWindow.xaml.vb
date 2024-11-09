Imports Microsoft.Speech.Recognition
Imports Microsoft.Speech.AudioFormat
Imports Microsoft.Kinect
Imports System.Text
Imports System.IO
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

Namespace Microsoft.Samples.Kinect.SpeechBasics

	''' <summary>
	''' Interaction logic for MainWindow.xaml
	''' </summary>
	<System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1001:TypesThatOwnDisposableFieldsShouldBeDisposable", Justification := "In a full-fledged application, the SpeechRecognitionEngine object should be properly disposed. For the sake of simplicity, we're omitting that code in this sample.")> _
	Partial Public Class MainWindow
		Inherits Window

		''' <summary>
		''' Map between each direction and the direction immediately to its right.
		''' </summary>
		Private Shared ReadOnly TurnRight As New Dictionary(Of Direction, Direction)() From {{ Direction.Up, Direction.Right }, { Direction.Right, Direction.Down }, { Direction.Down, Direction.Left }, { Direction.Left, Direction.Up }}

		''' <summary>
		''' Map between each direction and the direction immediately to its left.
		''' </summary>
		Private Shared ReadOnly TurnLeft As New Dictionary(Of Direction, Direction)() From {{ Direction.Up, Direction.Left }, { Direction.Right, Direction.Up }, { Direction.Down, Direction.Right }, { Direction.Left, Direction.Down }}

		''' <summary>
		''' Map between each direction and the displacement unit it represents.
		''' </summary>
		Private Shared ReadOnly Displacements As New Dictionary(Of Direction, Point)() From {{ Direction.Up, New Point With {.X = 0, .Y = -1} }, { Direction.Right, New Point With {.X = 1, .Y = 0} }, { Direction.Down, New Point With {.X = 0, .Y = 1} }, { Direction.Left, New Point With {.X = -1, .Y = 0} }}

		''' <summary>
		''' Active Kinect sensor.
		''' </summary>
		Private sensor As KinectSensor

		''' <summary>
		''' Speech recognition engine using audio data from Kinect.
		''' </summary>
		Private speechEngine As SpeechRecognitionEngine

		''' <summary>
		''' Current direction where turtle is facing.
		''' </summary>
		Private curDirection As Direction = Direction.Up

		''' <summary>
		''' List of all UI span elements used to select recognized text.
		''' </summary>
		Private recognitionSpans As List(Of Span)

		''' <summary>
		''' Initializes a new instance of the MainWindow class.
		''' </summary>
		Public Sub New()
			InitializeComponent()
		End Sub

		''' <summary>
		''' Enumeration of directions in which turtle may be facing.
		''' </summary>
		Private Enum Direction
			Up
			Down
			Left
			Right
		End Enum

		''' <summary>
		''' Gets the metadata for the speech recognizer (acoustic model) most suitable to
		''' process audio from Kinect device.
		''' </summary>
		''' <returns>
		''' RecognizerInfo if found, <code>null</code> otherwise.
		''' </returns>
		Private Shared Function GetKinectRecognizer() As RecognizerInfo
			For Each recognizer As RecognizerInfo In SpeechRecognitionEngine.InstalledRecognizers()
                Dim value As String = ""
				recognizer.AdditionalInfo.TryGetValue("Kinect", value)
				If "True".Equals(value, StringComparison.OrdinalIgnoreCase) AndAlso "en-US".Equals(recognizer.Culture.Name, StringComparison.OrdinalIgnoreCase) Then
					Return recognizer
				End If
			Next recognizer

			Return Nothing
		End Function

		''' <summary>
		''' Execute initialization tasks.
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

			Dim ri As RecognizerInfo = GetKinectRecognizer()

			If Nothing IsNot ri Then
				recognitionSpans = New List(Of Span) From {forwardSpan, backSpan, rightSpan, leftSpan}

				Me.speechEngine = New SpeechRecognitionEngine(ri.Id)

'''                **************************************************************
'''                * 
'''                * Use this code to create grammar programmatically rather than from
'''                * a grammar file.
'''                * 
'''                * var directions = new Choices();
'''                * directions.Add(new SemanticResultValue("forward", "FORWARD"));
'''                * directions.Add(new SemanticResultValue("forwards", "FORWARD"));
'''                * directions.Add(new SemanticResultValue("straight", "FORWARD"));
'''                * directions.Add(new SemanticResultValue("backward", "BACKWARD"));
'''                * directions.Add(new SemanticResultValue("backwards", "BACKWARD"));
'''                * directions.Add(new SemanticResultValue("back", "BACKWARD"));
'''                * directions.Add(new SemanticResultValue("turn left", "LEFT"));
'''                * directions.Add(new SemanticResultValue("turn right", "RIGHT"));
'''                *
'''                * var gb = new GrammarBuilder { Culture = ri.Culture };
'''                * gb.Append(directions);
'''                *
'''                * var g = new Grammar(gb);
'''                * 
'''                ***************************************************************

				' Create a grammar from grammar definition XML file.
				Using memoryStream = New MemoryStream(Encoding.ASCII.GetBytes(My.Resources.SpeechGrammar))
					Dim g = New Grammar(memoryStream)
					speechEngine.LoadGrammar(g)
				End Using

				AddHandler speechEngine.SpeechRecognized, AddressOf SpeechRecognized
				AddHandler speechEngine.SpeechRecognitionRejected, AddressOf SpeechRejected

				speechEngine.SetInputToAudioStream(sensor.AudioSource.Start(), New SpeechAudioFormatInfo(EncodingFormat.Pcm, 16000, 16, 1, 32000, 2, Nothing))
				speechEngine.RecognizeAsync(RecognizeMode.Multiple)
			Else
				Me.statusBarText.Text = My.Resources.NoSpeechRecognizer
			End If
		End Sub

		''' <summary>
		''' Execute uninitialization tasks.
		''' </summary>
		''' <param name="sender">object sending the event.</param>
		''' <param name="e">event arguments.</param>
		Private Sub WindowClosing(ByVal sender As Object, ByVal e As CancelEventArgs)
			If Nothing IsNot Me.sensor Then
				Me.sensor.AudioSource.Stop()

				Me.sensor.Stop()
				Me.sensor = Nothing
			End If

			If Nothing IsNot Me.speechEngine Then
				RemoveHandler Me.speechEngine.SpeechRecognized, AddressOf SpeechRecognized
				RemoveHandler Me.speechEngine.SpeechRecognitionRejected, AddressOf SpeechRejected
				Me.speechEngine.RecognizeAsyncStop()
			End If
		End Sub

		''' <summary>
		''' Remove any highlighting from recognition instructions.
		''' </summary>
		Private Sub ClearRecognitionHighlights()
			For Each span As Span In recognitionSpans
                span.Foreground = DirectCast(Me.FindResource("MediumGreyBrush"), Brush)
                span.FontWeight = FontWeights.Normal
			Next span
		End Sub

		''' <summary>
		''' Handler for recognized speech events.
		''' </summary>
		''' <param name="sender">object sending the event.</param>
		''' <param name="e">event arguments.</param>
		Private Sub SpeechRecognized(ByVal sender As Object, ByVal e As SpeechRecognizedEventArgs)
			' Speech utterance confidence below which we treat speech as if it hadn't been heard
			Const ConfidenceThreshold As Double = 0.3

			' Number of degrees in a right angle.
			Const DegreesInRightAngle As Integer = 90

			' Number of pixels turtle should move forwards or backwards each time.
			Const DisplacementAmount As Integer = 60

			ClearRecognitionHighlights()

			If e.Result.Confidence >= ConfidenceThreshold Then
				Select Case e.Result.Semantics.Value.ToString()
					Case "FORWARD"
						forwardSpan.Foreground = Brushes.DeepSkyBlue
						forwardSpan.FontWeight = FontWeights.Bold
						turtleTranslation.X = (playArea.Width + turtleTranslation.X + (DisplacementAmount * Displacements(curDirection).X)) Mod playArea.Width
						turtleTranslation.Y = (playArea.Height + turtleTranslation.Y + (DisplacementAmount * Displacements(curDirection).Y)) Mod playArea.Height

					Case "BACKWARD"
						backSpan.Foreground = Brushes.DeepSkyBlue
						backSpan.FontWeight = FontWeights.Bold
						turtleTranslation.X = (playArea.Width + turtleTranslation.X - (DisplacementAmount * Displacements(curDirection).X)) Mod playArea.Width
						turtleTranslation.Y = (playArea.Height + turtleTranslation.Y - (DisplacementAmount * Displacements(curDirection).Y)) Mod playArea.Height

					Case "LEFT"
						leftSpan.Foreground = Brushes.DeepSkyBlue
						leftSpan.FontWeight = FontWeights.Bold
						curDirection = TurnLeft(curDirection)

						' We take a left turn to mean a counter-clockwise right angle rotation for the displayed turtle.
						turtleRotation.Angle -= DegreesInRightAngle

					Case "RIGHT"
						rightSpan.Foreground = Brushes.DeepSkyBlue
						rightSpan.FontWeight = FontWeights.Bold
						curDirection = TurnRight(curDirection)

						' We take a right turn to mean a clockwise right angle rotation for the displayed turtle.
						turtleRotation.Angle += DegreesInRightAngle
				End Select
			End If
		End Sub

		''' <summary>
		''' Handler for rejected speech events.
		''' </summary>
		''' <param name="sender">object sending the event.</param>
		''' <param name="e">event arguments.</param>
		Private Sub SpeechRejected(ByVal sender As Object, ByVal e As SpeechRecognitionRejectedEventArgs)
			ClearRecognitionHighlights()
		End Sub
	End Class
End Namespace
