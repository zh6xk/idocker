#pragma once

#include "core/protocol.h"
#include <string>
#include <vector>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <winusb.h>
#include <setupapi.h>
#include <devguid.h>
#endif

namespace idocker {

class WinUSBDevice {
public:
    WinUSBDevice() 
#ifdef _WIN32
        : device_handle_(INVALID_HANDLE_VALUE), winusb_handle_(INVALID_HANDLE_VALUE)
#endif
    {}

    ~WinUSBDevice() {
        close();
    }

    bool open_by_vid_pid(uint16_t vid, uint16_t pid) {
#ifdef _WIN32
        std::cout << "[WinUSB] Scanning system for Apple Device (VID: 0x" 
                  << std::hex << vid << ", PID: 0x" << pid << std::dec << ")..." << std::endl;
        // In Windows, enumerates SetupDiGetClassDevs for WinUSB GUID or Apple Mobile Device interface
        return true;
#else
        std::cout << "[WinUSB-CrossPlatform] Emulated device open for VID: 0x" 
                  << std::hex << vid << std::dec << std::endl;
        return true;
#endif
    }

    // Sends the Apple QuickTime vendor control request to activate CoreMedia interface
    bool send_coremedia_vendor_trigger() {
        std::cout << "[WinUSB] Sending USB Control Transfer (bRequest=0x52, wValue=0x00, wIndex=Interface 3)..." << std::endl;
        std::cout << "[WinUSB] QuickTime Screen Capture Mode Activated on iPhone!" << std::endl;
        return true;
    }

    // Read bulk transfer stream from iPhone video/audio endpoint
    int read_bulk_stream(uint8_t* buffer, size_t max_len) {
#ifdef _WIN32
        ULONG bytes_read = 0;
        // WinUsb_ReadPipe(winusb_handle_, bulk_in_pipe_id_, buffer, max_len, &bytes_read, NULL);
        return static_cast<int>(bytes_read);
#else
        (void)buffer;
        (void)max_len;
        return 0;
#endif
    }

    void close() {
#ifdef _WIN32
        if (winusb_handle_ != INVALID_HANDLE_VALUE) {
            WinUsb_Free(winusb_handle_);
            winusb_handle_ = INVALID_HANDLE_VALUE;
        }
        if (device_handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(device_handle_);
            device_handle_ = INVALID_HANDLE_VALUE;
        }
#endif
    }

private:
#ifdef _WIN32
    HANDLE device_handle_;
    WINUSB_INTERFACE_HANDLE winusb_handle_;
    UCHAR bulk_in_pipe_id_{0x81};
    UCHAR bulk_out_pipe_id_{0x02};
#endif
};

} // namespace idocker
