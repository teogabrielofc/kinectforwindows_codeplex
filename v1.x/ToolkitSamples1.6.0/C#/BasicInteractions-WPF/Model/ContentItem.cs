//------------------------------------------------------------------------------
// <copyright file="ContentItem.cs" company="Microsoft">
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
    using System.Windows.Media;

    public class ContentItem
    {
        public ContentItem()
        {
            this.Rating = new Rating();
        }

        public string Title { get; set; }

        public Category Category { get; set; }

        public string Subcategory { get; set; }

        public string Content { get; set; }

        public ImageSource ContentImage { get; set; }

        public Uri ContentVideo { get; set; }

        public Rating Rating { get; set; }

        public int ItemId { get; set; }
    }
}
