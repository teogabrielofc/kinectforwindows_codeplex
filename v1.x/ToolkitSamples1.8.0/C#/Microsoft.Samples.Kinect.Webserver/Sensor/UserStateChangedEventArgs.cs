// -----------------------------------------------------------------------
// <copyright file="UserStateChangedEventArgs.cs" company="Microsoft">
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
// -----------------------------------------------------------------------

namespace Microsoft.Samples.Kinect.Webserver.Sensor
{
    using System;

    using Microsoft.Samples.Kinect.Webserver.Sensor.Serialization;

    /// <summary>
    /// Event arguments for IUserStateManager.UserStateChanged event.
    /// </summary>
    public class UserStateChangedEventArgs : EventArgs
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="UserStateChangedEventArgs"/> class.
        /// </summary>
        /// <param name="message">
        /// Representation of event as a web message to be sent.
        /// </param>
        public UserStateChangedEventArgs(EventMessage message)
        {
            this.Message = message;
        }

        /// <summary>
        /// Representation of event as a web message to be sent.
        /// </summary>
        public EventMessage Message { get; private set; }
    }
}
