// -----------------------------------------------------------------------
// <copyright file="PlayerFactory.cs" company="Microsoft">
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
    using System;
    using System.Windows;

    using Microsoft.Kinect;

    /// <summary>
    /// Factory in charge of creating and updating Kinect players as the
    /// PlayerTracker sees the need to.
    /// </summary>
    public class PlayerFactory : IPlayerFactory<Player>
    {
        /// <summary>
        /// Create a new instance of a Kinect player.
        /// </summary>
        /// <returns>
        /// A new Kinect player.
        /// </returns>
        public Player Create()
        {
            return new Player();
        }

        /// <summary>
        /// Clean resources associated with specified player.
        /// </summary>
        /// <param name="player">
        /// Player whose associated resources should be cleaned up.
        /// </param>
        public void Cleanup(Player player)
        {
        }

        /// <summary>
        /// Perform necessary initialization steps associated with specified Kinect sensor.
        /// </summary>
        /// <param name="newSensor">
        /// Sensor to initialize with required parameters.
        /// </param>
        public void InitializeSensor(KinectSensor newSensor)
        {
            this.UninitializeSensor();

            if (null != newSensor)
            {
                // Ensure depth stream is enabled to be able to use image frame mapping functionality
                newSensor.DepthStream.Enable();

                // Ensure color stream is enabled to be able to get color format for mapping
                newSensor.ColorStream.Enable();
            }
        }

        /// <summary>
        /// Perform necessary uninitialization steps associated with specified Kinect sensor.
        /// </summary>
        public void UninitializeSensor()
        {
        }
    }
}
