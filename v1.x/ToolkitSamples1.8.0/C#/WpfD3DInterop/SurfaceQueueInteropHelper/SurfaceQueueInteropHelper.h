//------------------------------------------------------------------------------
// <copyright file="SurfaceQueueInteropHelper.h" company="Microsoft">
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


#pragma once

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Interop;

#define WIDTH 640
#define HEIGHT 480

REFIID                  surfaceIDDXGI = __uuidof(IDXGISurface);
REFIID                  surfaceID9 = __uuidof(IDirect3DTexture9);

namespace Microsoft { namespace Windows { namespace Media {

    public ref class SurfaceQueueInteropHelper: IDisposable
    {
        Action<IntPtr>^ m_renderD2D;
        D3DImage^ m_d3dImage;
        DependencyPropertyChangedEventHandler^ m_frontBufferAvailableChanged;
        UINT m_pixelWidth, m_pixelHeight;
        HWND m_hwnd;

        IDirect3D9Ex*           m_pD3D9;
        IDirect3DDevice9Ex*     m_pD3D9Device;

        ID3D10Device1*          m_D3D10Device;

        ISurfaceQueue*			m_ABQueue;
        ISurfaceQueue*			m_BAQueue;

        ISurfaceConsumer*		m_ABConsumer;
        ISurfaceProducer*		m_BAProducer;
        ISurfaceConsumer*		m_BAConsumer;
        ISurfaceProducer*		m_ABProducer;

        bool m_isD3DInitialized;
        bool m_areSurfacesInitialized;
        bool m_shouldSkipRender;

        // Could hypothetically add additional types
        enum struct QueueRenderMode
        {
            None = 0,
            RenderDXGI = 1
        };

        HRESULT InitD3D10()
        {
            HRESULT hr;
            UINT		DeviceFlags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
            //DWORD		dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#ifdef _DEBUG
            // To debug DirectX, uncomment the following lines:

            //DeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
            //dwShaderFlags	|= D3D10_SHADER_DEBUG;
#endif

            {
                pin_ptr<ID3D10Device1*> pinD3D10Device = &m_D3D10Device;
                ID3D10Device1** ppD3D10Device = pinD3D10Device;

                if (FAILED(hr = D3D10CreateDevice1(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL,
                    DeviceFlags, D3D10_FEATURE_LEVEL_10_0, D3D10_1_SDK_VERSION, ppD3D10Device)))
                {
                    return hr;
                }
            }

            D3D10_VIEWPORT vp;
            vp.Width = WIDTH;
            vp.Height = HEIGHT;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = 0;
            vp.TopLeftY = 0;
            m_D3D10Device->RSSetViewports( 1, &vp );

            return S_OK;
        }

        void RenderToDXGI(IntPtr pdxgiSurface)
        {
            if (nullptr != m_renderD2D)
            {
                m_renderD2D(pdxgiSurface);
            }
        }

        void CleanupD3D10()
        {
            ReleaseInterface(m_D3D10Device);
        }

        HRESULT InitD3D9()
        {
            HRESULT hr;

            {
                pin_ptr<IDirect3D9Ex*> pinD3D9 = &m_pD3D9;
                IDirect3D9Ex** ppD3D9 = pinD3D9;

                Direct3DCreate9Ex( D3D_SDK_VERSION, ppD3D9);
            }

            if (!m_pD3D9)
            {
                return E_FAIL;
            }

            D3DPRESENT_PARAMETERS		d3dpp;
            ZeroMemory( &d3dpp, sizeof(d3dpp) );
            d3dpp.Windowed				= TRUE;
            d3dpp.SwapEffect			= D3DSWAPEFFECT_DISCARD;
            d3dpp.hDeviceWindow			= NULL;
            d3dpp.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;

            {
                pin_ptr<IDirect3DDevice9Ex*> pinD3D9Device = &m_pD3D9Device;
                IDirect3DDevice9Ex** ppD3D9Device = pinD3D9Device;

                hr = m_pD3D9->CreateDeviceEx(
                    D3DADAPTER_DEFAULT,
                    D3DDEVTYPE_HAL,
                    m_hwnd,
                    D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                    &d3dpp,
                    NULL,
                    ppD3D9Device);
            }

            return hr;
        }

        void CleanupD3D9()
        {
            ReleaseInterface(m_pD3D9Device);
            ReleaseInterface(m_pD3D9);
        }

        void CleanupSurfaces()
        {
            m_areSurfacesInitialized = false;

            ReleaseInterface(m_BAProducer);
            ReleaseInterface(m_ABProducer);
            ReleaseInterface(m_BAConsumer);
            ReleaseInterface(m_ABConsumer);

            ReleaseInterface(m_ABQueue);
            ReleaseInterface(m_BAQueue);
        }

        void CleanupD3D()
        {
            if (m_areSurfacesInitialized)
            {
                CleanupSurfaces();
            }

            m_isD3DInitialized = false;

            CleanupD3D10();
            CleanupD3D9();
        }

        HRESULT InitD3D()
        {
            HRESULT hr = S_OK;

            if (!m_isD3DInitialized)
            {
                IFC(InitD3D9());
                IFC(InitD3D10());

                m_isD3DInitialized = true;
            }

Cleanup:
            if (FAILED(hr))
            {
                CleanupD3D();
            }

            return hr;
        }

        HRESULT InitSurfaces()
        {
            HRESULT hr = S_OK;

            SURFACE_QUEUE_DESC  desc;
            ZeroMemory( &desc, sizeof(desc) );
            desc.Width                  =  m_pixelWidth; 
            desc.Height                 =  m_pixelHeight;
            desc.Format                 = DXGI_FORMAT_B8G8R8A8_UNORM;
            desc.NumSurfaces            = 1;
            desc.MetaDataSize           = sizeof(int);
            desc.Flags                  = SURFACE_QUEUE_FLAG_SINGLE_THREADED;

            SURFACE_QUEUE_CLONE_DESC CloneDesc = {0};
            CloneDesc.MetaDataSize      = 0;
            CloneDesc.Flags             = SURFACE_QUEUE_FLAG_SINGLE_THREADED;

            if (!m_isD3DInitialized || (desc.Width <= 0) || (desc.Height <= 0))
            {
                hr = S_FALSE;
                goto Cleanup;
            }

            if (!m_areSurfacesInitialized)
            {
                // 
                // Initialize the surface queues
                //

                {
                    pin_ptr<ISurfaceQueue*> pinABQueue = &m_ABQueue;
                    ISurfaceQueue** ppABQueue = pinABQueue;

                    IFC(CreateSurfaceQueue(&desc, m_pD3D9Device, ppABQueue));
                }

                // Clone the queue           
                {
                    pin_ptr<ISurfaceQueue*> pinBAQueue = &m_BAQueue;
                    ISurfaceQueue** ppBAQueue = pinBAQueue;

                    IFC(m_ABQueue->Clone(&CloneDesc, ppBAQueue));
                }

                // Setup queue management
                {
                    pin_ptr<ISurfaceProducer*> pinm_BAProducer = &m_BAProducer;
                    ISurfaceProducer** ppm_BAProducer = pinm_BAProducer;

                    IFC(m_BAQueue->OpenProducer(m_D3D10Device, ppm_BAProducer));
                }

                {
                    pin_ptr<ISurfaceConsumer*> pinm_ABConsumer = &m_ABConsumer;
                    ISurfaceConsumer** ppm_ABConsumer = pinm_ABConsumer;

                    IFC(m_ABQueue->OpenConsumer(m_D3D10Device, ppm_ABConsumer));
                }

                {
                    pin_ptr<ISurfaceProducer*> pinm_ABProducer = &m_ABProducer;
                    ISurfaceProducer** ppm_ABProducer = pinm_ABProducer;

                    IFC(m_ABQueue->OpenProducer(m_pD3D9Device, ppm_ABProducer));
                }

                {
                    pin_ptr<ISurfaceConsumer*> pinm_BAConsumer = &m_BAConsumer;
                    ISurfaceConsumer** ppm_BAConsumer = pinm_BAConsumer;

                    IFC(m_BAQueue->OpenConsumer(m_pD3D9Device, ppm_BAConsumer));
                }

                m_areSurfacesInitialized = true;
            }

Cleanup:

            return hr;
        }

        // Returns true if this instance is now initialized.
        bool Initialize()
        {
            HRESULT hr = S_OK;

            if (m_isD3DInitialized)
            {
                hr = m_pD3D9Device->CheckDeviceState(NULL);

                if (D3D_OK != hr)
                {
                    CleanupD3D();
                }
            }

            if (!m_isD3DInitialized)
            {
                IFC(InitD3D());
            }

            if (!m_areSurfacesInitialized)
            {
                // Can be S_FALSE if there's nothing to do.
                IFC(InitSurfaces());
            }

Cleanup:

            // Clean up, but don't throw, as this can be a transient failure.
            // TODO: Consider if/how to differentiate between fatal failure and transient failure.
            if (FAILED(hr))
            {                
                CleanupD3D();
            }

            return m_areSurfacesInitialized;
        }

        // If fShouldRenderD3D10 is true, this method performs the callout to RenderD3D10.
        // In any case, this method always initializes m_d3dImage which incurrs no cost if this results in no change.
        void QueueHelper(QueueRenderMode renderMode)
        {
            HRESULT hr = S_OK;

            IDXGISurface*           pDXGISurface = NULL;
            IUnknown*               pUnkDXGISurface = NULL;

            IDirect3DTexture9*      pTexture9 = NULL;
            IUnknown*               pUnkTexture9 = NULL;

            IDirect3DSurface9*      pSurface9 = NULL;

            DXGI_SURFACE_DESC desc;

            // D3D10 portion
            int count = 0;
            UINT size = sizeof(int);

            bool fNeedUnlock = false;

            if (m_shouldSkipRender || (nullptr == m_d3dImage) || !Initialize())
            {
                goto Cleanup;
            }

            m_d3dImage->Lock();
            fNeedUnlock = true;

            // Flush the AB queue
            m_ABProducer->Flush(0 /* wait */, NULL);

            // Dequeue from AB queue
            IFC(m_ABConsumer->Dequeue(surfaceIDDXGI, &pUnkDXGISurface, &count, &size, INFINITE));

            IFC(pUnkDXGISurface->QueryInterface(surfaceIDDXGI, (void**)&pDXGISurface));

            IFC(pDXGISurface->GetDesc(&desc));

            if (renderMode == QueueRenderMode::RenderDXGI)
            {
                // Render D3D10 content
                try 
                {
                    RenderToDXGI((IntPtr)(void*)pDXGISurface);
                }
                catch (Exception^)
                {
                    IFC(E_FAIL);
                }
            }

            // Produce the surface
            m_BAProducer->Enqueue(pDXGISurface, NULL, NULL, SURFACE_QUEUE_FLAG_DO_NOT_WAIT);

            // Flush the BA queue
            m_BAProducer->Flush(0 /* wait, *not* SURFACE_QUEUE_FLAG_DO_NOT_WAIT*/, NULL);

            // Dequeue from BA queue
            IFC(m_BAConsumer->Dequeue(surfaceID9, &pUnkTexture9, NULL, NULL, INFINITE));
            IFC(pUnkTexture9->QueryInterface(surfaceID9, (void**)&pTexture9));

            // Get the top level surface from the texture
            IFC(pTexture9->GetSurfaceLevel(0, &pSurface9));

            m_d3dImage->SetBackBuffer(System::Windows::Interop::D3DResourceType::IDirect3DSurface9, (IntPtr)(void*)pSurface9);

            // Produce Surface
            m_ABProducer->Enqueue(pTexture9, &count, sizeof(int), SURFACE_QUEUE_FLAG_DO_NOT_WAIT);

            // Flush the AB queue - use "do not wait" here, we'll block at the top of the *next* call if we need to
            m_ABProducer->Flush(SURFACE_QUEUE_FLAG_DO_NOT_WAIT, NULL);

Cleanup:
            if (fNeedUnlock)
            {
                m_d3dImage->AddDirtyRect(Int32Rect(0, 0, m_d3dImage->PixelWidth, m_d3dImage->PixelHeight));
                m_d3dImage->Unlock();
            }

            ReleaseInterface(pSurface9);

            ReleaseInterface(pTexture9);
            ReleaseInterface(pUnkTexture9);

            ReleaseInterface(pDXGISurface);
            ReleaseInterface(pUnkDXGISurface);
        }


        void D3DImageFrontBufferAvailableChangedHandler (Object^ sender, DependencyPropertyChangedEventArgs e)
        {
            if (!((bool)e.NewValue))
            {                
                //CleanupSurfaces();
                CleanupD3D();
                m_shouldSkipRender = true;
            }
            else
            {
                m_shouldSkipRender = false;
                QueueHelper(QueueRenderMode::RenderDXGI);
            }
        }

    public:

        property Action<IntPtr>^ RenderD2D
        {
            void set(Action<IntPtr>^ value) { m_renderD2D = value; }
        }

        property D3DImage^ D3DImage
        {
            System::Windows::Interop::D3DImage^ get() 
            { 
                return m_d3dImage; 
            }

            void set(System::Windows::Interop::D3DImage^ d3dImage) 
            { 
                if (d3dImage != m_d3dImage)
                {
                    if (nullptr != m_d3dImage)
                    {
                        m_d3dImage->IsFrontBufferAvailableChanged -= m_frontBufferAvailableChanged;
                        m_d3dImage->SetBackBuffer(System::Windows::Interop::D3DResourceType::IDirect3DSurface9, (IntPtr)nullptr);
                    }

                    m_d3dImage = d3dImage;

                    if (nullptr != m_d3dImage)
                    {
                        if (nullptr == m_frontBufferAvailableChanged)
                        {
                            m_frontBufferAvailableChanged = gcnew DependencyPropertyChangedEventHandler(this, &SurfaceQueueInteropHelper::D3DImageFrontBufferAvailableChangedHandler);
                        }

                        m_d3dImage->IsFrontBufferAvailableChanged += m_frontBufferAvailableChanged;
                    }

                    // TODO: Force a rerender...?
                }
            }
        }

        property unsigned int PixelWidth
        {
            unsigned int get() 
            { 
                return m_pixelWidth; 
            }
        }

        property unsigned int PixelHeight
        {
            unsigned int get() 
            { 
                return m_pixelHeight; 
            }
        }

        void SetPixelSize(unsigned int pixelWidth, unsigned int pixelHeight)
        {
            if ((m_pixelWidth != pixelWidth) ||
                (m_pixelHeight != pixelHeight))
            {
                m_pixelWidth = pixelWidth;
                m_pixelHeight = pixelHeight;
                CleanupSurfaces();
                QueueHelper(QueueRenderMode::RenderDXGI);
            }
        }

        property IntPtr HWND
        {
            IntPtr get() { return (IntPtr)(void*)m_hwnd; }
            void set(IntPtr hwnd) { m_hwnd = (::HWND)(void*)hwnd; }
        }

        void RequestRenderD2D()
        {
            QueueHelper(QueueRenderMode::RenderDXGI);
        }

        !SurfaceQueueInteropHelper()
        {            
            CleanupD3D();
        }

        ~SurfaceQueueInteropHelper()
        {
            CleanupD3D();
        }        		
    };

}}}
