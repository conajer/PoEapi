/*
* Hud.cpp, 9/30/2021 12:54 AM
*/

#include <dwmapi.h>
#include <windows.h>

#include <mutex>
#include <thread>

#include "Canvas.cpp"

class Hud : public DXGICanvas {

    std::mutex thread_mutex;
    DWORD thread_id;
    bool is_visible = false;

    void render_thread_proc() {
        WNDCLASSEX wcx = {};

        wcx.cbSize = sizeof(WNDCLASSEX);
        wcx.lpfnWndProc = DefWindowProc;
        wcx.hInstance = GetModuleHandle(NULL);
        wcx.lpszClassName = "HudWinClass";
        RegisterClassEx(&wcx);

        HWND hwnd = CreateWindowEx(
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
            "HudWinClass",
            "Hud",
            WS_POPUP,
            0, 0, 800, 600,
            nullptr,
            nullptr,
            GetModuleHandle(NULL),
            nullptr);

        MARGINS margins = {-1};
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY | LWA_ALPHA);
        DwmExtendFrameIntoClientArea(hwnd, &margins);
        DXGICanvas::bind(hwnd);

        MSG msg = {};
        RECT rect = {};
    
        std::lock_guard<std::mutex> guard(thread_mutex);
        thread_id = GetCurrentThreadId();
        while (msg.message != WM_QUIT) {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                DispatchMessage(&msg);
            } else {
                if (GetForegroundWindow() != target_window
                    || !GetClientRect(target_window, &rect) || !rect.right || !rect.bottom)
                {
                    hide();
                    Sleep(100);
                    continue;
                }

                try {
                    if (!is_visible || width != rect.right || height != rect.bottom)
                        show();

                    begin_draw();
                    render();
                    end_draw();
                } catch (const std::exception& e) {
                }
            }
        }
        DestroyWindow(hwnd);
    }

public:

    HWND target_window = 0;

    Hud() {
        std::thread render_thread(&Hud::render_thread_proc, std::ref(*this));
        render_thread.detach();
    }

    ~Hud() {
        PostThreadMessage(thread_id, WM_QUIT, 0, 0);
        thread_mutex.lock();
    }

    void bind(HWND hwnd) {
        target_window = hwnd;
    }

    virtual void render() {
        static float x = 51.0, xSpeed = 5.0f, y = 51.0, ySpeed = 5.0f;
        static int n_frames;
        static unsigned int t1, t2;
        static float fps;
        wchar_t buffer[128] = {};

        RECT rect;

        GetClientRect(hwnd, &rect);

        // Ball physics

        // xSpeed += 0.6f;
        x += xSpeed;

        ySpeed += 0.05f;
        y += ySpeed;

        if (y > rect.bottom - 50)
            ySpeed = -ySpeed;
        else if (y < 50)
            ySpeed = -ySpeed;

        if (x > rect.right - 50)
            xSpeed = -xSpeed;
        else if (x < 50)
            xSpeed = -xSpeed;

        // Redraw ball

        n_frames++;
        t2 = GetTickCount();
        if (t2 - t1 >= 1000) {
            fps = 1000. * n_frames / (t2 - t1);
            n_frames = 0;
            t1 = t2;
        }

        swprintf(buffer, L" Ball (%.2f, %.2f) \n %.2f FPS", x, y, fps);
        clear();
        draw_text(buffer, 10., 10., 0xffffff, 0xff);
        draw_circle(x, y, 50.0f, 0xff0000, 2);
    }

    void clear_all() {
        begin_draw();
        clear();
        end_draw();
    }

    void show() {
        RECT rect = {};

        if (target_window) {
            GetClientRect(target_window, &rect);
            ClientToScreen(target_window, (LPPOINT)&rect);
            MoveWindow(hwnd, rect.left, rect.top, rect.right, rect.bottom, false);
            resize();
        }

        is_visible = true;
        ShowWindow(hwnd, SW_SHOWNA);
        UpdateWindow(hwnd);
    }

    void hide() {
        is_visible = false;
        ShowWindow(hwnd, SW_HIDE);
    }
};