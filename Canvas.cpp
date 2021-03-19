/*
* Canvas.cpp, 10/4/2020 10:54 PM
*/

#include <d2d1.h>
#include <dwrite.h>

class Canvas {
public:

    ID2D1Factory* factory = nullptr;
    IDWriteFactory* write_factory = nullptr;
    ID2D1RenderTarget* render = nullptr;
    IDWriteTextFormat* text_format = nullptr;
    ID2D1SolidColorBrush* brush;
    bool is_dc_render_target = true;

    const wchar_t* font_name = L"Fontin Smallcaps";
    float font_size = 18;

    Canvas() {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &factory);
        HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(write_factory),
            reinterpret_cast<IUnknown **>(&write_factory));

        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
        factory->CreateDCRenderTarget(&props, (ID2D1DCRenderTarget**)&render);

        if (SUCCEEDED(hr)) {
            write_factory->CreateTextFormat(
                font_name,
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                font_size,
                L"en-us",
                &text_format);
            render->CreateSolidColorBrush(D2D1::ColorF(0, 0), &brush);
        }
    }

    virtual ~Canvas() {
        render->BeginDraw();
        render->Clear();
        render->EndDraw();

        text_format->Release();
        render->Release();
        write_factory->Release();
        factory->Release();
    }

    void bind(HWND hwnd, bool use_dc = true) {
        RECT r;

        if (is_dc_render_target && use_dc) {
            GetClientRect(hwnd, &r);
            ((ID2D1DCRenderTarget*)render)->BindDC(GetDC(hwnd), &r);
        } else {
            GetWindowRect(hwnd, &r);
            D2D1_SIZE_U size = D2D1::SizeU(r.right - r.left, r.bottom - r.top);
            factory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
                D2D1::HwndRenderTargetProperties(hwnd, size),
                (ID2D1HwndRenderTarget**)&render);
            is_dc_render_target = false;
        }
    }

    void begin_draw() {
        render->BeginDraw();
    }

    void end_draw() {
        render->EndDraw();
    }

    void clear() {
        render->Clear();
    }

    void push_rectangle_clip(float x0, float y0, float x1, float y1) {
        render->PushAxisAlignedClip({x0, y0, x1, y1}, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    }

    void pop_rectangle_clip() {
        render->PopAxisAlignedClip();
    }

    void draw_bitmap(ID2D1Bitmap* bitmap, float x0, float y0, float x1, float y1, float opacity = 1.0) {
        D2D1_RECT_F dest = D2D1::RectF(x0, y0, x1, y1);
        render->DrawBitmap(bitmap, &dest, opacity);
    }

    void draw_text(wstring text, float x, float y, int rgb, int backgroud, float alpha = 1.0, int align = 0) {
        IDWriteTextLayout* layout;
        
        HRESULT hr = write_factory->CreateTextLayout(text.c_str(), text.size(),
            text_format, 1920, 1080, &layout);

        if (SUCCEEDED(hr)) {
            DWRITE_TEXT_METRICS m;

            layout->GetMetrics(&m);
            x = x - m.width * align / 2;
            fill_rect(x, y, x + m.width, y + m.height, backgroud, alpha);
            brush->SetColor(D2D1::ColorF(rgb, alpha));
            render->DrawTextLayout({x, y}, layout, brush);
            layout->Release();
        }
    }

    void draw_line(float x0, float y0, float x1, float y1, int rgb) {
        brush->SetColor(D2D1::ColorF(rgb));
        render->DrawLine({x0, y0}, {x1, y1}, brush);
    }

    void draw_rect(float x0, float y0, float x1, float y1, int rgb, float width = 1.0) {
        brush->SetColor(D2D1::ColorF(rgb));
        render->DrawRectangle({x0, y0, x1, y1}, brush, width);
    }

    void draw_rounded_rect(float x0, float y0, float x1, float y1, float rx, float ry, int rgb, float width = 1.0) {
        brush->SetColor(D2D1::ColorF(rgb));
        render->DrawRoundedRectangle({{x0, y0, x1, y1}, rx, ry}, brush, width);
    }

    void draw_circle(float x, float y, float radius, int rgb, float width = 1) {
        brush->SetColor(D2D1::ColorF(rgb));
        render->DrawEllipse({{x, y}, radius, radius}, brush, width);
    }

    void fill_rect(float x0, float y0, float x1, float y1, int rgb, float alpha = 1.0) {
        brush->SetColor(D2D1::ColorF(rgb, alpha));
        render->FillRectangle({x0, y0, x1, y1}, brush);
    }

    void fill_rounded_rect(float x0, float y0, float x1, float y1, float rx, float ry, int rgb, float alpha = 1.0) {
        brush->SetColor(D2D1::ColorF(rgb, alpha));
        render->FillRoundedRectangle({{x0, y0, x1, y1}, rx, ry}, brush);
    }

    void fill_circle(float x, float y, float radius, int rgb, float alpha = 1.0) {
        brush->SetColor(D2D1::ColorF(rgb, alpha));
        render->FillEllipse({{x, y}, radius, radius}, brush);
    }
};
