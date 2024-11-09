// --------------------------------------------------------------------------------------------------------------------
// <copyright file="D3D11D3DImage.cs" company="Microsoft">
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
// --------------------------------------------------------------------------------------------------------------------

namespace WpfD3DInterop
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Interop;
    using Microsoft.Windows.Media;

    public sealed class D3D11D3DImage : D3DImage, IDisposable
    {
        // Using a DependencyProperty as the backing store for HWNDOwner.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty WindowOwnerHandle =
            DependencyProperty.Register("WindowOwner", typeof(IntPtr), typeof(D3D11D3DImage), new UIPropertyMetadata(IntPtr.Zero, new PropertyChangedCallback(HWNDOwnerChanged)));
        
        // Using a DependencyProperty as the backing store for OnRender.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty OnRenderProperty =
            DependencyProperty.Register("OnRender", typeof(Action<IntPtr>), typeof(D3D11D3DImage), new UIPropertyMetadata(null, new PropertyChangedCallback(RenderChanged)));

        public Action<IntPtr> OnRender
        {
            get { return (Action<IntPtr>)GetValue(OnRenderProperty); }
            set { this.SetValue(OnRenderProperty, value); }
        }

        public IntPtr WindowOwner
        {
            get { return (IntPtr)GetValue(WindowOwnerHandle); }
            set { this.SetValue(WindowOwnerHandle, value); }
        }

        // Interop
        private SurfaceQueueInteropHelper Helper { get; set; }

        public void RequestRender()
        {
            this.EnsureHelper();

            // Don't bother with a call if there's no callback registered.
            if (null != this.OnRender)
            {
                this.Helper.RequestRenderD2D();
            }
        }

        public void SetPixelSize(int pixelWidth, int pixelHeight)
        {
            this.EnsureHelper();
            this.Helper.SetPixelSize((uint)pixelWidth, (uint)pixelHeight);
        }

        #region IDisposable Members
        public void Dispose()
        {
            if (this.Helper != null)
            {
                this.Helper.Dispose();
                this.Helper = null;
            }
        }
        #endregion

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Reliability", "CA2000:Dispose objects before losing scope", Justification = "Lifetime of the new instance belongs to caller of this method.")]
        protected override System.Windows.Freezable CreateInstanceCore()
        {
            return new D3D11D3DImage();
        }

        #region Callbacks
        private static void HWNDOwnerChanged(DependencyObject sender, DependencyPropertyChangedEventArgs args)
        {
            D3D11D3DImage image = sender as D3D11D3DImage;

            if (image != null)
            {
                if (image.Helper != null)
                {
                    image.Helper.HWND = (IntPtr)args.NewValue;
                }
            }
        }

        private static void RenderChanged(DependencyObject sender, DependencyPropertyChangedEventArgs args)
        {
            D3D11D3DImage image = sender as D3D11D3DImage;

            if (image != null)
            {
                if (image.Helper != null)
                {
                    image.Helper.RenderD2D = (Action<IntPtr>)args.NewValue;
                }
            }
        }
        #endregion Callbacks

        #region Helpers
        private void EnsureHelper()
        {
            if (this.Helper == null)
            {
                this.Helper = new SurfaceQueueInteropHelper();
                this.Helper.HWND = this.WindowOwner;
                this.Helper.D3DImage = this;
                this.Helper.RenderD2D = this.OnRender;
            }
        }
        #endregion Helpers
    }
}
