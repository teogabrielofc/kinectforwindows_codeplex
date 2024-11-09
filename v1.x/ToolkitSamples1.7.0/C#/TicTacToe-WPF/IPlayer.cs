// -----------------------------------------------------------------------
// <copyright file="IPlayer.cs" company="Microsoft">
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
    using Microsoft.Kinect;

    /// <summary>
    /// An abstraction representing a player interacting with Kinect sensor.
    /// </summary>
    public interface IPlayer
    {
        /// <summary>
        /// Last seen skeleton data for this player.
        /// </summary>
        Skeleton Skeleton { get; }

        /// <summary>
        /// Update player with data from Kinect sensor.
        /// </summary>
        /// <param name="skeleton">
        /// Skeleton data corresponding to player.
        /// </param>
        /// <param name="eventArgs">
        /// Event arguments corresponding to specified skeleton.
        /// </param>
        void Update(Skeleton skeleton, AllFramesReadyEventArgs eventArgs);
    }
}
