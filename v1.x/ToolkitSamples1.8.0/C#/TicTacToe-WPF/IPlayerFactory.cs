// -----------------------------------------------------------------------
// <copyright file="IPlayerFactory.cs" company="Microsoft">
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
    /// A factory of players interacting with a Kinect sensor.
    /// </summary>
    /// <typeparam name="TPlayer">
    /// Type used to represent a Kinect player.
    /// </typeparam>
    public interface IPlayerFactory<TPlayer> where TPlayer : IPlayer
    {
        /// <summary>
        /// Create a new instance of a Kinect player.
        /// </summary>
        /// <returns>
        /// A new Kinect player.
        /// </returns>
        TPlayer Create();

        /// <summary>
        /// Clean resources associated with specified player.
        /// </summary>
        /// <param name="player">
        /// Player whose associated resources should be cleaned up.
        /// </param>
        void Cleanup(TPlayer player);

        /// <summary>
        /// Perform necessary initialization steps associated with specified Kinect sensor.
        /// </summary>
        /// <param name="newSensor">
        /// Sensor to initialize with required parameters.
        /// </param>
        void InitializeSensor(KinectSensor newSensor);

        /// <summary>
        /// Perform necessary uninitialization steps associated with specified Kinect sensor.
        /// </summary>
        void UninitializeSensor();
    }
}
