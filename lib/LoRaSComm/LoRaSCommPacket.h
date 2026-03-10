/*
 * LoRaSComm - Packet Structure and Encoding/Decoding
 * 
 * Packet Format (Total: up to 128 bytes)
 * ┌─────────────────────────────────────────────────────────────┐
 * │ Header (7 bytes)                                            │
 * ├─────────────────────────────────────────────────────────────┤
 * │ Payload (up to 119 bytes)                                   │
 * ├─────────────────────────────────────────────────────────────┤
 * │ Footer (2 bytes CRC16)                                      │
 * └─────────────────────────────────────────────────────────────┘
 */

#ifndef LORASCOMM_PACKET_H
#define LORASCOMM_PACKET_H

#include <Arduino.h>

// Packet constants
#define LORASCOMM_MAX_PACKET_SIZE 128
#define LORASCOMM_HEADER_SIZE 7
#define LORASCOMM_FOOTER_SIZE 2
#define LORASCOMM_MAX_PAYLOAD_SIZE (LORASCOMM_MAX_PACKET_SIZE - LORASCOMM_HEADER_SIZE - LORASCOMM_FOOTER_SIZE)

// Special addresses
#define LORASCOMM_BROADCAST_ADDR 0xFF

// Packet types
enum LoRaSCommPacketType : uint8_t {
    PACKET_DATA_UNRELIABLE = 0x00,
    PACKET_DATA_RELIABLE = 0x01,
    PACKET_ACK = 0x02,
    PACKET_NACK = 0x03
};

// Packet header structure
struct LoRaSCommHeader {
    uint8_t srcAddr;        // Source address (1-254, 255=broadcast)
    uint8_t destAddr;       // Destination address
    uint8_t packetType;     // Type of packet (see LoRaSCommPacketType)
    uint8_t sequence;       // Sequence number (0-255, rolls over)
    uint8_t flags;          // Reserved for future use
    uint8_t payloadLen;     // Actual payload length (0-119)
    uint8_t reserved;       // Reserved byte for alignment
} __attribute__((packed));

// Complete packet structure
struct LoRaSCommPacket {
    LoRaSCommHeader header;
    uint8_t payload[LORASCOMM_MAX_PAYLOAD_SIZE];
    uint16_t crc16;
    
    // Constructor
    LoRaSCommPacket() {
        memset(this, 0, sizeof(LoRaSCommPacket));
    }
} __attribute__((packed));

// CRC16-CCITT implementation (polynomial 0x1021)
class LoRaSCommCRC {
public:
    static uint16_t calculate(const uint8_t* data, size_t length) {
        uint16_t crc = 0xFFFF; // Initial value
        
        for (size_t i = 0; i < length; i++) {
            crc ^= (uint16_t)data[i] << 8;
            
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ 0x1021;
                } else {
                    crc = crc << 1;
                }
            }
        }
        
        return crc;
    }
    
    static bool verify(const uint8_t* data, size_t length, uint16_t receivedCrc) {
        return calculate(data, length) == receivedCrc;
    }
};

// Packet encoder/decoder
class LoRaSCommPacketCodec {
public:
    // Encode packet into buffer
    // Returns total packet size in bytes
    static size_t encode(uint8_t* buffer, 
                        uint8_t srcAddr,
                        uint8_t destAddr,
                        LoRaSCommPacketType type,
                        uint8_t sequence,
                        const uint8_t* payload,
                        size_t payloadLen) {
        
        if (payloadLen > LORASCOMM_MAX_PAYLOAD_SIZE) {
            return 0; // Error: payload too large
        }
        
        // Build header
        LoRaSCommHeader header;
        header.srcAddr = srcAddr;
        header.destAddr = destAddr;
        header.packetType = type;
        header.sequence = sequence;
        header.flags = 0;
        header.payloadLen = payloadLen;
        header.reserved = 0;
        
        // Copy header to buffer
        memcpy(buffer, &header, LORASCOMM_HEADER_SIZE);
        
        // Copy payload
        if (payloadLen > 0) {
            memcpy(buffer + LORASCOMM_HEADER_SIZE, payload, payloadLen);
        }
        
        // Calculate CRC over header + payload
        size_t dataSize = LORASCOMM_HEADER_SIZE + payloadLen;
        uint16_t crc = LoRaSCommCRC::calculate(buffer, dataSize);
        
        // Append CRC (big-endian)
        buffer[dataSize] = (crc >> 8) & 0xFF;
        buffer[dataSize + 1] = crc & 0xFF;
        
        return dataSize + LORASCOMM_FOOTER_SIZE;
    }
    
    // Decode packet from buffer
    // Returns true if valid, false if CRC failed or invalid format
    static bool decode(const uint8_t* buffer, 
                      size_t bufferLen,
                      LoRaSCommPacket& packet) {
        
        // Minimum packet size check
        if (bufferLen < LORASCOMM_HEADER_SIZE + LORASCOMM_FOOTER_SIZE) {
            return false;
        }
        
        // Extract header
        memcpy(&packet.header, buffer, LORASCOMM_HEADER_SIZE);
        
        // Validate payload length
        if (packet.header.payloadLen > LORASCOMM_MAX_PAYLOAD_SIZE) {
            return false;
        }
        
        // Check buffer has enough data
        size_t expectedSize = LORASCOMM_HEADER_SIZE + packet.header.payloadLen + LORASCOMM_FOOTER_SIZE;
        if (bufferLen < expectedSize) {
            return false;
        }
        
        // Extract payload
        if (packet.header.payloadLen > 0) {
            memcpy(packet.payload, buffer + LORASCOMM_HEADER_SIZE, packet.header.payloadLen);
        }
        
        // Extract CRC (big-endian)
        size_t crcOffset = LORASCOMM_HEADER_SIZE + packet.header.payloadLen;
        uint16_t receivedCrc = ((uint16_t)buffer[crcOffset] << 8) | buffer[crcOffset + 1];
        
        // Verify CRC
        size_t dataSize = LORASCOMM_HEADER_SIZE + packet.header.payloadLen;
        if (!LoRaSCommCRC::verify(buffer, dataSize, receivedCrc)) {
            return false;
        }
        
        packet.crc16 = receivedCrc;
        return true;
    }
    
    // Helper: Create ACK packet
    static size_t encodeAck(uint8_t* buffer,
                           uint8_t srcAddr,
                           uint8_t destAddr,
                           uint8_t sequence) {
        return encode(buffer, srcAddr, destAddr, PACKET_ACK, sequence, nullptr, 0);
    }
    
    // Helper: Create NACK packet
    static size_t encodeNack(uint8_t* buffer,
                            uint8_t srcAddr,
                            uint8_t destAddr,
                            uint8_t sequence) {
        return encode(buffer, srcAddr, destAddr, PACKET_NACK, sequence, nullptr, 0);
    }
    
    // Validate address
    static bool isValidAddress(uint8_t addr) {
        return addr >= 0x01 && addr <= 0xFF; // 0x00 reserved, 0xFF broadcast
    }
};

#endif // LORASCOMM_PACKET_H