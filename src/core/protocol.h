#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace idocker {

// USB Vendor & Device Constants for Apple iOS Devices
constexpr uint16_t APPLE_VENDOR_ID = 0x05AC;

// Magic Words used in Apple QuickTime / CoreMedia AV Stream Container
constexpr uint32_t MAGIC_FEEDFACE = 0xFEEDFACE;
constexpr uint32_t MAGIC_PING     = 0x50696E47; // 'PinG'
constexpr uint32_t MAGIC_SYNC     = 0x53796E43; // 'SynC'
constexpr uint32_t MAGIC_ASYN     = 0x4173796E; // 'Asyn'
constexpr uint32_t MAGIC_RPLY     = 0x52504C59; // 'RPLY'
constexpr uint32_t MAGIC_EAT_ME   = 0x6561746D; // 'eatm'

// Sub-packet Type Identifiers
constexpr uint32_t TYPE_CWPA      = 0x63777061; // 'cwpa' - Clock Wait Packet
constexpr uint32_t TYPE_AFMT      = 0x61666D74; // 'afmt' - Audio Format
constexpr uint32_t TYPE_CVRP      = 0x63767270; // 'cvrp' - CoreVideo Request Packet
constexpr uint32_t TYPE_CLOK      = 0x636C6F6B; // 'clok' - Clock Timebase
constexpr uint32_t TYPE_TIME      = 0x74696D65; // 'time' - Time synchronization
constexpr uint32_t TYPE_SPRP      = 0x73707270; // 'sprp' - Set Property
constexpr uint32_t TYPE_SRAT      = 0x73726174; // 'srat' - Set Time Rate & Anchor
constexpr uint32_t TYPE_TBAS      = 0x74626173; // 'tbas' - Set TimeBase
constexpr uint32_t TYPE_HPAK      = 0x6870616B; // 'hpak' - H.264 packet container
constexpr uint32_t TYPE_NEED      = 0x6E656564; // 'need' - Flow control / Frame request

// Data Representation Markers
constexpr uint32_t MARKER_DICT    = 0x64696374; // 'dict' (Key-Value Dictionary)
constexpr uint32_t MARKER_DATV    = 0x64617476; // 'datv' (Binary Data Value)
constexpr uint32_t MARKER_STRV    = 0x73747276; // 'strv' (String Value)
constexpr uint32_t MARKER_BULV    = 0x62756C76; // 'bulv' (Boolean Value)
constexpr uint32_t MARKER_NMBV    = 0x6E6D6276; // 'nmbv' (Number Value)
constexpr uint32_t MARKER_FDSC    = 0x66647363; // 'fdsc' (Format Description)

// CoreMedia Stream Packet Header (16 bytes)
#pragma pack(push, 1)
struct PacketHeader {
    uint32_t length;     // Total packet length (Big-Endian)
    uint32_t magic;      // Magic identifier ('Asyn', 'SynC', 'PinG')
    uint64_t correlation_id; // Session correlation or clock index
};
#pragma pack(pop)

enum class PacketType {
    Unknown,
    Ping,
    Sync,
    Async,
    Reply
};

inline PacketType identify_packet(uint32_t magic) {
    switch (magic) {
        case MAGIC_PING: return PacketType::Ping;
        case MAGIC_SYNC: return PacketType::Sync;
        case MAGIC_ASYN: return PacketType::Async;
        case MAGIC_RPLY: return PacketType::Reply;
        default: return PacketType::Unknown;
    }
}

} // namespace idocker
