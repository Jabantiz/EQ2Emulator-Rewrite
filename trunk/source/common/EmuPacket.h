#pragma once

#include "Packets\Packet.h"

enum EmuOpcode_t : uint8_t;
class WorldStream;
class ZoneStream;

class EmuPacket : public Packet {
protected:
	EmuPacket(EmuOpcode_t op);

public:
	~EmuPacket() = default;

	uint32_t Write(unsigned char*& writeBuffer) override;

	bool Read(const unsigned char* in_buf, uint32_t bufsize);

	bool bCompressed;

	virtual void HandlePacket(ZoneStream* s) { LogError(LOG_PACKET, 0, "Called HandlePacket(ZoneStream*) on packet %u which has no handler.", opcode); }
	virtual void HandlePacket(WorldStream* s) { LogError(LOG_PACKET, 0, "Called HandlePacket(WorldStream*) on packet %u which has no handler.", opcode); }

private:
	EmuOpcode_t opcode;
};