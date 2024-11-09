//------------------------------------------------------------------------------
// <copyright file="Utility.cs" company="Microsoft">
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
    using System.Windows;
    using System.Windows.Media;

    public static class Utility
    {
        public static T FindParent<T>(object child)
            where T : DependencyObject
        {
            var search = child as DependencyObject;
            T parent = null;
            while (search != null && (parent = search as T) == null)
            {
                search = VisualTreeHelper.GetParent(search);
            }

            return parent;
        }

        public static bool IsElementChild(DependencyObject parentElement, DependencyObject childElement)
        {
            DependencyObject search = childElement;
            while (search != null && search != parentElement)
            {
                search = VisualTreeHelper.GetParent(search);
            }

            return search != null;
        }
    }
}
