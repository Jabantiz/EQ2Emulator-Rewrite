#pragma once

#include "../../common/Packets/EQ2Packet.h"
#include "../../common/Packets/PacketElements/PacketElements.h"
#include "../../common/Packets/PacketElements/PacketEncodedData.h"

class OP_UpdateInventoryMsg_Packet : public EQ2Packet {
public:
	OP_UpdateInventoryMsg_Packet(uint32_t version) : EQ2Packet(version), packedItems(GetVersion() <= 283), itemArray(version) {
		packedItems.LinkSubstruct(itemArray, "packedItems");
		RegisterElements();
	}

	struct Substruct_InventoryItem : public PacketSubstruct {
		uint32_t uniqueID;
		uint32_t bagID;
		uint32_t inventorySlotID;
		uint64_t flags;
		uint16_t index;
		uint16_t icon;
		uint8_t slotID;
		uint16_t count;
		uint8_t unknown4b;
		uint8_t unknown4c;
		uint8_t itemLevel;
		uint8_t tier;
		uint8_t numSlots;
		uint8_t numEmptySlots;
		uint8_t unknown5;
		uint32_t itemID;
		uint64_t brokerID;
		char itemName[81];

		Substruct_InventoryItem(uint32_t ver = 0) : PacketSubstruct(ver, true) {
			memset(&uniqueID, 0, (itemName + 81) - reinterpret_cast<char*>(&uniqueID));
			RegisterElements();
		}

		void RegisterElements() {
			RegisterUInt32(uniqueID);
			RegisterUInt32(bagID);
			RegisterUInt32(inventorySlotID);
			if (GetVersion() >= 893) {
				RegisterUInt64(flags);
			}
			else {
				RescopeToReference(flags, uint32_t);
				RegisterUInt32(flags);
			}
			RegisterUInt16(index);
			RegisterUInt16(icon);
			RegisterUInt8(slotID);
			if (GetVersion() >= 1208) {
				RegisterUInt16(count);
			}
			else {
				RescopeToReference(count, uint8_t);
				RegisterUInt8(count);
			}
			RegisterUInt8(unknown4b);
			if (GetVersion() >= 57048) {
				RegisterUInt8(unknown4c);
				RegisterUInt8(itemLevel);
			}
			RegisterUInt8(tier);
			RegisterUInt8(numSlots);
			if (GetVersion() >= 1193) {
				RegisterUInt8(numEmptySlots);
			}
			if (GetVersion() >= 1208) {
				RegisterUInt8(unknown5);
			}
			RegisterUInt32(itemID);
			if (GetVersion() >= 57107) {
				RegisterUInt64(brokerID);
			}
			RescopeArrayElement(itemName);
			RegisterChar(itemName)->SetCount(GetVersion() >= 67650 ? 80 : 81);
		}
	};

	struct Substruct_InventoryItemArray : public PacketEncodedData {
		std::vector<Substruct_InventoryItem> items;
		PacketArrayBase* myArrayElement;

		Substruct_InventoryItemArray(uint32_t ver = 0) : PacketEncodedData(ver, true), myArrayElement(nullptr) {
			RegisterElements();
		}

		void RegisterElements() {
			myArrayElement = RegisterArray(items, Substruct_InventoryItem);
		}
	};

	uint16_t itemCount;
	uint8_t inventoryType;
	Substruct_InventoryItemArray itemArray;
	PacketPackedData packedItems;

private:
	void RegisterElements() {
		RegisterUInt16(itemCount)->SetMyArray(itemArray.myArrayElement);
		RegisterSubstruct(packedItems);
		RegisterUInt8(inventoryType);
	}
};