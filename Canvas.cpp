/*
* Canvas.cpp, 10/4/2020 10:54 PM
*/

#include <string>
#include <stdexcept>
#include <wrl.h>

#include <dxgi1_3.h>
#include <d3d11.h>
#include <d2d1_1.h>
#include <dcomp.h>
#include <dwrite.h>

using namespace Microsoft::WRL;

namespace DX {
    inline void ThrowIfFailed(HRESULT hr) {
        if (FAILED(hr)) {
            char what_msg[32];

            sprintf(what_msg, "DX error code: %#x.", hr);
            throw std::runtime_error(what_msg);
        }
    }
}

class D2DCanvas {
public:

    // D2D render target and common brush object
    ID2D1RenderTarget* rt = nullptr;
    ComPtr<ID2D1SolidColorBrush> brush;

    // for drawing text
    ComPtr<IDWriteFactory> write_factory;
    ComPtr<IDWriteTextFormat> default_format;
    ComPtr<IDWriteTextFormat> default_big_format;

    // the window handle and size
    HWND hwnd;
    UINT32 width = 800, height = 600;

    D2DCanvas() {
    }

    ~D2DCanvas() {
        default_format.Reset();
        default_big_format.Reset();
        write_factory.Reset();
    }

    virtual void bind(HWND hwnd) {
        this->hwnd = hwnd;

        // set default font name and size
        if (!default_format)
            set_font(L"Fontin SmallCaps", 12);

        // resize to fit the window
        resize();
    }

    virtual bool resize() {
        RECT rect;

        GetClientRect(hwnd, &rect);
        if (rect.right > 0 && rect.bottom > 0) {
            if (width != rect.right || height != rect.bottom) {
                width = rect.right;
                height = rect.bottom;
                return true;
            }
        }

        return false;
    }

    virtual void begin_draw() {
        rt->BeginDraw();
    }

    virtual void end_draw() {
        DX::ThrowIfFailed(rt->EndDraw());
    }

    void clear() {
        rt->Clear();
    }

    void set_font(const wchar_t* font_name, int font_size) {
        if (!write_factory) {
            DX::ThrowIfFailed(
                DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                    __uuidof(write_factory.Get()),
                    reinterpret_cast<IUnknown **>(write_factory.GetAddressOf()))
                );
        }

        DX::ThrowIfFailed(
            write_factory->CreateTextFormat(
                font_name,
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                font_size,
                L"en-us",
                &default_format)
            );

        DX::ThrowIfFailed(
            write_factory->CreateTextFormat(
                font_name,
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                font_size * 2,
                L"en-us",
                &default_big_format)
            );
    }

    void push_rectangle_clip(float x0, float y0, float x1, float y1) {
        rt->PushAxisAlignedClip({x0, y0, x1, y1}, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    }

    void pop_rectangle_clip() {
        rt->PopAxisAlignedClip();
    }

    void draw_bitmap(ID2D1Bitmap* bitmap, float x0, float y0, float x1, float y1, float opacity = 1.0) {
        D2D1_RECT_F dest = D2D1::RectF(x0, y0, x1, y1);
        rt->DrawBitmap(bitmap, &dest, opacity);
    }

    void draw_text(std::wstring text, float x, float y, int rgb, int backgroud, float alpha = 1.0, int align = 0) {
        IDWriteTextLayout* layout;
        
        HRESULT hr = write_factory->CreateTextLayout(text.c_str(), text.size(),
            default_format.Get(), width, height, &layout);

        if (SUCCEEDED(hr)) {
            DWRITE_TEXT_METRICS m;

            layout->GetMetrics(&m);
            x = x - m.width * align / 2;
            fill_rect(x, y, x + m.width, y + m.height, backgroud, alpha);
            brush->SetColor(D2D1::ColorF(rgb, alpha));
            rt->DrawTextLayout({x, y}, layout, brush.Get());
            layout->Release();
        }
    }

    void draw_big_text(std::wstring text, float x, float y, int rgb, int backgroud, float alpha = 1.0, int align = 0) {
        IDWriteTextLayout* layout;
        
        HRESULT hr = write_factory->CreateTextLayout(text.c_str(), text.size(),
            default_big_format.Get(), width, height, &layout);

        if (SUCCEEDED(hr)) {
            DWRITE_TEXT_METRICS m;

            layout->GetMetrics(&m);
            x = x - m.width * align / 2;
            fill_rect(x, y, x + m.width, y + m.height, backgroud, alpha);
            brush->SetColor(D2D1::ColorF(rgb, alpha));
            rt->DrawTextLayout({x, y}, layout, brush.Get());
            layout->Release();
        }
    }

    void draw_line(float x0, float y0, float x1, float y1, int rgb) {
        brush->SetColor(D2D1::ColorF(rgb));
        rt->DrawLine({x0, y0}, {x1, y1}, brush.Get());
    }

    void draw_rect(float x0, float y0, float x1, float y1, int rgb, float width = 1.0) {
        brush->SetColor(D2D1::ColorF(rgb));
        rt->DrawRectangle({x0, y0, x1, y1}, brush.Get(), width);
    }

    void draw_rounded_rect(float x0, float y0, float x1, float y1, float rx, float ry, int rgb, float width = 1.0) {
        brush->SetColor(D2D1::ColorF(rgb));
        rt->DrawRoundedRectangle({{x0, y0, x1, y1}, rx, ry}, brush.Get(), width);
    }

    void draw_circle(float x, float y, float radius, int rgb, float width = 1) {
        brush->SetColor(D2D1::ColorF(rgb));
        rt->DrawEllipse({{x, y}, radius, radius}, brush.Get(), width);
    }

    void fill_rect(float x0, float y0, float x1, float y1, int rgb, float alpha = 1.0) {
        brush->SetColor(D2D1::ColorF(rgb, alpha));
        rt->FillRectangle({x0, y0, x1, y1}, brush.Get());
    }

    void fill_rounded_rect(float x0, float y0, float x1, float y1, float rx, float ry, int rgb, float alpha = 1.0) {
        brush->SetColor(D2D1::ColorF(rgb, alpha));
        rt->FillRoundedRectangle({{x0, y0, x1, y1}, rx, ry}, brush.Get());
    }

    void fill_circle(float x, float y, float radius, int rgb, float alpha = 1.0) {
        brush->SetColor(D2D1::ColorF(rgb, alpha));
        rt->FillEllipse({{x, y}, radius, radius}, brush.Get());
    }
};

