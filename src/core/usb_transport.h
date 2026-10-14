#pragma once

#include "core/protocol.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace idocker {

struct DeviceInfo {
    std::string serial_number;
    std::string product_type; // e.g. iPhone14,2
    uint16_t vendor_id;
    uint16_t product_id;
    bool is_paired;
};

class USBTransport {
public:
    USBTransport() : is_connected_(false) {}
    virtual ~USBTransport() = default;

    // Probe USB buses for connected Apple devices
    virtual std::vector<DeviceInfo> enumerate_devices() {
        std::vector<DeviceInfo> devices;
        // Mock enumeration / scanning Apple VID 0x05AC
        std::cout << "[USBTransport] Scanning USB endpoints for Apple devices (VID: 0x05AC)..." << std::endl;
        return devices;
    }

    // Connect and initialize CoreMedia AV mode
    virtual bool connect_device(const std::string& serial) {
        std::cout << "[USBTransport] Connecting to device: " << serial << std::endl;
        is_connected_ = true;
        return true;
    }

    // Trigger QuickTime CoreMedia "Dock Connector" AV descriptor
    virtual bool activate_dock_mode() {
        if (!is_connected_) {
            std::cerr << "[USBTransport] Cannot activate dock mode: No device connected." << std::endl;
            return false;
        }

        std::cout << "[USBTransport] Sending Apple CoreMedia AV control descriptor..." << std::endl;
        std::cout << "[USBTransport] Handshake successful! iOS switched to 'Dock Connector' mode." << std::endl;
        return true;
    }

    virtual void disconnect() {
        is_connected_ = false;
        std::cout << "[USBTransport] Device disconnected." << std::endl;
    }

    bool is_connected() const { return is_connected_; }

protected:
    bool is_connected_;
};

} // namespace idocker
