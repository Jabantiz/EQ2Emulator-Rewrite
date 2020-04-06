#include "stdafx.h"
#include "EncodedPackets.h"

using namespace std;

void EncodedPackets::TypeRefCheck(EEncodedPackets t, uint32_t ref) {
	if (ref != 0) {
		//the ref id is only used for certain packets, sanity check
		assert(t == EEncoded_UpdateSpawnPos || t == EEncoded_UpdateSpawnInfo || t == EEncoded_UpdatePosition);
	}
}

shared_ptr<EncodedBuffer> EncodedPackets::GetBuffer(EEncodedPackets t, uint32_t ref) {
	TypeRefCheck(t, ref);
	WriteLocker lock(bufferLock);
	shared_ptr<EncodedBuffer>& ret = buffers[t][ref];
	if (!ret) {
		ret = make_shared<EncodedBuffer>();
	}
	return ret;
}

void EncodedPackets::RemoveBuffer(EEncodedPackets t, uint32_t ref) {
	TypeRefCheck(t, ref);
	WriteLocker lock(bufferLock);
	buffers[t].erase(ref);
}