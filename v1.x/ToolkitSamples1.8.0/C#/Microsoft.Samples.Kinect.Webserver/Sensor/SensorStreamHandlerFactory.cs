// -----------------------------------------------------------------------
// <copyright file="SensorStreamHandlerFactory.cs" company="Microsoft">
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
    using Microsoft.Samples.Kinect.Webserver.Properties;

    /// <summary>
    /// The supported sensor stream type.
    /// </summary>
    public enum StreamHandlerType
    {
        Skeleton,
        Interaction,
        BackgroundRemoval,
        SensorStatus,
    }

    /// <summary>
    /// Implementation of ISensorStreamHandlerFactory used to create instances of
    /// sensor stream handler objects.
    /// </summary>
    public class SensorStreamHandlerFactory : ISensorStreamHandlerFactory
    {
        /// <summary>
        /// The type of the created stream.
        /// </summary>
        private readonly StreamHandlerType streamType;

        /// <summary>
        /// Initializes a new instance of the <see cref="SensorStreamHandlerFactory"/> class.
        /// </summary>
        /// <param name="streamType">The stream type.</param>
        public SensorStreamHandlerFactory(StreamHandlerType streamType)
        {
            this.streamType = streamType;
        }

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
            switch (streamType)
            {
                case StreamHandlerType.Skeleton:
                    return new SkeletonStreamHandler(context);
                case StreamHandlerType.Interaction:
                    return new InteractionStreamHandler(context);
                case StreamHandlerType.BackgroundRemoval:
                    return new BackgroundRemovalStreamHandler(context);
                case StreamHandlerType.SensorStatus:
                    return new SensorStatusStreamHandler(context);
                default:
                    throw new NotSupportedException(Resources.UnsupportedStreamType);
            }
        }
    }
}