class DCCanvas : public D2DCanvas {
public:

    void bind(HWND hwnd) {
        ComPtr<ID2D1Factory1> factory;
        DX::ThrowIfFailed(
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.GetAddressOf())
            );

        D2D1_RENDER_TARGET_PROPERTIES props =
            D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
        DX::ThrowIfFailed(
            factory->CreateDCRenderTarget(&props, (ID2D1DCRenderTarget**)&rt)
            );

        rt->CreateSolidColorBrush(D2D1::ColorF(0, 0), &brush);
        D2DCanvas::bind(hwnd);
    }

    bool resize() {
        if (D2DCanvas::resize()) {
            RECT rect;

            GetClientRect(hwnd, &rect);
            DX::ThrowIfFailed(
                ((ID2D1DCRenderTarget*)rt)->BindDC(GetDC(hwnd), &rect)
                );

            return true;
        }

        return false;
    }
};

class HwndCanvas : public D2DCanvas {
public:

    void bind(HWND hwnd) {
        ComPtr<ID2D1Factory1> factory;
        DX::ThrowIfFailed(
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.GetAddressOf())
            );

        RECT rect;
        GetWindowRect(hwnd, &rect);
        D2D1_SIZE_U size = D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top);
        DX::ThrowIfFailed(
            factory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 96, 96),
                D2D1::HwndRenderTargetProperties(hwnd, size),
                (ID2D1HwndRenderTarget**)&rt)
            );

        rt->CreateSolidColorBrush(D2D1::ColorF(0, 0), &brush);
        D2DCanvas::bind(hwnd);
    }

    bool resize() {
        if (D2DCanvas::resize()) {
            RECT rect;

            GetWindowRect(hwnd, &rect);
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;

            DX::ThrowIfFailed(
                ((ID2D1HwndRenderTarget*)rt)->Resize(D2D1_SIZE_U{width, height})
                );

            return true;
        }

        return false;
    }
};

class DXGICanvas : public D2DCanvas {

    void create_swap_chain(ComPtr<ID3D11Device>& device, ComPtr<IDXGIDevice1>& dxgi_device) {
        // Allocate a descriptor.
        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
        swap_chain_desc.Width = width;
        swap_chain_desc.Height = height;
        swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
        swap_chain_desc.SampleDesc.Count = 1;                // don't use multi-sampling
        swap_chain_desc.SampleDesc.Quality = 0;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount = 2;                     // use double buffering to enable flip
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this SwapEffect
        swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;


        // Identify the physical adapter (GPU or card) this device is runs on.
        IDXGIAdapter* dxgi_adapter;
        DX::ThrowIfFailed(
            dxgi_device->GetAdapter(&dxgi_adapter)
            );

        // Get the factory object that created the DXGI device.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(
            dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
            );

