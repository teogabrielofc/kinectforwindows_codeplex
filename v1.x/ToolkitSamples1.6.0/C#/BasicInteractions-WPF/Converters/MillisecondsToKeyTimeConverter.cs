//------------------------------------------------------------------------------
// <copyright file="MillisecondsToKeyTimeConverter.cs" company="Microsoft">
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
    using System.Globalization;
    using System.Windows.Data;
    using System.Windows.Media.Animation;

    public class MillisecondsToKeyTimeConverter : IValueConverter
    {
        #region IValueConverter Members

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            KeyTime keyTime = KeyTime.FromTimeSpan(TimeSpan.FromSeconds(0));
            if (value is double)
            {
                keyTime = KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds((double)value));
                if (parameter is double)
                {
                    keyTime = KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds((double)value + (double)parameter));
                }
            }

            return keyTime;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion
    }
}
