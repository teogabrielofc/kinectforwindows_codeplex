// -----------------------------------------------------------------------
// <copyright file="FileRequestHandlerFactory.cs" company="Microsoft">
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

namespace Microsoft.Samples.Kinect.Webserver
{
    using System;
    using System.Globalization;
    using System.IO;

    /// <summary>
    /// Implementation of IHttpRequestHandlerFactory used to create instances of
    /// <see cref="FileRequestHandler"/> objects.
    /// </summary>
    public class FileRequestHandlerFactory : IHttpRequestHandlerFactory
    {
        /// <summary>
        /// Root directory in server's file system from which we're serving files.
        /// </summary>
        private readonly DirectoryInfo rootDirectory;

        /// <summary>
        /// Initializes a new instance of the <see cref="FileRequestHandlerFactory"/> class.
        /// </summary>
        /// <param name="rootDirectoryName">
        /// Root directory name in server's file system from which files should be served.
        /// The directory must exist at the time of the call.
        /// </param>
        internal FileRequestHandlerFactory(string rootDirectoryName)
        {
            if (!Directory.Exists(rootDirectoryName))
            {
                throw new ArgumentException(string.Format(CultureInfo.InvariantCulture, @"The specified directory '{0}' does not exist", rootDirectoryName), "rootDirectoryName");
            }

            this.rootDirectory = new DirectoryInfo(rootDirectoryName);
        }

        public IHttpRequestHandler CreateHandler()
        {
            return new FileRequestHandler(this.rootDirectory);
        }
    }
}