        // Get the final swap chain for this window from the DXGI factory.
        DX::ThrowIfFailed(
            dxgiFactory->CreateSwapChainForComposition(
                device.Get(),
                &swap_chain_desc,
                nullptr,    // allow on all displays
                &swap_chain
                )
            );
    }

    void create_bitmap() {
        // Now we set up the Direct2D render target bitmap linked to the swapchain. 
        // Whenever we render to this bitmap, it is directly rendered to the 
        // swap chain associated with the window.
        D2D1_BITMAP_PROPERTIES1 props = 
            D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                0,
                0);

        // Direct2D needs the dxgi version of the backbuffer surface pointer.
        ComPtr<IDXGISurface1> dxgi_back_buffer;
        DX::ThrowIfFailed(
            swap_chain->GetBuffer(0, IID_PPV_ARGS(&dxgi_back_buffer))
            );

        // Get a D2D surface from the DXGI back buffer to use as the D2D render target.
        DX::ThrowIfFailed(
            ((ID2D1DeviceContext*)rt)->CreateBitmapFromDxgiSurface(
                dxgi_back_buffer.Get(),
                &props,
                &target_bitmap
                )
            );

        // Now we can set the Direct2D render target.
        ((ID2D1DeviceContext*)rt)->SetTarget(target_bitmap.Get());

        // create the brush object
        rt->CreateSolidColorBrush(D2D1::ColorF(0, 0), &brush);
    }

    void create_dcomp(ComPtr<IDXGIDevice1>& dxgi_device, HWND hwnd) {
        // Create the DirectComposition device object.
        DX::ThrowIfFailed(
            DCompositionCreateDevice(
               dxgi_device.Get(),
               __uuidof(IDCompositionDevice),
               reinterpret_cast<void **>(dcomp_device.GetAddressOf())));

        // Bind the DirectComposition device to the target window.
        DX::ThrowIfFailed(
            dcomp_device->CreateTargetForHwnd(hwnd, false, &dcomp_target)
            );

        // Create a visual.
        ComPtr<IDCompositionVisual> visual;
        DX::ThrowIfFailed(
            dcomp_device->CreateVisual(&visual)
            );

        // Set the content of the visual.
        DX::ThrowIfFailed(
            visual->SetContent(swap_chain.Get())
            );

        // Sets a visual object as the new root object of a visual tree.
        DX::ThrowIfFailed(
            dcomp_target->SetRoot(visual.Get())
            );
    }

public:

    // DXGI swap chain
    ComPtr<IDXGISwapChain1> swap_chain;
    ComPtr<ID2D1Bitmap1> target_bitmap;

    // composition device and target
    ComPtr<IDCompositionDevice> dcomp_device;
    ComPtr<IDCompositionTarget> dcomp_target;

    void bind(HWND hwnd) {
        // This array defines the set of DirectX hardware feature levels this app supports.
        // The ordering is important and you should  preserve it.
        // Don't forget to declare your app's minimum required feature level in its
        // description.  All apps are assumed to support 9.1 unless otherwise stated.
        D3D_FEATURE_LEVEL featureLevels[] = 
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };
        D3D_FEATURE_LEVEL m_featureLevel;

        // Create the DX11 API device object, and get a corresponding context.
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        DX::ThrowIfFailed(
            D3D11CreateDevice(
                nullptr,                    // specify null to use the default adapter
                D3D_DRIVER_TYPE_HARDWARE,
                0,                          
                D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                featureLevels,              // list of feature levels this app can support
                ARRAYSIZE(featureLevels),   // number of possible feature levels
                D3D11_SDK_VERSION,          
                &device,                    // returns the Direct3D device created
                &m_featureLevel,            // returns feature level of device created
                &context                    // returns the device immediate context
                )
            );

        // Obtain the underlying DXGI device of the Direct3D11 device.
        ComPtr<IDXGIDevice1> dxgi_device;
        DX::ThrowIfFailed(
            device.As(&dxgi_device)
            );

        ComPtr<ID2D1Factory1> factory;
        DX::ThrowIfFailed(
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.GetAddressOf())
            );

        // Obtain the Direct2D device for 2-D rendering.
        ComPtr<ID2D1Device> d2d_device;
        DX::ThrowIfFailed(
            factory->CreateDevice(dxgi_device.Get(), &d2d_device)
            );

        // Get Direct2D device's corresponding device context object.
        DX::ThrowIfFailed(
            d2d_device->CreateDeviceContext(
                D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
                (ID2D1DeviceContext**)&rt)
            );

        // Ensure that DXGI doesn't queue more than one frame at a time.
        DX::ThrowIfFailed(
            dxgi_device->SetMaximumFrameLatency(1)
            );


        create_swap_chain(device, dxgi_device);
        create_bitmap();
        create_dcomp(dxgi_device, hwnd);
        D2DCanvas::bind(hwnd);
    }

    bool resize() {
        if (D2DCanvas::resize()) {
            ((ID2D1DeviceContext*)rt)->SetTarget(nullptr);
            target_bitmap.Reset();
            DX::ThrowIfFailed(
                swap_chain->ResizeBuffers(2, width, height, DXGI_FORMAT_UNKNOWN, 0)
                );
            create_bitmap();

            return true;
        }

        return false;
    }

    void end_draw() {
        DX::ThrowIfFailed(rt->EndDraw());
        DX::ThrowIfFailed(swap_chain->Present(2, 0));
        DX::ThrowIfFailed(dcomp_device->Commit());
    }
};
