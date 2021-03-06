#pragma once

class PacketBuffer
{
public:
    static const uint16 MAX_BUF_SIZE = boost::integer_traits<uint16>::max();
    static const uint16 MIN_BUF_SIZE = MAX_BUF_SIZE / 2;

public:
    explicit PacketBuffer();
    ~PacketBuffer();

public:
    // | Packet Header                         | Payload(packet data) |
    // | PacketSize (2byte) | PacketNo (2byte) |                      |
    uint16 GetHeaderSize() const { return sizeof(uint16) + sizeof(uint16); }
    uint16 GetPayloadSize() const { return GetPacketSize() - GetHeaderSize(); }
    uint16 GetRemainSize() const { return static_cast<uint16>(_buffer.size() - _writePos); }
    uint16 GetPacketNo() const;

    bool IsEmptyData() const;
    bool IsNotEnoughBuffer() const;

    bool IsAbleToGetPacket() const;
    void ConsumePacket();
    void TruncateBuffer(uint16 size);

    bool AppendWriteSize(uint16 size);
    void ReArrange();

    char* GetMutableBuffer();
    const char* GetBuffer() const;
    uint16 GetBufferSize() const;

    const char* GetPacketBuffer() const;
    uint16 GetPacketSize() const;

    const char* GetPayloadBuffer() const;
    uint16 GetPayloadBufferSize() const;

    template<typename TPacket>
    bool SetPacket(const TPacket& packet)
    {
        const uint16 packetSize =
            GetHeaderSize() + static_cast<uint16>(packet.ByteSize());
        if (packetSize > GetRemainSize())
        {
            ReArrange();
        }

        if (packetSize > GetRemainSize())
        {
            LOG_ERROR(LOG_FILTER_PACKET_BUFFER, "Not enough remain buffer.");
            return false;
        }

        if (!SetPacketSize(packetSize))
            return false;

        if (!SetPacketNo(TPacket::PROTOCOL_NUMBER))
            return false;

        std::array<char, MAX_BUF_SIZE> serializedBuffer = {0};
        if (!packet.SerializeToArray(serializedBuffer.data(),
            static_cast<int>(serializedBuffer.size())))
            return false;

        if (!AppendBuffer(serializedBuffer.data(),
            static_cast<uint16>(packet.ByteSize())))
            return false;

        return true;
    }

private:
    void Clear() noexcept;
    bool AppendBuffer(const char* pBuffer, uint16 size);

    bool SetPacketSize(uint16 packetSize);
    bool SetPacketNo(uint16 packetNo);

private:
    std::array<char, MAX_BUF_SIZE> _buffer;
    uint16 _readPos;
    uint16 _writePos;
};
