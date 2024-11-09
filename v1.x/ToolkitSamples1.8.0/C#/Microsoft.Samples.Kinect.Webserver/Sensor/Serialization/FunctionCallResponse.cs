// -----------------------------------------------------------------------
// <copyright file="FunctionCallResponse.cs" company="Microsoft">
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

namespace Microsoft.Samples.Kinect.Webserver.Sensor.Serialization
{
    using System.Diagnostics.CodeAnalysis;

    /// <summary>
    /// Serializable representation of a function call response.
    /// </summary>
    /// <typeparam name="T">
    /// Type of function call result.
    /// </typeparam>
    [SuppressMessage("StyleCop.CSharp.NamingRules", "SA1300:ElementMustBeginWithUpperCaseLetter", Justification = "Lower case names allowed for JSON serialization.")]
    internal class FunctionCallResponse<T>
    {
        /// <summary>
        /// Sequence Id used to match function call request with its response.
        /// </summary>
        public int id { get; set; }

        /// <summary>
        /// Result of remote function call.
        /// </summary>
        public T result { get; set; }
    }
}
