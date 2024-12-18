// -----------------------------------------------------------------------
// <copyright file="RecentEventBufferTraceListener.cs" company="Microsoft">
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

namespace Microsoft.Samples.Kinect.WebserverBasics
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;

    /// <summary>
    /// Remembers the most recent trace events observed and exposes them as
    /// a chunk of displayable text.
    /// </summary>
    public class RecentEventBufferTraceListener : TraceListener
    {
        private const int DefaultMaximumLines = 10;

        private readonly StringBuilder traceLineBuilder = new StringBuilder();

        private readonly Queue<string> entries;

        private readonly int maximumLines;

        /// <summary>
        /// Initializes a new instance of the <see cref="RecentEventBufferTraceListener"/> class.
        /// </summary>
        /// <remarks>
        /// Remembers the default number of trace lines.
        /// </remarks>
        public RecentEventBufferTraceListener()
            : this(DefaultMaximumLines)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="RecentEventBufferTraceListener"/> class.
        /// </summary>
        /// <param name="maximumLines">
        /// Maximum number of trace lines to remember.
        /// </param>
        public RecentEventBufferTraceListener(int maximumLines)
        {
            this.maximumLines = maximumLines;
            this.entries = new Queue<string>(maximumLines);
        }

        public event EventHandler<EventArgs> RecentEventBufferChanged;

        /// <summary>
        /// Displayable string representing buffer of recent trace events seen.
        /// </summary>
        public string RecentEventBuffer { get; private set; }

        /// <summary>
        /// Remembers the specified message as the latest message seen.
        /// </summary>
        /// <param name="message">
        /// Message to remember.
        /// </param>
        public override void Write(string message)
        {
            this.traceLineBuilder.Append(message);
        }

        /// <summary>
        /// Remembers the specified message, followed by a line terminator, as the latest
        /// message seen.
        /// </summary>
        /// <param name="message">
        /// Message to remember.
        /// </param>
        public override void WriteLine(string message)
        {
            this.traceLineBuilder.Append(message);
            this.traceLineBuilder.Append("\n");
            this.QueueMessage(this.traceLineBuilder.ToString());
            this.traceLineBuilder.Clear();
        }

        /// <summary>
        /// Remembers the specified message as the latest message seen.
        /// </summary>
        /// <param name="message">
        /// Message to remember.
        /// </param>
        private void QueueMessage(string message)
        {
            this.entries.Enqueue(message);
            if (this.entries.Count > this.maximumLines)
            {
                this.entries.Dequeue();
            }

            var builder = new StringBuilder();
            foreach (var entry in this.entries)
            {
                builder.Append(entry);
            }

            this.RecentEventBuffer = builder.ToString();

            if (this.RecentEventBufferChanged != null)
            {
                this.RecentEventBufferChanged(this, EventArgs.Empty);
            }
        }
    }
}
