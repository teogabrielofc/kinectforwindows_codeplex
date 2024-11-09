//------------------------------------------------------------------------------
// <copyright file="App.xaml.cs" company="Microsoft">
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

namespace Microsoft.Samples.Kinect.BasicInteractions
{
    using System.ComponentModel;
    using System.Windows;
    using Microsoft.Samples.Kinect.BasicInteractions.Properties;

    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private static KinectController controller;

        public App()
        {
            Model = new Model();
        }

        public static Model Model { get; private set; }

        public static KinectController Controller
        {
            get
            {
                if (controller == null)
                {
                    if (DesignerProperties.GetIsInDesignMode(Current.MainWindow) == false)
                    {
                        controller = new KinectController(Current.MainWindow);
                        controller.Initialize();
                        controller.SetSpeechGrammar(Model.CreateSpeechGrammar());
                        controller.MinimumSpeechConfidence = Settings.Default.SpeechMinimumConfidence;
                    }
                }

                return controller;
            }
        }
    }
}
