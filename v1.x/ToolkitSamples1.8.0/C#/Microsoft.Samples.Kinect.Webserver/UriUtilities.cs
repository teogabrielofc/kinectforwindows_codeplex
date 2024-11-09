// -----------------------------------------------------------------------
// <copyright file="UriUtilities.cs" company="Microsoft">
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
    using System.Diagnostics;

    /// <summary>
    /// Static class that defines uri manipulation utilities.
    /// </summary>
    public static class UriUtilities
    {
        /// <summary>
        /// Separator between URI path segments.
        /// </summary>
        public const string PathSeparator = "/";

        /// <summary>
        /// Concatenate specified path segments at the end of specified URI.
        /// </summary>
        /// <param name="uri">
        /// Absolute URI to serve as starting point of concatenation.
        /// </param>
        /// <param name="pathSegments">
        /// Path segments to concatenate at the end of URI.
        /// </param>
        /// <returns>
        /// URI that represents the combination of the specified uri and path segments.
        /// May be null if uri segments could not be concatenated.
        /// </returns>
        public static Uri ConcatenateSegments(this Uri uri, params string[] pathSegments)
        {
            Uri result = uri;

            if (uri == null)
            {
                throw new ArgumentNullException("uri");
            }

            if (pathSegments == null)
            {
                throw new ArgumentNullException("pathSegments");
            }

            for (int i = 0; i < pathSegments.Length; ++i)
            {
                var segment = pathSegments[i];

                if (segment == null)
                {
                    throw new ArgumentException(@"One or more of the specified path segments is null", "pathSegments");
                }

                if (i < pathSegments.Length - 1)
                {
                    // For each element other than the last element, make sure it ends in the
                    // path separator character so that it's treated as a path segment rather
                    // than an endpoint or resource (see CoInternetCombineIUri documentation
                    // for an explanation of standard URI combination behavior)
                    segment = segment.EndsWith(PathSeparator, StringComparison.OrdinalIgnoreCase) ? segment : (segment + PathSeparator);
                }

                // Now call the standard URI class to take care of canonicalization and other
                // combination functionality
                var previous = result;
                try
                {
                    result = new Uri(previous, new Uri(segment.Trim(), UriKind.Relative));
                }
                catch (UriFormatException)
                {
                    Trace.TraceError("Unable to concatenate uri \"{0}\" with path segment \"{1}\"", previous, segment);
                    result = null;
                    break;
                }
            }

            return result;
        }
    }
}
