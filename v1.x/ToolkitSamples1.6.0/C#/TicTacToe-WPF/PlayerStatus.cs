// -----------------------------------------------------------------------
// <copyright file="PlayerStatus.cs" company="Microsoft">
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
    /// <summary>
    /// Represents the status of a Kinect player.
    /// </summary>
    public enum PlayerStatus
    {
        /// <summary>
        /// A new player has started interacting with Kinect sensor.
        /// </summary>
        Joined,

        /// <summary>
        /// A player has stopped interacting with Kinect sensor.
        /// </summary>
        Left,

        /// <summary>
        /// Data for a current player has been updated.
        /// </summary>
        Updated
    }
}
