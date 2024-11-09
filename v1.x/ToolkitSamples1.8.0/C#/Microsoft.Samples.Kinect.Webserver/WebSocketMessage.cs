//------------------------------------------------------------------------------
// <copyright file="WebSocketMessage.cs" company="Microsoft">
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

namespace Microsoft.Samples.Kinect.Webserver
{
    using System;
    using System.Net.WebSockets;

    /// <summary>
    /// Represents a web socket message with associated type (UTF8 versus binary).
    /// </summary>
    public class WebSocketMessage
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="WebSocketMessage"/> class.
        /// </summary>
        /// <param name="messageContent">
        /// Message content.
        /// </param>
        /// <param name="messageType">
        /// Type of message (UTF8 versus binary).
        /// </param>
        public WebSocketMessage(ArraySegment<byte> messageContent, WebSocketMessageType messageType)
        {
            this.Content = messageContent;
            this.MessageType = messageType;
        }

        /// <summary>
        /// Message content.
        /// </summary>
        public ArraySegment<byte> Content { get; private set; }

        /// <summary>
        /// Message type (UTF8 text versus binary).
        /// </summary>
        public WebSocketMessageType MessageType { get; private set; }
    }
}
