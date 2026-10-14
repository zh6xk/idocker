#pragma once

#ifdef _WIN32
#include <windows.h>
#include <string>
#include <iostream>

namespace idocker {

class DesktopWindow {
public:
    DesktopWindow() : hwnd_(NULL), is_running_(false) {}

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_DESTROY) {
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    bool create(const std::wstring& title, int width = 450, int height = 900) {
        HINSTANCE hInstance = GetModuleHandleW(NULL);
        const wchar_t CLASS_NAME[] = L"idocker_WindowClass";

        WNDCLASSW wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

        RegisterClassW(&wc);

        hwnd_ = CreateWindowExW(
            0,
            CLASS_NAME,
            title.c_str(),
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            NULL, NULL, hInstance, NULL
        );

        if (!hwnd_) {
            std::cerr << "[Window] Failed to create native desktop window." << std::endl;
            return false;
        }

        ShowWindow(hwnd_, SW_SHOW);
        UpdateWindow(hwnd_);
        is_running_ = true;
        return true;
    }

    void pump_events() {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                is_running_ = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    HWND get_hwnd() const { return hwnd_; }
    bool is_running() const { return is_running_; }

private:
    HWND hwnd_;
    bool is_running_;
};

} // namespace idocker
#endif
