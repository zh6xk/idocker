#pragma once

#include "core/protocol.h"
#include <vector>
#include <functional>
#include <cstdint>
#include <iostream>

namespace idocker {

struct VideoFrame {
    std::vector<uint8_t> nalu_data;
    uint64_t pts;
    bool is_keyframe;
};

struct AudioSample {
    std::vector<uint8_t> pcm_data;
    uint32_t sample_rate;
    uint16_t channels;
    uint16_t bits_per_sample;
    uint64_t pts;
};

class StreamDemuxer {
public:
    using VideoCallback = std::function<void(const VideoFrame&)>;
    using AudioCallback = std::function<void(const AudioSample&)>;

    StreamDemuxer() = default;

    void set_video_callback(VideoCallback cb) { video_cb_ = std::move(cb); }
    void set_audio_callback(AudioCallback cb) { audio_cb_ = std::move(cb); }

    // Feeds incoming raw USB bytes and demuxes frames
    void feed_bytes(const uint8_t* data, size_t length) {
        buffer_.insert(buffer_.end(), data, data + length);
        parse_buffer();
    }

private:
    std::vector<uint8_t> buffer_;
    VideoCallback video_cb_;
    AudioCallback audio_cb_;

    static uint32_t read_be32(const uint8_t* p) {
        return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
    }

    static uint32_t read_le32(const uint8_t* p) {
        return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
    }

    void parse_buffer() {
        while (buffer_.size() >= sizeof(PacketHeader)) {
            // Check packet length (CoreMedia headers specify length as big-endian or little-endian depending on frame type)
            uint32_t packet_len = read_be32(buffer_.data());
            
            // Basic sanity check to avoid out-of-bounds or infinite loops
            if (packet_len < 8 || packet_len > 10 * 1024 * 1024) {
                // Try sliding 1 byte forward to resynchronize stream
                buffer_.erase(buffer_.begin());
                continue;
            }

            if (buffer_.size() < packet_len) {
                // Wait for the full packet to arrive via USB endpoint
                break;
            }

            // Extract magic
            uint32_t magic = read_be32(buffer_.data() + 4);
            process_packet(buffer_.data(), packet_len, magic);

            // Pop processed packet
            buffer_.erase(buffer_.begin(), buffer_.begin() + packet_len);
        }
    }

    void process_packet(const uint8_t* packet, size_t length, uint32_t magic) {
        switch (magic) {
            case MAGIC_ASYN:
                handle_async_packet(packet + 8, length - 8);
                break;
            case MAGIC_SYNC:
                handle_sync_packet(packet + 8, length - 8);
                break;
            case MAGIC_PING:
                // Device heartbeat
                break;
            default:
                break;
        }
    }

    void handle_async_packet(const uint8_t* payload, size_t length) {
        if (length < 8) return;
        uint32_t sub_type = read_be32(payload);

        if (sub_type == TYPE_HPAK) {
            // H.264 Video Packet Container
            VideoFrame frame;
            frame.nalu_data.assign(payload + 8, payload + length);
            frame.pts = 0; // extracted from CMTime payload
            frame.is_keyframe = false;

            if (video_cb_) video_cb_(frame);
        } else if (sub_type == TYPE_AFMT) {
            // LPCM Audio Sample Container
            AudioSample sample;
            sample.pcm_data.assign(payload + 8, payload + length);
            sample.sample_rate = 48000;
            sample.channels = 2;
            sample.bits_per_sample = 16;
            sample.pts = 0;

            if (audio_cb_) audio_cb_(sample);
        }
    }

    void handle_sync_packet(const uint8_t* payload, size_t length) {
        // Sync & Handshake replies (CWPA, CVRP, CLOK)
        (void)payload;
        (void)length;
    }
};

} // namespace idocker
