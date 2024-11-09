//------------------------------------------------------------------------------
// <copyright file="Rating.cs" company="Microsoft">
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
    using System.ComponentModel;
    using System.Linq.Expressions;

    public class Rating : INotifyPropertyChanged
    {
        private int dislikes;
        private int likes;

        #region INotifyPropertyChanged Members

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion

        public int Likes
        {
            get 
            { 
                return this.likes; 
            }

            set
            {
                this.likes = value;
                this.OnPropertyChanged(() => this.Likes);
            }
        }

        public int Dislikes
        {
            get 
            { 
                return this.dislikes; 
            }

            set
            {
                this.dislikes = value;
                this.OnPropertyChanged(() => this.Dislikes);
            }
        }


        private void OnPropertyChanged<T>(Expression<Func<T>> expression)
        {
            if (this.PropertyChanged == null)
            {
                return;
            }

            var body = (MemberExpression)expression.Body;
            string propertyName = body.Member.Name;
            var args = new PropertyChangedEventArgs(propertyName);
            this.PropertyChanged(this, args);
        }
    }
}
