//------------------------------------------------------------------------------
// <copyright file="HandHoverTimer.cs" company="Microsoft">
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

namespace Microsoft.Samples.Kinect.BasicInteractions
{
    using System;
    using System.Windows.Threading;

    public class HandHoverTimer
    {
        private readonly DispatcherTimer timer;
        private DateTime startTime;
        private bool startTimeValid;

        public HandHoverTimer(DispatcherPriority priority, Dispatcher dispatcher)
        {
            this.timer = new DispatcherTimer(priority, dispatcher);
        }

        public event EventHandler Tick
        {
            add { this.timer.Tick += value; }
            remove { this.timer.Tick -= value; }
        }

        public HandPosition Hand { get; set; }

        public TimeSpan Interval
        {
            get { return this.timer.Interval; }
            set { this.timer.Interval = value; }
        }

        public TimeSpan TimeRemaining
        {
            get { return this.startTimeValid ? this.Interval - (DateTime.Now - this.startTime) : TimeSpan.MaxValue; }
        }

        public void Start()
        {
            this.startTime = DateTime.Now;
            this.startTimeValid = true;
            this.timer.Start();
        }

        public void Stop()
        {
            this.startTimeValid = false;
            this.timer.Stop();
        }
    }
}
