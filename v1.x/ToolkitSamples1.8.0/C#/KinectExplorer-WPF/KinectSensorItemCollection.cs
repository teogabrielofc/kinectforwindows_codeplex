//------------------------------------------------------------------------------
// <copyright file="KinectSensorItemCollection.cs" company="Microsoft">
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

namespace Microsoft.Samples.Kinect.KinectExplorer
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using Microsoft.Kinect;

    /// <summary>
    /// An ObservableCollection of KinectSensorItems, used to track collection changes.
    /// </summary>
    public class KinectSensorItemCollection : ObservableCollection<KinectSensorItem>
    {
        private readonly Dictionary<KinectSensor, KinectSensorItem> sensorLookup = new Dictionary<KinectSensor, KinectSensorItem>();

        public Dictionary<KinectSensor, KinectSensorItem> SensorLookup
        {
            get
            {
                return this.sensorLookup;
            }
        }

        protected override void InsertItem(int index, KinectSensorItem item)
        {
            if (item == null)
            {
                throw new ArgumentException("Inserted item can't be null.", "item");
            }

            this.SensorLookup.Add(item.Sensor, item);
            base.InsertItem(index, item);
        }

        protected override void RemoveItem(int index)
        {
            this.SensorLookup.Remove(this[index].Sensor);
            base.RemoveItem(index);
        }

        protected override void ClearItems()
        {
            this.SensorLookup.Clear();
            base.ClearItems();
        }
    }
}
