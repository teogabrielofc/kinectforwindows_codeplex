//------------------------------------------------------------------------------
// <copyright file="HandInputEventArgs.cs" company="Microsoft">
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
    using System.Windows;

    public class HandInputEventArgs : RoutedEventArgs
    {
        public HandInputEventArgs()
        {
        }

        public HandInputEventArgs(RoutedEvent routedEvent) : base(routedEvent)
        {
        }

        public HandInputEventArgs(RoutedEvent routedEvent, object source) : base(routedEvent, source)
        {
        }

        public HandInputEventArgs(RoutedEvent routedEvent, object source, HandPosition hand)
            : base(routedEvent, source)
        {
            this.Hand = hand;
        }

        public HandPosition Hand { get; set; }
    }
}
