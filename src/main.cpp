#include "core/protocol.h"
#include "core/demuxer.h"
#include "core/usb_transport.h"
#include "core/usbmux_client.h"
#include "core/av_state_machine.h"
#include "windows/winusb_driver.h"
#include "windows/d3d11_renderer.h"
#include "windows/wasapi_audio.h"
#ifdef _WIN32
#include "windows/desktop_window.h"
#endif
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    std::cout << "=================================================" << std::endl;
    std::cout << "  idocker - iOS USB Screen & Audio Mirroring     " << std::endl;
    std::cout << "  Version 0.3.0 (Native usbmuxd + AV Handshake)  " << std::endl;
    std::cout << "=================================================" << std::endl;

#ifdef _WIN32
    // Create native desktop window
    idocker::DesktopWindow window;
    if (!window.create(L"idocker - iPhone Screen Mirror", 460, 920)) {
        std::cerr << "Could not create window. Exiting." << std::endl;
        return 1;
    }
    HWND hwnd = window.get_hwnd();
#else
    void* hwnd = nullptr;
#endif

    idocker::UsbMuxClient usbmux;
    idocker::AVStateMachine state_machine;
    idocker::D3D11Renderer renderer;
    idocker::WASAPIAudioPlayer audio;

    // 1. Initialize Audio Engine & Video Renderer
    audio.initialize(48000, 2);
    renderer.initialize(hwnd, 1920, 1080);

    // 2. Connect to Apple Mobile Device Service (usbmuxd)
    std::cout << "\n[Step 1] Interfacing with Apple Mobile Device Service..." << std::endl;
    if (usbmux.connect_daemon("127.0.0.1", 27015)) {
        usbmux.list_devices();
    } else {
        std::cout << "[Note] If Apple Service is not running, ensure iTunes/Apple Devices is installed." << std::endl;
    }

    // 3. Start CoreMedia AV State Machine
    std::cout << "\n[Step 2] Initializing CoreMedia AV State Machine..." << std::endl;
    state_machine.start_session();

    std::cout << "\n[Status] Ready & Listening for iOS device..." << std::endl;

#ifdef _WIN32
    // Windows Message Pump Loop (Keeps window alive and interactive)
    while (window.is_running()) {
        window.pump_events();
        std::this_thread::sleep_for(std::chrono::milliseconds(8)); // ~120Hz polling
    }
#else
    std::cout << "[Info] Emulated run finished." << std::endl;
#endif

    return 0;
}
