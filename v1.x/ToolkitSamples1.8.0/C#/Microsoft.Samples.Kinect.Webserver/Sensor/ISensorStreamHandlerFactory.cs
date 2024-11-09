// -----------------------------------------------------------------------
// <copyright file="ISensorStreamHandlerFactory.cs" company="Microsoft">
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
    public interface ISensorStreamHandlerFactory
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
        ISensorStreamHandler CreateHandler(SensorStreamHandlerContext context);
    }
}
