/*
* Canvas.cpp, 10/4/2020 10:54 PM
*/

#include <d2d1.h>

class Canvas {
public:

    ID2D1Factory* factory = nullptr;
    ID2D1DCRenderTarget* render = nullptr;

    Canvas() {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &factory);
        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
            0, 0,
            D2D1_RENDER_TARGET_USAGE_NONE,
            D2D1_FEATURE_LEVEL_DEFAULT);
        factory->CreateDCRenderTarget(&props, (ID2D1DCRenderTarget**)&render);
    }

    virtual ~Canvas() {
        render->BeginDraw();
        render->Clear();
        render->EndDraw();

        render->Release();
        factory->Release();
    }

    void bind(HWND hwnd) {
        RECT r;
        GetClientRect(hwnd, &r);
        render->BindDC(GetDC(hwnd), &r);
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

    void draw_line(float x0, float y0, float x1, float y1, int rgb) {
        ID2D1SolidColorBrush* brush;
        render->CreateSolidColorBrush(D2D1::ColorF(rgb), &brush);
        render->DrawLine({x0, y0}, {x1, y1}, brush);
        brush->Release();
    }

    void draw_rect(float x0, float y0, float x1, float y1, int rgb, float width = 1.0) {
        ID2D1SolidColorBrush* brush;
        render->CreateSolidColorBrush(D2D1::ColorF(rgb), &brush);
        render->DrawRectangle({x0, y0, x1, y1}, brush, width);
        brush->Release();
    }

    void draw_rounded_rect(float x0, float y0, float x1, float y1, float rx, float ry, int rgb, float width = 1.0) {
        ID2D1SolidColorBrush* brush;
        render->CreateSolidColorBrush(D2D1::ColorF(rgb), &brush);
        render->DrawRoundedRectangle({{x0, y0, x1, y1}, rx, ry}, brush, width);
        brush->Release();
    }

    void fill_rect(float x0, float y0, float x1, float y1, int rgb) {
        ID2D1SolidColorBrush* brush;
        render->CreateSolidColorBrush(D2D1::ColorF(rgb), &brush);
        render->FillRectangle({x0, y0, x1, y1}, brush);
        brush->Release();
    }

    void fill_rounded_rect(float x0, float y0, float x1, float y1, float rx, float ry, int rgb) {
        ID2D1SolidColorBrush* brush;
        render->CreateSolidColorBrush(D2D1::ColorF(rgb), &brush);
        render->FillRoundedRectangle({{x0, y0, x1, y1}, rx, ry}, brush);
        brush->Release();
    }
};
