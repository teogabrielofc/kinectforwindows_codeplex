//------------------------------------------------------------------------------
// <copyright file="RpcResult.cs" company="Microsoft">
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
    /// <summary>
    /// Represents a remote procedure call result.
    /// </summary>
    /// <typeparam name="T">
    /// Type of call result.
    /// </typeparam>
    public class RpcResult<T>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="RpcResult{T}"/> class.
        /// </summary>
        /// <param name="success">
        /// True if call was successful. False otherwise.
        /// </param>
        /// <param name="result">
        /// Result of remote procedure call.
        /// </param>
        public RpcResult(bool success, T result)
        {
            this.Success = success;
            this.Result = result;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="RpcResult{T}"/> class with
        /// the default value of type T as the result.
        /// </summary>
        /// <param name="success">
        /// True if call was successful. False otherwise.
        /// </param>
        public RpcResult(bool success) : this(success, default(T))
        {
        }

        /// <summary>
        /// True if call was successful. False otherwise.
        /// </summary>
        public bool Success { get; private set; }

        /// <summary>
        /// Result of remote procedure call.
        /// </summary>
        public T Result { get; private set; }
    }
}
