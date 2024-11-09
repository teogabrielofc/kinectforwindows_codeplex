//------------------------------------------------------------------------------
// <copyright file="SpeechStreamHandler.cs" company="Microsoft">
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
namespace Microsoft.Samples.Kinect.WebserverSpeech
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Text;

    using Microsoft.Kinect;
    using Microsoft.Samples.Kinect.Webserver.Sensor;
    using Microsoft.Samples.Kinect.Webserver.Sensor.Serialization;
    using Microsoft.Speech.AudioFormat;
    using Microsoft.Speech.Recognition;
    
    /// <summary>
    /// Implementation of ISensorStreamHandler that exposes speech events.
    /// </summary>
    public class SpeechStreamHandler : SensorStreamHandlerBase, IDisposable
    {
        /// <summary>
        /// JSON name of speech event category.
        /// </summary>
        private const string SpeechEventCategory = "speech";

        /// <summary>
        /// Name of property for setting speech grammar XML.
        /// </summary>
        private const string GrammarXmlPropertyName = "grammarXml";

        /// <summary>
        /// Context that allows this stream handler to communicate with its owner.
        /// </summary>
        private readonly SensorStreamHandlerContext ownerContext;

        /// <summary>
        /// Sensor providing data to speech stream.
        /// </summary>
        private KinectSensor sensor;

        /// <summary>
        /// true if speech stream is enabled.
        /// </summary>
        private bool speechIsEnabled;

        /// <summary>
        /// Grammar to be used for speech recognition.
        /// </summary>
        private Grammar grammar;

        /// <summary>
        /// Speech recognition engine using audio data from Kinect.
        /// </summary>
        private SpeechRecognitionEngine speechEngine;

        /// <summary>
        /// Initializes static members of the SpeechStreamHandler class.
        /// </summary>
        static SpeechStreamHandler()
        {
            Factory = new SpeechStreamHandlerFactory();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SpeechStreamHandler"/> class
        /// and associates it with a context that allows it to communicate with its owner.
        /// </summary>
        /// <param name="ownerContext">
        /// An instance of <see cref="SensorStreamHandlerContext"/> class.
        /// </param>
        internal SpeechStreamHandler(SensorStreamHandlerContext ownerContext)
        {
            this.ownerContext = ownerContext;

            this.AddStreamConfiguration(SpeechEventCategory, new StreamConfiguration(this.GetProperties, this.SetProperty));

            RecognizerInfo ri = GetKinectRecognizer();

            if (ri != null)
            {
                this.speechEngine = new SpeechRecognitionEngine(ri.Id);

                if (this.speechEngine != null)
                {
                    // disable speech engine adaptation feature
                    this.speechEngine.UpdateRecognizerSetting("AdaptationOn", 0);
                    this.speechEngine.UpdateRecognizerSetting("PersistedBackgroundAdaptation", 0);

                    this.speechEngine.AudioStateChanged += this.AudioStateChanged;
                    this.speechEngine.SpeechRecognitionRejected += this.SpeechRecognitionRejected;
                    this.speechEngine.SpeechRecognized += this.SpeechRecognized;
                }
            }
        }

        /// <summary>
        /// Gets a factory instance that creates SpeechStreamHandler objects.
        /// </summary>
        public static ISensorStreamHandlerFactory Factory { get; private set; }

        /// <summary>
        /// Disposes an instance of the <see cref="SpeechStreamHandler"/> class.
        /// </summary>
        public void Dispose()
        {
            if (this.speechEngine != null)
            {
                this.speechEngine.AudioStateChanged -= this.AudioStateChanged;
                this.speechEngine.SpeechRecognitionRejected -= this.SpeechRecognitionRejected;
                this.speechEngine.SpeechRecognized -= this.SpeechRecognized;

                // Avoid calling dispose on the the speech engine, as it can block for 30 seconds.
                this.speechEngine = null;
            }
        }

        /// <summary>
        /// Lets ISensorStreamHandler know that Kinect Sensor associated with this stream
        /// handler has changed.
        /// </summary>
        /// <param name="newSensor">
        /// New KinectSensor.
        /// </param>
        public override void OnSensorChanged(KinectSensor newSensor)
        {
            base.OnSensorChanged(newSensor);

            if (this.sensor != null)
            {
                if (this.speechEngine != null)
                {
                    this.StopRecognition();
                    this.speechEngine.SetInputToNull();
                    this.sensor.AudioSource.Stop();
                }
            }

            this.sensor = newSensor;

            if (newSensor != null)
            {
                if (this.speechEngine != null)
                {
                    this.speechEngine.SetInputToAudioStream(
                        newSensor.AudioSource.Start(), new SpeechAudioFormatInfo(EncodingFormat.Pcm, 16000, 16, 1, 32000, 2, null));
                    this.StartRecognition(this.grammar);
                }
            }
        }

        /// <summary>
        /// Gets the metadata for the speech recognizer (acoustic model) most suitable to
        /// process audio from Kinect device.
        /// </summary>
        /// <returns>
        /// RecognizerInfo if found, <code>null</code> otherwise.
        /// </returns>
        private static RecognizerInfo GetKinectRecognizer()
        {
            foreach (RecognizerInfo recognizer in SpeechRecognitionEngine.InstalledRecognizers())
            {
                string value;
                recognizer.AdditionalInfo.TryGetValue("Kinect", out value);
                if ("True".Equals(value, StringComparison.OrdinalIgnoreCase) && "en-US".Equals(recognizer.Culture.Name, StringComparison.OrdinalIgnoreCase))
                {
                    return recognizer;
                }
            }

            return null;
        }

        /// <summary>
        /// Gets the set of speech stream properties.
        /// </summary>
        /// <param name="propertyMap">
        /// Property name->value map where property values should be set.
        /// </param>
        private void GetProperties(Dictionary<string, object> propertyMap)
        {
            propertyMap.Add(KinectRequestHandler.EnabledPropertyName, this.speechIsEnabled);
        }

        /// <summary>
        /// Set a speech stream property value.
        /// </summary>
        /// <param name="propertyName">
        /// Name of property to set.
        /// </param>
        /// <param name="propertyValue">
        /// Property value to set.
        /// </param>
        /// <returns>
        /// null if property setting was successful, error message otherwise.
        /// </returns>
        private string SetProperty(string propertyName, object propertyValue)
        {
            bool recognized = true;

            if (propertyValue == null)
            {
                // None of the speech stream properties accept a null value
                return Properties.Resources.PropertyValueInvalidFormat;
            }

            try
            {
                switch (propertyName)
                {
                    case KinectRequestHandler.EnabledPropertyName:
                        this.SetSpeechStreamIsEnabled((bool)propertyValue);
                        break;

                    case GrammarXmlPropertyName:
                        this.LoadGrammarXml((string)propertyValue);
                        break;

                    default:
                        recognized = false;
                        break;
                }
            }
            catch (InvalidCastException)
            {
                return Properties.Resources.PropertyValueInvalidFormat;
            }
            catch (InvalidOperationException e)
            {
                return e.Message;
            }

            if (!recognized)
            {
                return Properties.Resources.PropertyNameUnrecognized;
            }

            return null;
        }

        /// <summary>
        /// Enable/Disable speech stream.
        /// </summary>
        /// <param name="isEnabled">
        /// New value for speech stream isEnabled property.
        /// </param>
        private void SetSpeechStreamIsEnabled(bool isEnabled)
        {
            this.speechIsEnabled = isEnabled;
        }

        /// <summary>
        /// Replaces the currently loaded speech grammar with the given grammar and starts
        /// recognizing speech.
        /// </summary>
        /// <param name="g">
        /// The new speech grammar to load.
        /// </param>
        /// <exception cref="InvalidOperationException">
        /// If speech engine could not start recognizing speech.
        /// </exception>
        private void StartRecognition(Grammar g)
        {
            if ((this.sensor != null) && (g != null))
            {
                this.speechEngine.LoadGrammar(g);
                this.speechEngine.RecognizeAsync(RecognizeMode.Multiple);
            }

            this.grammar = g;
        }

        /// <summary>
        /// Stops recognizing speech.
        /// </summary>
        private void StopRecognition()
        {
            this.speechEngine.RecognizeAsyncStop();
            this.speechEngine.UnloadAllGrammars();
        }

        /// <summary>
        /// Replaces the currently loaded speech grammar with the given grammar.
        /// </summary>
        /// <param name="grammarXml">
        /// The new speech grammar as an XML string.
        /// </param>
        private void LoadGrammarXml(string grammarXml)
        {
            this.StopRecognition();

            if (!string.IsNullOrEmpty(grammarXml))
            {
                using (var memoryStream = new MemoryStream(Encoding.UTF8.GetBytes(grammarXml)))
                {
                    Grammar newGrammar;

                    try
                    {
                        newGrammar = new Grammar(memoryStream);
                    }
                    catch (ArgumentException e)
                    {
                        throw new InvalidOperationException("Requested grammar might not contain a root rule", e);
                    }
                    catch (FormatException e)
                    {
                        throw new InvalidOperationException("Requested grammar was specified with an invalid format", e);
                    }

                    this.StartRecognition(newGrammar);
                }
            }
        }

        /// <summary>
        /// Handles AudioStateChanged events from the speech recognition engine.
        /// </summary>
        /// <param name="sender">Event sender.</param>
        /// <param name="args">Event arguments.</param>
        private async void AudioStateChanged(object sender, AudioStateChangedEventArgs args)
        {
            var message = new AudioStateChangedSpeechMessage(args);
            await this.ownerContext.SendEventMessageAsync(message);
        }

        /// <summary>
        /// Handles SpeechRecognitionRejected events from the speech recognition engine.
        /// </summary>
        /// <param name="sender">Event sender.</param>
        /// <param name="args">Event arguments.</param>
        private async void SpeechRecognitionRejected(object sender, SpeechRecognitionRejectedEventArgs args)
        {
            var message = new RecognitionRejectedSpeechMessage(args);
            await this.ownerContext.SendEventMessageAsync(message);
        }

        /// <summary>
        /// Handles SpeechRecognized events from the speech recognition engine.
        /// </summary>
        /// <param name="sender">Event sender.</param>
        /// <param name="args">Event arguments.</param>
        private async void SpeechRecognized(object sender, SpeechRecognizedEventArgs args)
        {
            var message = new RecognizedSpeechMessage(args);
            await this.ownerContext.SendEventMessageAsync(message);
        }

        /// <summary>
        /// Serializable representation of a speech stream message to send to client.
        /// </summary>
        [SuppressMessage("StyleCop.CSharp.NamingRules", "SA1300:ElementMustBeginWithUpperCaseLetter", Justification = "Lower case names allowed for JSON serialization.")]
        private class AudioStateChangedSpeechMessage : EventMessage
        {
            /// <summary>
            /// Initializes a new instance of the AudioStateChangedSpeechMessage class.
            /// </summary>
            /// <param name="args">Event arguments.</param>
            internal AudioStateChangedSpeechMessage(AudioStateChangedEventArgs args)
            {
                this.category = SpeechStreamHandler.SpeechEventCategory;
                this.eventType = "audioStateChanged";

                this.audioStateChanged = new MessageArgs();
                switch (args.AudioState)
                {
                    case AudioState.Stopped:
                        this.audioStateChanged.audioState = "stopped";
                        break;

                    case AudioState.Silence:
                        this.audioStateChanged.audioState = "silence";
                        break;

                    case AudioState.Speech:
                        this.audioStateChanged.audioState = "speech";
                        break;
                }
            }

            /// <summary>
            /// Gets or sets the type-specific properties of the message.
            /// </summary>
            public MessageArgs audioStateChanged { get; set; }

            /// <summary>
            /// Represents the type-specific properties of the message.
            /// </summary>
            public class MessageArgs
            {
                /// <summary>
                /// Gets or sets the new audio state.
                /// </summary>
                public string audioState { get; set; }
            }
        }

        /// <summary>
        /// Serializable representation of a recognition rejected speech stream message to send to client.
        /// </summary>
        [SuppressMessage("StyleCop.CSharp.NamingRules", "SA1300:ElementMustBeginWithUpperCaseLetter", Justification = "Lower case names allowed for JSON serialization.")]
        private class RecognitionRejectedSpeechMessage : EventMessage
        {
            /// <summary>
            /// Initializes a new instance of the RecognitionRejectedSpeechMessage class.
            /// </summary>
            /// <param name="args">Event arguments.</param>
            internal RecognitionRejectedSpeechMessage(SpeechRecognitionRejectedEventArgs args)
            {
                this.category = SpeechStreamHandler.SpeechEventCategory;
                this.eventType = "recognitionRejected";
            }

            /// <summary>
            /// Gets or sets the type-specific properties of the message.
            /// </summary>
            public MessageArgs recognitionRejected { get; set; }

            /// <summary>
            /// Represents the type-specific properties of the message.
            /// </summary>
            public class MessageArgs
            {
                /// <summary>
                /// Gets or sets the alternates.
                /// </summary>
                public SpeechRecognizedPhrase[] alternates { get; set; }
            }
        }

        /// <summary>
        /// Serializable representation of a speech recognized speech stream message to send to client.
        /// </summary>
        [SuppressMessage("StyleCop.CSharp.NamingRules", "SA1300:ElementMustBeginWithUpperCaseLetter", Justification = "Lower case names allowed for JSON serialization.")]
        private class RecognizedSpeechMessage : EventMessage
        {
            /// <summary>
            /// Initializes a new instance of the RecognizedSpeechMessage class.
            /// </summary>
            /// <param name="args">Event arguments.</param>
            internal RecognizedSpeechMessage(SpeechRecognizedEventArgs args)
            {
                this.category = SpeechStreamHandler.SpeechEventCategory;
                this.eventType = "recognized";

                SpeechSemanticValue semanticValue = null;
                try
                {
                    semanticValue = new SpeechSemanticValue(args.Result.Semantics);
                }
                catch (InvalidOperationException)
                {
                    // Failed to parse semantics, leave it empty.
                }

                this.recognized = new MessageArgs
                {
                    grammar = args.Result.Grammar.Name,
                    text = args.Result.Text,
                    confidence = args.Result.Confidence,
                    semantics = semanticValue,
                    alternates = SpeechRecognizedPhrase.CreateArray(args.Result.Alternates),
                    words = SpeechWord.CreateArray(args.Result.Words)
                };
            }

            /// <summary>
            /// Gets or sets the type-specific properties of the message.
            /// </summary>
            public MessageArgs recognized { get; set; }

            /// <summary>
            /// Represents the type-specific properties of the message.
            /// </summary>
            public class MessageArgs
            {
                /// <summary>
                /// Gets or sets the name of the grammar to which the recognized text belongs.
                /// </summary>
                public string grammar { get; set; }

                /// <summary>
                /// Gets or sets the recognized text.
                /// </summary>
                public string text { get; set; }

                /// <summary>
                /// Gets or sets the confidence of the recognized text.
                /// </summary>
                public float confidence { get; set; }

                /// <summary>
                /// Gets or sets the semantics.
                /// </summary>
                public SpeechSemanticValue semantics { get; set; }

                /// <summary>
                /// Gets or sets the alternates.
                /// </summary>
                public SpeechRecognizedPhrase[] alternates { get; set; }

                /// <summary>
                /// Gets or sets the recognized words.
                /// </summary>
                public SpeechWord[] words { get; set; }
            }
        }

        /// <summary>
        /// Represents a semantic value from the speech recognition engine.
        /// </summary>
        public class SpeechSemanticValue
        {
            /// <summary>
            /// Initializes a new instance of the SemanticValue class.
            /// </summary>
            /// <param name="value">The semantic value from the recognition engine.</param>
            public SpeechSemanticValue(SemanticValue value)
            {
                this.value = value.Value != null ? value.Value.ToString() : string.Empty;
                this.confidence = value.Confidence;
                this.items = new SpeechSemanticChildren(value);
            }

            /// <summary>
            /// Gets or sets a string representation of the semantic value.
            /// </summary>
            public string value { get; set; }

            /// <summary>
            /// Gets or sets the confidence of the semantic parsing.
            /// </summary>
            public float confidence { get; set; }

            /// <summary>
            /// Gets or sets the semantic value's child objects.
            /// </summary>
            public SpeechSemanticChildren items { get; set; }

            /// <summary>
            /// Represents the child objects of a semantic value.
            /// </summary>
            public class SpeechSemanticChildren : Dictionary<string, object>
            {
                /// <summary>
                /// Initializes a new instance of the SpeechSemanticChildren class.
                /// </summary>
                /// <param name="children">The dictionary of semantic children.</param>
                public SpeechSemanticChildren(IDictionary<string, SemanticValue> children)
                {
                    foreach (var child in children)
                    {
                        if (child.Value != null && child.Value.Value is string)
                        {
                            this.Add(child.Key, child.Value.Value);
                        }
                        else
                        {
                            this.Add(child.Key, child.Value);
                        }

                    }
                }
            }
        }

        /// <summary>
        /// Represents a recognized phrase from the speech recognition engine.
        /// </summary>
        public class SpeechRecognizedPhrase
        {
            /// <summary>
            /// Initializes a new instance of the Speech RecognizedPhrase class.
            /// </summary>
            /// <param name="phrase"></param>
            public SpeechRecognizedPhrase(RecognizedPhrase phrase)
            {
                SpeechSemanticValue semanticValue = null;
                try
                {
                    semanticValue = new SpeechSemanticValue(phrase.Semantics);
                }
                catch (InvalidOperationException)
                {
                    // Failed to parse semantics, leave it empty.
                }

                this.text = phrase.Text;
                this.confidence = phrase.Confidence;
                this.grammar = phrase.Grammar.Name;
                this.semantics = semanticValue;
                this.words = SpeechWord.CreateArray(phrase.Words);
            }

            /// <summary>
            /// Allocates and initializes an array of SpeechRecognizedPhrase objects.
            /// </summary>
            /// <param name="phrases">The colleciton of phrases from the speech recognition engine.</param>
            /// <returns>A new array of SpeechRecognizedPhrase objects</returns>
            public static SpeechRecognizedPhrase[] CreateArray(ICollection<RecognizedPhrase> phrases)
            {
                var array = new SpeechRecognizedPhrase[phrases.Count];
                int i = 0;
                foreach (var phrase in phrases)
                {
                    array[i++] = new SpeechRecognizedPhrase(phrase);
                }

                return array;
            }

            /// <summary>
            /// Gets or sets the alternate text.
            /// </summary>
            public string text { get; set; }

            /// <summary>
            /// Gets or sets the confidence of the alternate text.
            /// </summary>
            public float confidence { get; set; }

            /// <summary>
            /// Gets or sets the name of the grammar to which the alternate text belongs.
            /// </summary>
            public string grammar { get; set; }

            /// <summary>
            /// Gets or sets the semantic value of the recognized phrase.
            /// </summary>
            public SpeechSemanticValue semantics { get; set; }

            /// <summary>
            /// Gets or sets the semantic value of the recognized phrase.
            /// </summary>
            public SpeechWord[] words { get; set; }
        }

        /// <summary>
        /// Represents a recognized word.
        /// </summary>
        public class SpeechWord
        {
            public static SpeechWord[] CreateArray(ICollection<RecognizedWordUnit> words)
            {
                var array = new SpeechWord[words.Count];
                int i = 0;
                foreach (var word in words)
                {
                    array[i++] = new SpeechWord()
                    {
                        lexical = word.LexicalForm,
                        text = word.Text
                    };
                }

                return array;
            }

            /// <summary>
            /// Gets or sets the text of the word.
            /// </summary>
            public string text { get; set; }

            /// <summary>
            /// Gets or sets the lexical form of the word.
            /// </summary>
            public string lexical { get; set; }
        }

        /// <summary>
        /// Factory for SpeechStreamHandler.
        /// </summary>
        private class SpeechStreamHandlerFactory : ISensorStreamHandlerFactory
        {
            /// <summary>
            /// Creates a sensor stream handler object and associates it with a context that
            /// allows it to communicate with its owner.
            /// </summary>
            /// <param name="context">
            /// An instance of <see cref="SensorStreamHandlerContext"/> class.
            /// </param>
            /// <returns>
            /// A new <see cref="ISensorStreamHandler"/> instance.
            /// </returns>
            public ISensorStreamHandler CreateHandler(SensorStreamHandlerContext context)
            {
                return new SpeechStreamHandler(context);
            }
        }
    }
}
