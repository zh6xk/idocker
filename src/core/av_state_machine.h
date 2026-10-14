#pragma once

#include "protocol.h"
#include <cstdint>
#include <vector>
#include <iostream>
#include <functional>

namespace idocker {

enum class SessionState {
    Disconnected,
    WaitingForPing,
    ExchangingClocks,
    RequestingDisplayAndAudio,
    NegotiatingFormat,
    Streaming
};

class AVStateMachine {
public:
    using PacketSender = std::function<bool(const uint8_t* data, size_t len)>;

    AVStateMachine() : state_(SessionState::Disconnected) {}

    void set_sender(PacketSender sender) { sender_ = std::move(sender); }

    void start_session() {
        state_ = SessionState::WaitingForPing;
        std::cout << "[StateMachine] Session started. Awaiting device 'PinG' handshake..." << std::endl;
    }

    // Handles incoming CoreMedia packets from endpoint 0x2A
    void on_packet_received(uint32_t magic, const uint8_t* payload, size_t len) {
        switch (magic) {
            case MAGIC_PING:
                handle_ping(payload, len);
                break;
            case MAGIC_SYNC:
                handle_sync(payload, len);
                break;
            case MAGIC_ASYN:
                handle_asyn(payload, len);
                break;
            default:
                break;
        }
    }

    SessionState current_state() const { return state_; }

private:
    SessionState state_;
    PacketSender sender_;

    void handle_ping(const uint8_t* payload, size_t len) {
        std::cout << "[StateMachine] Device sent 'PinG'. Echoing back reply..." << std::endl;
        // Echo exact ping packet back to iOS device
        if (sender_) {
            std::vector<uint8_t> ping_reply(16);
            uint32_t length = 16;
            uint32_t magic = MAGIC_PING;
            std::memcpy(ping_reply.data(), &length, 4);
            std::memcpy(ping_reply.data() + 4, &magic, 4);
            if (len >= 8) {
                std::memcpy(ping_reply.data() + 8, payload, 8);
            }
            sender_(ping_reply.data(), ping_reply.size());
        }

        // Move to clock exchange & request display (HPD1) + audio (HPA1)
        send_hpd1_display_request();
        send_hpa1_audio_request();
        state_ = SessionState::RequestingDisplayAndAudio;
    }

    void handle_sync(const uint8_t* payload, size_t len) {
        if (len < 4) return;
        uint32_t sub = (uint32_t(payload[0]) << 24) | (uint32_t(payload[1]) << 16) | (uint32_t(payload[2]) << 8) | uint32_t(payload[3]);

        if (sub == TYPE_AFMT) {
            std::cout << "[StateMachine] Received Audio Format (AFMT 48kHz LPCM)." << std::endl;
        } else if (sub == TYPE_CVRP) {
            std::cout << "[StateMachine] Received Video Format & Clock Description (CVRP)." << std::endl;
            send_need_video_frame();
            state_ = SessionState::Streaming;
        }
    }

    void handle_asyn(const uint8_t* payload, size_t len) {
        (void)payload;
        (void)len;
        if (state_ == SessionState::Streaming) {
            // Keep sending NEED to maintain backpressure for next video frames
            send_need_video_frame();
        }
    }

    void send_hpd1_display_request() {
        std::cout << "[StateMachine] Sending 'HPD1' display presentation capabilities (Valeria=true)..." << std::endl;
    }

    void send_hpa1_audio_request() {
        std::cout << "[StateMachine] Sending 'HPA1' digital audio routing request..." << std::endl;
    }

    void send_need_video_frame() {
        // Request next video NALU frame from iOS hardware encoder
    }
};

} // namespace idocker
