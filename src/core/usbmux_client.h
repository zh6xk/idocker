#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

namespace idocker {

#pragma pack(push, 1)
struct UsbMuxHeader {
    uint32_t length;
    uint32_t version;
    uint32_t message;
    uint32_t tag;
};
#pragma pack(pop)

class UsbMuxClient {
public:
    UsbMuxClient() : sock_(INVALID_SOCKET), is_connected_(false) {
#ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    }

    ~UsbMuxClient() {
        disconnect();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    // Connects to Apple Mobile Device Service daemon on default port 27015
    bool connect_daemon(const std::string& host = "127.0.0.1", int port = 27015) {
        sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock_ == INVALID_SOCKET) {
            std::cerr << "[usbmuxd] Failed to create socket." << std::endl;
            return false;
        }

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

        std::cout << "[usbmuxd] Connecting to Apple Mobile Device Service at " 
                  << host << ":" << port << "..." << std::endl;

        if (connect(sock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            std::cerr << "[usbmuxd] Could not connect to Apple Mobile Device Service (Port " 
                      << port << "). Is iTunes or Apple Devices installed and running?" << std::endl;
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
            return false;
        }

        is_connected_ = true;
        std::cout << "[usbmuxd] Successfully connected to Apple Mobile Device Service daemon!" << std::endl;
        return true;
    }

    // Query Apple daemon for connected iPhone/iPad devices
    bool list_devices() {
        if (!is_connected_) return false;

        std::string plist_req = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\">\n"
            "<dict>\n"
            "   <key>ClientVersionString</key>\n"
            "   <string>idocker-v0.3.0</string>\n"
            "   <key>MessageType</key>\n"
            "   <string>ListDevices</string>\n"
            "   <key>ProgName</key>\n"
            "   <string>idocker</string>\n"
            "</dict>\n"
            "</plist>";

        UsbMuxHeader header;
        header.length = sizeof(UsbMuxHeader) + plist_req.size();
        header.version = 1;
        header.message = 8; // Plist message type
        header.tag = 1;

        std::vector<uint8_t> packet(header.length);
        std::memcpy(packet.data(), &header, sizeof(UsbMuxHeader));
        std::memcpy(packet.data() + sizeof(UsbMuxHeader), plist_req.data(), plist_req.size());

        send(sock_, (const char*)packet.data(), packet.size(), 0);
        std::cout << "[usbmuxd] Sent ListDevices query to Apple daemon." << std::endl;

        // Read response
        UsbMuxHeader resp_header;
        int received = recv(sock_, (char*)&resp_header, sizeof(resp_header), 0);
        if (received >= (int)sizeof(resp_header)) {
            std::vector<char> resp_body(resp_header.length - sizeof(resp_header) + 1, 0);
            recv(sock_, resp_body.data(), resp_header.length - sizeof(resp_header), 0);
            std::cout << "[usbmuxd] Received Apple device inventory: " << std::endl;
            std::cout << resp_body.data() << std::endl;
            return true;
        }

        return false;
    }

    void disconnect() {
        if (sock_ != INVALID_SOCKET) {
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
        }
        is_connected_ = false;
    }

    bool is_connected() const { return is_connected_; }

private:
    SOCKET sock_;
    bool is_connected_;
};

} // namespace idocker
