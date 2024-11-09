// -----------------------------------------------------------------------
// <copyright file="ISkeletonFilter.cs" company="Microsoft">
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

namespace Microsoft.Samples.Kinect.TicTacToe
{
    using System.Collections.Generic;
    using Microsoft.Kinect;

    /// <summary>
    /// Used to filter a set of skeletons into a subset of interest.
    /// </summary>
    public interface ISkeletonFilter
    {
        /// <summary>
        /// Filters the specified enumerable set of skeletons to obtain a smaller subset of interest.
        /// </summary>
        /// <param name="skeletons">
        /// Enumerable set of skeletons to be filtered.
        /// </param>
        /// <returns>
        /// Enumerable set of skeletons output by filtering operation.
        /// </returns>
        IEnumerable<Skeleton> Filter(IEnumerable<Skeleton> skeletons);
    }
}
