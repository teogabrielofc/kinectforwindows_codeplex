// -----------------------------------------------------------------------
// <copyright file="KinectRequestHandlerFactory.cs" company="Microsoft">
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
    using System.Collections.ObjectModel;

    using Microsoft.Kinect.Toolkit;
    using Microsoft.Samples.Kinect.Webserver;

    /// <summary>
    /// Implementation of IHttpRequestHandlerFactory used to create instances of
    /// <see cref="KinectRequestHandler"/> objects.
    /// </summary>
    public class KinectRequestHandlerFactory : IHttpRequestHandlerFactory
    {
        /// <summary>
        /// Sensor chooser used to obtain a KinectSensor.
        /// </summary>
        private readonly KinectSensorChooser sensorChooser;

        /// <summary>
        /// Collection of sensor stream handler factories used to process kinect data and deliver
        /// a data streams ready for web consumption.
        /// </summary>
        private readonly Collection<ISensorStreamHandlerFactory> streamHandlerFactories;

        /// <summary>
        /// Initializes a new instance of the KinectRequestHandlerFactory class.
        /// </summary>
        /// <param name="sensorChooser">
        /// Sensor chooser that will be used to obtain a KinectSensor.
        /// </param>
        /// <remarks>
        /// Default set of sensor stream handler factories will be used.
        /// </remarks>
        public KinectRequestHandlerFactory(KinectSensorChooser sensorChooser)
        {
            this.sensorChooser = sensorChooser;
            this.streamHandlerFactories = CreateDefaultStreamHandlerFactories();
        }

        /// <summary>
        /// Initializes a new instance of the KinectRequestHandlerFactory class.
        /// </summary>
        /// <param name="sensorChooser">
        /// Sensor chooser that will be used to obtain a KinectSensor.
        /// </param>
        /// <param name="streamHandlerFactories">
        /// Collection of stream handler factories to be used to process kinect data and deliver
        /// data streams ready for web consumption.
        /// </param>
        public KinectRequestHandlerFactory(KinectSensorChooser sensorChooser, Collection<ISensorStreamHandlerFactory> streamHandlerFactories)
        {
            this.sensorChooser = sensorChooser;
            this.streamHandlerFactories = streamHandlerFactories;
        }

        /// <summary>
        /// Create collection of default stream handler factories.
        /// </summary>
        /// <returns>
        /// Collection containing default stream handler factories.
        /// </returns>
        public static Collection<ISensorStreamHandlerFactory> CreateDefaultStreamHandlerFactories()
        {
            var streamHandlerTypes = new[]
            {
                StreamHandlerType.Interaction,
                StreamHandlerType.Skeleton,
                StreamHandlerType.BackgroundRemoval,
                StreamHandlerType.SensorStatus
            };

            var factoryCollection = new Collection<ISensorStreamHandlerFactory>();
            foreach (var type in streamHandlerTypes)
            {
                factoryCollection.Add(new SensorStreamHandlerFactory(type));
            }

            return factoryCollection;
        }

        /// <summary>
        /// Creates a request handler object.
        /// </summary>
        /// <returns>
        /// A new <see cref="IHttpRequestHandler"/> instance.
        /// </returns>
        public IHttpRequestHandler CreateHandler()
        {
            return new KinectRequestHandler(this.sensorChooser, this.streamHandlerFactories);
        }
    }
}
