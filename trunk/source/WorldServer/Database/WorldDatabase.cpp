#include "stdafx.h"

#include "WorldDatabase.h"
#include "../../common/log.h"
#include "../../common/timer.h"
#include "../../common/Packets/EQ2Packets/OpcodeManager.h"

#include "../Packets/OP_AllCharactersDescReplyMsg_Packet.h"
#include "../Packets/OP_AllCharactersDescRequestMsg_Packet.h"
#include "../Packets/OP_AllWSDescRequestMsg_Packet.h"
#include "../Packets/OP_CreateCharacterRequestMsg_Packet.h"
#include "../Packets/OP_CreateCharacterReplyMsg_Packet.h" // needed for the defines

#include "../../common/Classes.h"

#ifdef _WIN32
	#include <WS2tcpip.h>
#else
	#include <arpa/inet.h>
#endif

extern Classes classes;

static void DatabaseQueryError(Database *db) {
	LogError(LOG_DATABASE, 0, "Error running MySQL query: %s", db->GetError());
}

WorldDatabase::WorldDatabase() {

}

WorldDatabase::~WorldDatabase() {
	Disconnect();
}

bool WorldDatabase::Start() {
	DatabaseCallbacks callbacks;
	callbacks.query_error = DatabaseQueryError;
	SetCallbacks(&callbacks);

	if (Connect()) {
		LogInfo(LOG_DATABASE, 0, "Connected to MySQL server at %s:%u", GetHost(), GetPort());
		return true;
	}

	LogError(LOG_DATABASE, 0, "Error connecting to MySQL: %s", GetError());
	return false;
}

void WorldDatabase::Stop() {
	Disconnect();
}

bool WorldDatabase::LoadOpcodes() {
	DatabaseResult result;
	bool success;
	uint32_t count = 0;

	success = Select(&result, "SELECT `name`, `version_range1`, `version_range2`, `opcode` FROM `opcodes`");
	if (!success)
		return false;

	while (result.Next()) {
		OpcodeManager::GetGlobal()->RegisterVersionOpcode(result.GetString(0), result.GetUInt16(1), result.GetUInt16(2), result.GetUInt16(3));
		count++;
	}

	LogDebug(LOG_DATABASE, 0, "Loaded %u opcodes", count);
	return true;
}

bool WorldDatabase::GetAccount(Client* client, std::string user, std::string pass) {
	DatabaseResult result;
	bool success;

	char* esc_user = Escape(user.c_str());
	char* esc_pass = Escape(pass.c_str());

	success = Select(&result, "SELECT * FROM `account` WHERE `name` = '%s' AND passwd = md5('%s')", esc_user, esc_pass);
	if (success) {
		success = result.Next();
		if (success) {
			uint32_t id = result.GetUInt32(0);
			client->SetAccount(id);
			UpdateAccountIPAddress(id, client->GetIP());
			UpdateAccountClientVersion(id, client->GetVersion());
		}
		// if user and pass check failed
		else {
			// if auto account creation is enabled
			if (true) {
				// see if there is already an account with this username
				success = Select(&result, "SELECT * FROM `account` WHERE `name` = '%s'", esc_user);
				if (success)
					success = result.Next();

				// account found which means bad password so return false
				if (success) {
					success = false;
				}
				// no account found so create one
				else {
					in_addr ip_addr;
					ip_addr.s_addr = client->GetIP();
					success = Query("INSERT INTO account(`name`, `passwd`, `ip_address`, `last_client_version`) VALUES ('%s', md5('%s'), '%s', %u)", esc_user, esc_pass, inet_ntoa(ip_addr), client->GetVersion());
					if (success) 
						client->SetAccount(LastInsertID());
				}
					
			}
			// No auto account so return false
			else {
				success = false;
			}
		}
	}

	free(esc_user);
	free(esc_pass);
	return success;
}

bool WorldDatabase::UpdateAccountIPAddress(uint32_t account, uint32_t address) {
	in_addr ip_addr;
	ip_addr.s_addr = address;
	return Query("UPDATE `account` SET `ip_address` = '%s' WHERE `id` = %u", inet_ntoa(ip_addr), account);
}

bool WorldDatabase::UpdateAccountClientVersion(uint32_t account, uint16_t version) {
	return Query("UPDATE `account` SET `last_client_version` = %u WHERE `id` = %u", version, account);
}

bool WorldDatabase::LoadCharacters(uint32_t account, OP_AllCharactersDescReplyMsg_Packet* packet) {
	bool ret;
	DatabaseResult result;
	DatabaseResult result2;

	ret = Select(&result, "SELECT id, name, race, class, gender, deity, body_size, body_age, current_zone_id, level, tradeskill_class, tradeskill_level, soga_wing_type, soga_chest_type, soga_legs_type, soga_hair_type, soga_facial_hair_type, soga_model_type, legs_type, chest_type, wing_type, hair_type, facial_hair_type, model_type, unix_timestamp(created_date), unix_timestamp(last_played) FROM characters WHERE account_id = %u AND deleted = 0", account);
	if (!ret)
		return ret;

	while (result.Next()) {
		OP_AllCharactersDescReplyMsg_Packet::CharacterListEntry c;
		c.account_id = account;
		c.server_id = 1;

		if (packet->GetVersion() >= 887)
			c.version = 6;
		else
			c.version = 5;

		c.charid = result.GetUInt32(0);
		c.name = result.GetString(1);
		c.race = result.GetUInt8(2);
		c._class = result.GetUInt8(3);
		c.gender = result.GetUInt8(4);
		//c.deity = result.GetUInt8(5);
		c.body_size = result.GetUInt8(6);
		//c.body_age = result.GetUInt8(7);
		uint32_t zone_id = result.GetUInt32(8);
		ret = Select(&result2, "SELECT name, description FROM zones WHERE id = %u", zone_id);
		if (!ret)
			return ret;
		if (result2.Next()) {
			c.zone = result2.IsNull(0) ? " " : result2.GetString(0);
			c.zonedesc = result2.IsNull(1) ? " " : result2.GetString(1);
			c.zonename2 = " ";
		}
		c.level = result.GetUInt32(9);
		c.tradeskill_class = result.GetUInt8(10);
		c.tradeskill_level = result.GetUInt32(11);

		/* SOGA Appearances */
		//c.soga_wing_type = result.GetUInt16(12);
		//c.soga_cheek_type = result.GetUInt16(13);
		//c.soga_legs_type = result.GetUInt16(14);
		c.soga_hair_type = result.GetUInt16(15);
		c.soga_hair_face_type = result.GetUInt16(16);
		c.soga_race_type = result.GetUInt16(17);

		/* NORMAL Appearances */
		c.legs_type = result.GetUInt16(18);
		c.chest_type = result.GetUInt16(19);
		c.wing_type = result.GetUInt16(20);
		c.hair_type = result.GetUInt16(21);
		c.hair_face_type = result.GetUInt16(22);
		c.race_type = result.GetUInt16(23);

		if (!result.IsNull(24))
			c.created_date = result.GetUInt32(24);
		if (!result.IsNull(25))
			c.last_played = result.GetUInt32(25);


		// TODO char_colors table
		ret = Select(&result2, "SELECT type, red, green, blue FROM char_colors WHERE char_id = %u", c.charid);
		if (!ret)
			return ret;

		while (result2.Next()) {
			std::string type = result2.GetString(0);

			if (type == "skin_color") {
				c.skin_color.Red = result2.GetUInt8(1);
				c.skin_color.Green = result2.GetUInt8(2);
				c.skin_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "eye_color") {
				c.eye_color.Red = result2.GetUInt8(1);
				c.eye_color.Green = result2.GetUInt8(2);
				c.eye_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "hair_color1") {
				c.hair_color1.Red = result2.GetUInt8(1);
				c.hair_color1.Green = result2.GetUInt8(2);
				c.hair_color1.Blue = result2.GetUInt8(3);
			}
			else if (type == "hair_color2") {
				c.hair_color2.Red = result2.GetUInt8(1);
				c.hair_color2.Green = result2.GetUInt8(2);
				c.hair_color2.Blue = result2.GetUInt8(3);
			}
			else if (type == "hair_highlight") {
				// ??? - not in the structs?
				//c.hair_highlight_color.Red = result2.GetUInt8(1);
				//c.hair_highlight_color.Green = result2.GetUInt8(2);
				//c.hair_highlight_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "hair_type_color") {
				c.hair_type_color.Red = result2.GetUInt8(1);
				c.hair_type_color.Green = result2.GetUInt8(2);
				c.hair_type_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "hair_type_highlight_color") {
				c.hair_type_highlight_color.Red = result2.GetUInt8(1);
				c.hair_type_highlight_color.Green = result2.GetUInt8(2);
				c.hair_type_highlight_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "hair_face_color") {
				c.hair_face_color.Red = result2.GetUInt8(1);
				c.hair_face_color.Green = result2.GetUInt8(2);
				c.hair_face_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "hair_face_highlight_color") {
				c.hair_face_highlight_color.Red = result2.GetUInt8(1);
				c.hair_face_highlight_color.Green = result2.GetUInt8(2);
				c.hair_face_highlight_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "wing_color1") {
				c.wing_color1.Red = result2.GetUInt8(1);
				c.wing_color1.Green = result2.GetUInt8(2);
				c.wing_color1.Blue = result2.GetUInt8(3);
			}
			else if (type == "wing_color2") {
				c.wing_color2.Red = result2.GetUInt8(1);
				c.wing_color2.Green = result2.GetUInt8(2);
				c.wing_color2.Blue = result2.GetUInt8(3);
			}
			else if (type == "shirt_color") {
				c.shirt_color.Red = result2.GetUInt8(1);
				c.shirt_color.Green = result2.GetUInt8(2);
				c.shirt_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "unknown_chest_color") {
				c.unknown_chest_color.Red = result2.GetUInt8(1);
				c.unknown_chest_color.Green = result2.GetUInt8(2);
				c.unknown_chest_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "pants_color") {
				c.pants_color.Red = result2.GetUInt8(1);
				c.pants_color.Green = result2.GetUInt8(2);
				c.pants_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "unknown_legs_color") {
				c.unknown_legs_color.Red = result2.GetUInt8(1);
				c.unknown_legs_color.Green = result2.GetUInt8(2);
				c.unknown_legs_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "unknown9") {
				c.unknown9.Red = result2.GetUInt8(1);
				c.unknown9.Green = result2.GetUInt8(2);
				c.unknown9.Blue = result2.GetUInt8(3);
			}
			else if (type == "eye_type") {
				c.eye_type[0] = result2.GetInt8(1);
				c.eye_type[1] = result2.GetInt8(2);
				c.eye_type[2] = result2.GetInt8(3);
			}
			else if (type == "ear_type") {
				c.ear_type[0] = result2.GetInt8(1);
				c.ear_type[1] = result2.GetInt8(2);
				c.ear_type[2] = result2.GetInt8(3);
			}
			else if (type == "eye_brow_type") {
				c.eye_brow_type[0] = result2.GetInt8(1);
				c.eye_brow_type[1] = result2.GetInt8(2);
				c.eye_brow_type[2] = result2.GetInt8(3);
			}
			else if (type == "cheek_type") {
				c.cheek_type[0] = result2.GetInt8(1);
				c.cheek_type[1] = result2.GetInt8(2);
				c.cheek_type[2] = result2.GetInt8(3);
			}
			else if (type == "lip_type") {
				c.lip_type[0] = result2.GetInt8(1);
				c.lip_type[1] = result2.GetInt8(2);
				c.lip_type[2] = result2.GetInt8(3);
			}
			else if (type == "chin_type") {
				c.chin_type[0] = result2.GetInt8(1);
				c.chin_type[1] = result2.GetInt8(2);
				c.chin_type[2] = result2.GetInt8(3);
			}
			else if (type == "nose_type") {
				c.nose_type[0] = result2.GetInt8(1);
				c.nose_type[1] = result2.GetInt8(2);
				c.nose_type[2] = result2.GetInt8(3);
			}
			else if (type == "body_size") {
				c.body_size = result2.GetInt8(1);
			}
			else if (type == "soga_skin_color") {
				c.soga_skin_color.Red = result2.GetUInt8(1);
				c.soga_skin_color.Green = result2.GetUInt8(2);
				c.soga_skin_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_eye_color") {
				c.soga_eye_color.Red = result2.GetUInt8(1);
				c.soga_eye_color.Green = result2.GetUInt8(2);
				c.soga_eye_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_hair_color1") {
				c.soga_hair_color1.Red = result2.GetUInt8(1);
				c.soga_hair_color1.Green = result2.GetUInt8(2);
				c.soga_hair_color1.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_hair_color2") {
				c.soga_hair_color2.Red = result2.GetUInt8(1);
				c.soga_hair_color2.Green = result2.GetUInt8(2);
				c.soga_hair_color2.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_hair_highlight") {
				// ??? - not in the struct?
				//c.soga_hair_highlight.Red = result2.GetUInt8(1);
				//c.soga_hair_highlight.Green = result2.GetUInt8(2);
				//c.soga_hair_highlight.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_hair_type_color") {
				c.soga_hair_type_color.Red = result2.GetUInt8(1);
				c.soga_hair_type_color.Green = result2.GetUInt8(2);
				c.soga_hair_type_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_hair_type_highlight_color") {
				c.soga_hair_type_highlight_color.Red = result2.GetUInt8(1);
				c.soga_hair_type_highlight_color.Green = result2.GetUInt8(2);
				c.soga_hair_type_highlight_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_hair_face_color") {
				c.soga_hair_face_color.Red = result2.GetUInt8(1);
				c.soga_hair_face_color.Green = result2.GetUInt8(2);
				c.soga_hair_face_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_hair_face_highlight_color") {
				c.soga_hair_face_highlight_color.Red = result2.GetUInt8(1);
				c.soga_hair_face_highlight_color.Green = result2.GetUInt8(2);
				c.soga_hair_face_highlight_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_wing_color1") {
				c.wing_color1.Red = result2.GetUInt8(1);
				c.wing_color1.Green = result2.GetUInt8(2);
				c.wing_color1.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_wing_color2") {
				c.wing_color2.Red = result2.GetUInt8(1);
				c.wing_color2.Green = result2.GetUInt8(2);
				c.wing_color2.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_shirt_color") {
				// ??? - not in the struct?
				//c.soga_shirt_color.Red = result2.GetUInt8(1);
				//c.soga_shirt_color.Green = result2.GetUInt8(2);
				//c.soga_shirt_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_unknown_chest_color") {
				// ??? - not in the struct?
				//c.soga_unknown_chest_color.Red = result2.GetUInt8(1);
				//c.soga_unknown_chest_color.Green = result2.GetUInt8(2);
				//c.soga_unknown_chest_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_pants_color") {
				// ??? - not in the struct?
				//c.soga_pants_color.Red = result2.GetUInt8(1);
				//c.soga_pants_color.Green = result2.GetUInt8(2);
				//c.soga_pants_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_unknown_legs_color") {
				// ??? - not in the struct?
				//c.soga_unknown_legs_color.Red = result2.GetUInt8(1);
				//c.soga_unknown_legs_color.Green = result2.GetUInt8(2);
				//c.soga_unknown_legs_color.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_unknown13") {
				// ??? - not in the struct?
				//c.soga_unknown13.Red = result2.GetUInt8(1);
				//c.soga_unknown13.Green = result2.GetUInt8(2);
				//c.soga_unknown13.Blue = result2.GetUInt8(3);
			}
			else if (type == "soga_eye_type") {
				c.soga_eye_type[0] = result2.GetInt8(1);
				c.soga_eye_type[1] = result2.GetInt8(2);
				c.soga_eye_type[2] = result2.GetInt8(3);
			}
			else if (type == "soga_ear_type") {
				c.soga_ear_type[0] = result2.GetInt8(1);
				c.soga_ear_type[1] = result2.GetInt8(2);
				c.soga_ear_type[2] = result2.GetInt8(3);
			}
			else if (type == "soga_eye_brow_type") {
				c.soga_eye_brow_type[0] = result2.GetInt8(1);
				c.soga_eye_brow_type[1] = result2.GetInt8(2);
				c.soga_eye_brow_type[2] = result2.GetInt8(3);
			}
			else if (type == "soga_cheek_type") {
				c.soga_cheek_type[0] = result2.GetInt8(1);
				c.soga_cheek_type[1] = result2.GetInt8(2);
				c.soga_cheek_type[2] = result2.GetInt8(3);
			}
			else if (type == "soga_lip_type") {
				c.soga_lip_type[0] = result2.GetInt8(1);
				c.soga_lip_type[1] = result2.GetInt8(2);
				c.soga_lip_type[2] = result2.GetInt8(3);
			}
			else if (type == "soga_chin_type") {
				c.soga_chin_type[0] = result2.GetInt8(1);
				c.soga_chin_type[1] = result2.GetInt8(2);
				c.soga_chin_type[2] = result2.GetInt8(3);
			}
			else if (type == "soga_nose_type") {
				c.soga_nose_type[0] = result2.GetInt8(1);
				c.soga_nose_type[1] = result2.GetInt8(2);
				c.soga_nose_type[2] = result2.GetInt8(3);
			}
		}

		// TODO equipment
		ret = Select(&result2, "SELECT ci.slot, ia.equip_type, ia.red, ia.green, ia.blue, ia.highlight_red, ia.highlight_green, ia.highlight_blue FROM character_items ci INNER JOIN item_appearances ia ON ci.item_id = ia.item_id WHERE ci.type = 'EQUIPPED' AND ci.char_id = %u ORDER BY ci.slot ASC", c.charid);
		if (!ret)
			return ret;

		while (result2.Next()) {
			uint8_t slot = result2.GetUInt8(0);
			c.equip[slot].type = result2.GetUInt16(1);
			c.equip[slot].color.Red = result2.GetUInt8(2);
			c.equip[slot].color.Green = result2.GetUInt8(3);
			c.equip[slot].color.Blue = result2.GetUInt8(4);
			c.equip[slot].highlight.Red = result2.GetUInt8(5);
			c.equip[slot].highlight.Green = result2.GetUInt8(6);
			c.equip[slot].highlight.Blue = result2.GetUInt8(7);
		}
			
		c.server_name = "Rewrite Test Server";
		
		packet->CharacterList.push_back(c);
	}

	packet->NumCharacters = (uint8_t)packet->CharacterList.size();

	return ret;
}

bool WorldDatabase::DeleteCharacter(uint32_t account_id, uint32_t char_id, std::string name) {
	bool success;
	success = Query("UPDATE characters SET deleted = 1 WHERE id = %u AND account_id = %u AND name = '%s'", char_id, account_id, name.c_str());
	if (success && AffectedRows() == 1)
		return true;
	else
		return false;
}

bool WorldDatabase::SaveClientLog(std::string type, char* message, uint16_t version) {
	char* type_esc = Escape(type.c_str());
	char* message_esc = Escape(message);
	bool ret = Query("INSERT INTO log_messages (log_type, message, client_data_version, log_time) VALUES ('%s', '%s', %u, %u)", type_esc, message_esc, version, Timer::GetUnixTimeStamp());
	free(type_esc);
	free(message_esc);
	return ret;
}

uint32_t WorldDatabase::CreateCharacter(uint32_t account_id, OP_CreateCharacterRequestMsg_Packet* packet) {
	bool success;

	std::string create_char = std::string("INSERT INTO characters (account_id, server_id, name, race, class, gender, deity, body_size, body_age, soga_wing_type, soga_chest_type, soga_legs_type, soga_hair_type, soga_model_type, legs_type, chest_type, wing_type, hair_type, model_type, facial_hair_type, soga_facial_hair_type, created_date, last_saved, admin_status) VALUES (%i, %i, '%s', %i, %i, %i, %i, %f, %f, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, now(), unix_timestamp(), %i)");
	success = Query(create_char.c_str(),
		account_id,
		packet->server_id,
		packet->name.c_str(),
		packet->race,
		packet->_class,
		packet->gender,
		packet->deity,
		packet->body_size,
		packet->body_age,
		GetAppearanceID(packet->soga_wing_file),
		GetAppearanceID(packet->soga_chest_file),
		GetAppearanceID(packet->soga_legs_file),
		GetAppearanceID(packet->soga_hair_file),
		GetAppearanceID(packet->soga_race_file),
		GetAppearanceID(packet->legs_file),
		GetAppearanceID(packet->chest_file),
		GetAppearanceID(packet->wing_file),
		GetAppearanceID(packet->hair_file),
		GetAppearanceID(packet->race_file),
		GetAppearanceID(packet->face_file),
		GetAppearanceID(packet->soga_race_file),
		0
		);

	if (!success) {
		LogError(LOG_DATABASE, 0, "Error in SaveCharacter query: %s", GetError());
		return 0;
	}

	uint32_t char_id = LastInsertID();
	UpdateStartingFactions(char_id, packet->starting_zone);
	UpdateStartingZone(char_id, packet->_class, packet->race, packet->starting_zone);
	// Starting here
	UpdateStartingItems(char_id, packet->_class, packet->race);
	UpdateStartingSkills(char_id, packet->_class, packet->race);
	UpdateStartingSpells(char_id, packet->_class, packet->race);
	UpdateStartingSkillbar(char_id, packet->_class, packet->race);
	UpdateStartingTitles(char_id, packet->_class, packet->race, packet->gender);
	InsertCharacterStats(char_id, packet->_class, packet->race);
	// and ending here these functions still need to be implemented

	//AddNewPlayerToServerGuild(loginID, char_id);

	SaveCharacterColors(char_id, "skin_color", packet->skin_color);
	SaveCharacterColors(char_id, "eye_color", packet->eye_color);
	SaveCharacterColors(char_id, "hair_color1", packet->hair_color1);
	SaveCharacterColors(char_id, "hair_color2", packet->hair_color2);
	SaveCharacterColors(char_id, "hair_highlight", packet->hair_highlight);
	SaveCharacterColors(char_id, "hair_type_color", packet->hair_type_color);
	SaveCharacterColors(char_id, "hair_type_highlight_color", packet->hair_type_highlight_color);
	SaveCharacterColors(char_id, "hair_face_color", packet->hair_face_color);
	SaveCharacterColors(char_id, "hair_face_highlight_color", packet->hair_face_highlight_color);
	SaveCharacterColors(char_id, "wing_color1", packet->wing_color1);
	SaveCharacterColors(char_id, "wing_color2", packet->wing_color2);
	SaveCharacterColors(char_id, "shirt_color", packet->shirt_color);
	SaveCharacterColors(char_id, "unknown_chest_color", packet->unknown_chest_color);
	SaveCharacterColors(char_id, "pants_color", packet->pants_color);
	SaveCharacterColors(char_id, "unknown_legs_color", packet->unknown_legs_color);
	SaveCharacterColors(char_id, "unknown9", packet->unknown9);
	SaveCharacterFloats(char_id, "eye_type", packet->eyes2[0], packet->eyes2[1], packet->eyes2[2]);
	SaveCharacterFloats(char_id, "ear_type", packet->ears[0], packet->ears[1], packet->ears[2]);
	SaveCharacterFloats(char_id, "eye_brow_type", packet->eye_brows[0], packet->eye_brows[1], packet->eye_brows[2]);
	SaveCharacterFloats(char_id, "cheek_type", packet->cheeks[0], packet->cheeks[1], packet->cheeks[2]);
	SaveCharacterFloats(char_id, "lip_type", packet->lips[0], packet->lips[1], packet->lips[2]);
	SaveCharacterFloats(char_id, "chin_type", packet->chin[0], packet->chin[1], packet->chin[2]);
	SaveCharacterFloats(char_id, "nose_type", packet->nose[0], packet->nose[1], packet->nose[2]);
	SaveCharacterFloats(char_id, "body_size", packet->body_size, 0, 0);

	SaveCharacterColors(char_id, "soga_skin_color", packet->soga_skin_color);
	SaveCharacterColors(char_id, "soga_eye_color", packet->soga_eye_color);
	SaveCharacterColors(char_id, "soga_hair_color1", packet->soga_hair_color1);
	SaveCharacterColors(char_id, "soga_hair_color2", packet->soga_hair_color2);
	SaveCharacterColors(char_id, "soga_hair_highlight", packet->soga_hair_highlight);
	SaveCharacterColors(char_id, "soga_hair_type_color", packet->soga_hair_type_color);
	SaveCharacterColors(char_id, "soga_hair_type_highlight_color", packet->soga_hair_type_highlight_color);
	SaveCharacterColors(char_id, "soga_hair_face_color", packet->soga_hair_face_color);
	SaveCharacterColors(char_id, "soga_hair_face_highlight_color", packet->soga_hair_face_highlight_color);
	SaveCharacterColors(char_id, "soga_wing_color1", packet->soga_wing_color1);
	SaveCharacterColors(char_id, "soga_wing_color2", packet->soga_wing_color2);
	SaveCharacterColors(char_id, "soga_shirt_color", packet->soga_shirt_color);
	SaveCharacterColors(char_id, "soga_unknown_chest_color", packet->soga_unknown_chest_color);
	SaveCharacterColors(char_id, "soga_pants_color", packet->soga_pants_color);
	SaveCharacterColors(char_id, "soga_unknown_legs_color", packet->soga_unknown_legs_color);
	//SaveCharacterColors(char_id, "soga_unknown13", packet->soga_unknown13);
	SaveCharacterFloats(char_id, "soga_eye_type", packet->soga_eyes2[0], packet->soga_eyes2[1], packet->soga_eyes2[2]);
	SaveCharacterFloats(char_id, "soga_ear_type", packet->soga_ears[0], packet->soga_ears[1], packet->soga_ears[2]);
	SaveCharacterFloats(char_id, "soga_eye_brow_type", packet->soga_eye_brows[0], packet->soga_eye_brows[1], packet->soga_eye_brows[2]);
	SaveCharacterFloats(char_id, "soga_cheek_type", packet->soga_cheeks[0], packet->soga_cheeks[1], packet->soga_cheeks[2]);
	SaveCharacterFloats(char_id, "soga_lip_type", packet->soga_lips[0], packet->soga_lips[1], packet->soga_lips[2]);
	SaveCharacterFloats(char_id, "soga_chin_type", packet->soga_chin[0], packet->soga_chin[1], packet->soga_chin[2]);
	SaveCharacterFloats(char_id, "soga_nose_type", packet->soga_nose[0], packet->soga_nose[1], packet->soga_nose[2]);

	return char_id;
}

uint16_t WorldDatabase::GetAppearanceID(std::string name) {
	uint16_t id = 0;
	DatabaseResult result;
	bool success;
	char* name_esc = Escape(name.c_str());

	success = Select(&result, "SELECT appearance_id FROM appearances WHERE name = '%s'", name_esc);
	if (success && result.Next())
		id = result.GetUInt16(0);

	free(name_esc);
	return id;
}

void WorldDatabase::UpdateStartingFactions(uint32_t char_id, uint8_t choice) {
	Query("INSERT INTO character_factions (char_id, faction_id, faction_level) SELECT %u, faction_id, value FROM starting_factions WHERE starting_city = %u", char_id, choice);
}

void WorldDatabase::UpdateStartingZone(uint32_t char_id, uint8_t class_id, uint8_t race_id, uint8_t choice) {
	DatabaseResult result;
	bool success;

	
	//LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default zone for race: %i, class: %i for char_id: %u (choice: %i)", race_id, class_id, char_id, choice);

	// first, check to see if there is a starting_zones record for this race/class/choice combo (now using extended Archetype/BaseClass/Class combos
	success = Select(&result, "SELECT `name` FROM starting_zones sz, zones z WHERE sz.zone_id = z.id AND class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255) AND choice IN (%i, 255)",
		classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id, choice);

	// TODO: verify client version so clients do not crash trying to enter zones they do not own (paks)
	if (success && result.Next()) {
		string zone_name = result.GetString(0);

		Query("UPDATE characters c, zones z, starting_zones sz SET c.current_zone_id = z.id, c.x = z.safe_x, c.y = z.safe_y, c.z = z.safe_z, c.starting_city = %i WHERE z.id = sz.zone_id AND sz.class_id IN (%i, %i, %i, 255) AND sz.race_id IN (%i, 255) AND sz.choice IN (%i, 255) AND c.id = %u",
			choice, classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id, choice, char_id);

		if (GetErrno() && GetError() && GetErrno() < 0xFFFFFFFF) {
			//LogWrite(PLAYER__ERROR, 0, "Player", "Error in UpdateStartingZone custom starting_zones, query: '%s': %s", query.GetQuery(), query.GetError());
			return;
		}

		if (AffectedRows() > 0) {
			//LogWrite(PLAYER__DEBUG, 0, "Player", "Setting New Character Starting Zone to '%s' FROM starting_zones table.", zone_name.c_str());
			return;
		}
	}
	else {
		// there was no matching starting_zone value, so use default 'choice' starting city
		Query("UPDATE characters c, zones z SET c.current_zone_id = z.id, c.x = z.safe_x, c.y = z.safe_y, c.z = z.safe_z, c.starting_city = %i WHERE z.start_zone = %i and c.id = %u",
			choice, choice, char_id);

		if (GetErrno() && GetError() && GetErrno() < 0xFFFFFFFF) {
			//LogWrite(PLAYER__ERROR, 0, "Player", "Error in UpdateStartingZone player choice, query: '%s': %s", query.GetQuery(), query.GetError());
			return;
		}

		if (AffectedRows() > 0) {
			//LogWrite(PLAYER__DEBUG, 0, "Player", "Setting New Character Starting Zone to '%s' FROM player choice.", GetStartingZoneName(choice).c_str());
			return;
		}
	}

	// if we are here, it's a bad thing. zone tables have no start_city values to match client 'choice', so throw the player into zone according to R_World::DefaultStartingZoneID rule.
	// shout a few warnings so the admin fixes this asap!
	uint16_t default_zone_id = 1; // rule_manager.GetGlobalRule(R_World, DefaultStartingZoneID)->GetInt16();

	//LogWrite(WORLD__WARNING, 0, "World", "No Starting City defined for player choice: %i! BAD! BAD! BAD! Defaulting player to zone %i.", choice, default_zone_id);

	Query("UPDATE characters c, zones z SET c.current_zone_id = z.id, c.x = z.safe_x, c.y = z.safe_y, c.z = z.safe_z, c.heading = z.safe_heading, c.starting_city = 1 WHERE z.id = %i and c.id = %u", default_zone_id, char_id);

	if (GetErrno() && GetError() && GetErrno() < 0xFFFFFFFF) {
		//LogWrite(PLAYER__ERROR, 0, "Player", "Error in UpdateStartingZone default zone %i, query: '%s': %s", default_zone_id, query.GetQuery(), query.GetError());
		return;
	}

	if (AffectedRows() > 0) {
		//LogWrite(PLAYER__DEBUG, 0, "Player", "Setting New Character Starting Zone to '%s' due to no valid options!", GetZoneName(1)->c_str());
	}

	return;
}

void WorldDatabase::SaveCharacterColors(uint32_t char_id, const char* type, EQ2ColorFloat color) {
	uint8_t red = (uint8_t)(color.Red * 255);
	uint8_t green = (uint8_t)(color.Green * 255);
	uint8_t blue = (uint8_t)(color.Blue * 255);
	if (!Query("INSERT INTO char_colors (char_id, type, red, green, blue) VALUES (%u, '%s', %u, %u, %u)", char_id, type, red, green, blue)) {
		//LogWrite(WORLD__ERROR, 0, "World", "Error in SaveCharacterColors query: %s", GetError());
	}
}

void WorldDatabase::SaveCharacterFloats(uint32_t char_id, const char* type, float float1, float float2, float float3) {
	if (!Query("INSERT INTO char_colors (char_id, type, red, green, blue, signed_value) VALUES (%u, '%s', %i, %i, %i, 1)", char_id, type, (int8_t)(float1 * 100), (int8_t)(float2 * 100), (int8_t)(float3 * 100))) {
		//LogWrite(WORLD__ERROR, 0, "World", "Error in SaveCharacterFloats query: %s", GetError());
	}
}

void WorldDatabase::UpdateStartingItems(uint32_t char_id, uint8_t class_id, uint8_t race_id, bool base_class) {
	bool success;
	DatabaseResult result;

	struct StartingItem {
		std::string type;
		uint32_t id;
		std::string creator;
		uint8_t condition;
		uint8_t attuned;
		uint8_t count;
	};

	//success = Select(&result, "SELECT `type`, item_id, creator, condition_, attuned, count FROM starting_items WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255) ORDER BY id", classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id);

	/*
	SELECT si.`type`, si.item_id, si.creator, si.condition_, si.attuned, si.`count`, i.item_type FROM starting_items si
	INNER JOIN items i
	ON si.item_id = i.id
	WHERE si.class_id IN (%u, %u, %u, 255) AND si.race_id IN (%u, 255) ORDER BY si.id
	*/

	/*
	//LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default items for race: %i, class: %i for char_id: %u", race_id, class_id, char_id);
	Query query;
	Query query2;
	vector<Item*> items;
	vector<Item*> bags;
	map<int32, int8> total_slots;
	map<int32, int8> slots_left;
	map<int8, bool> equip_slots;
	map<Item*, StartingItem> item_list;
	int32 item_id = 0;
	Item* item = 0;
	StartingItem* starting_item = 0;
	//first get a list of the starting items for the character
	MYSQL_RES* result = 0;
	
	result = query2.RunQuery2(Q_SELECT, "SELECT `type`, item_id, creator, condition_, attuned, count FROM starting_items WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255) ORDER BY id", classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id);
	if (result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		while (result && (row = mysql_fetch_row(result))) {
			item_id = atoul(row[1]);
			item = master_item_list.GetItem(item_id);
			if (item) {
				starting_item = &(item_list[item]);
				starting_item->type = (row[0]) ? string(row[0]) : "";
				starting_item->item_id = atoul(row[1]);
				starting_item->creator = (row[2]) ? string(row[2]) : "";
				starting_item->condition = atoi(row[3]);
				starting_item->attuned = atoi(row[4]);
				starting_item->count = atoi(row[5]);
				item = master_item_list.GetItem(starting_item->item_id);
				if (item) {
					if (bags.size() < NUM_INV_SLOTS && item->IsBag() && item->details.num_slots > 0)
						bags.push_back(item);
					else
						items.push_back(item);
				}
			}
		}
	}
	slots_left[0] = NUM_INV_SLOTS;
	//next create the bags in the inventory
	for (int8 i = 0; i < bags.size(); i++) {
		item = bags[i];
		query.RunQuery2(Q_INSERT, "insert into character_items (char_id, type, slot, item_id, creator, condition_, attuned, bag_id, count) values(%u, '%s', %i, %u, '%s', %i, %i, %u, %i)",
			char_id, item_list[item].type.c_str(), i, item_list[item].item_id, getSafeEscapeString(item_list[item].creator.c_str()).c_str(), item_list[item].condition, item_list[item].attuned, 0, item_list[item].count);
		slots_left[query.GetLastInsertedID()] = item->details.num_slots;
		total_slots[query.GetLastInsertedID()] = item->details.num_slots;
		slots_left[0]--;
	}
	map<int32, int8>::iterator itr;
	int32 inv_slot = 0;
	int8  slot = 0;
	//finally process the rest of the items, placing them in the first available slot
	for (int32 x = 0; x < items.size(); x++) {
		item = items[x];
		if (item_list[item].type.find("NOT") < 0xFFFFFFFF) { // NOT-EQUIPPED Items
			for (itr = slots_left.begin(); itr != slots_left.end(); itr++) {
				if (itr->second > 0) {
					if (itr->first == 0 && slots_left.size() > 1) //we want items to go into bags first, then inventory after bags are full
						continue;
					inv_slot = itr->first;
					slot = total_slots[itr->first] - itr->second;
					itr->second--;
					if (itr->second == 0)
						slots_left.erase(itr);
					break;
				}
			}
			query.RunQuery2(Q_INSERT, "insert into character_items (char_id, type, slot, item_id, creator, condition_, attuned, bag_id, count) values(%u, '%s', %i, %u, '%s', %i, %i, %u, %i)",
				char_id, item_list[item].type.c_str(), slot, item_list[item].item_id, getSafeEscapeString(item_list[item].creator.c_str()).c_str(), item_list[item].condition, item_list[item].attuned, inv_slot, item_list[item].count);
		}
		else { //EQUIPPED Items
			for (int8 i = 0; i < item->slot_data.size(); i++) {
				if (equip_slots.count(item->slot_data[i]) == 0) {
					equip_slots[item->slot_data[i]] = true;
					query.RunQuery2(Q_INSERT, "insert into character_items (char_id, type, slot, item_id, creator, condition_, attuned, bag_id, count) values(%u, '%s', %i, %u, '%s', %i, %i, %u, %i)",
						char_id, item_list[item].type.c_str(), item->slot_data[i], item_list[item].item_id, getSafeEscapeString(item_list[item].creator.c_str()).c_str(), item_list[item].condition, item_list[item].attuned, 0, item_list[item].count);
					break;
				}
			}
		}
	}
	*/
}

void WorldDatabase::UpdateStartingSkills(uint32_t char_id, uint8_t class_id, uint8_t race_id) {
	//LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default skills for race: %i, class: %i for char_id: %u", race_id, class_id, char_id);
	Query("INSERT IGNORE INTO character_skills (char_id, skill_id, current_val, max_val) SELECT %u, skill_id, current_val, max_val FROM starting_skills WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255)",
		char_id, classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id);
}

void WorldDatabase::UpdateStartingSpells(uint32_t char_id, uint8_t class_id, uint8_t race_id) {
	//LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default spells for race: %i, class: %i for char_id: %u", race_id, class_id, char_id);
	Query("INSERT IGNORE INTO character_spells (char_id, spell_id, tier, knowledge_slot) SELECT %u, spell_id, tier, knowledge_slot FROM starting_spells WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255)",
		char_id, classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id);
}

void WorldDatabase::UpdateStartingSkillbar(uint32_t char_id, uint8_t class_id, uint8_t race_id) {
	//LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default skillbar for race: %i, class: %i for char_id: %u", race_id, class_id, char_id);
	Query("INSERT IGNORE INTO character_skillbar (char_id, type, hotbar, spell_id, slot, text_val) SELECT %u, type, hotbar, spell_id, slot, text_val FROM starting_skillbar WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255)",
		char_id, classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id);
}

void WorldDatabase::UpdateStartingTitles(uint32_t char_id, uint8_t class_id, uint8_t race_id, uint8_t gender_id) {
	//LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default titles for race: %i, class: %i, gender: %i for char_id: %u", race_id, class_id, gender_id, char_id);
	Query("INSERT IGNORE INTO character_titles (char_id, title_id) SELECT %u, title_id FROM  starting_titles WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255) and gender_id IN (%i, 255)",
		char_id, classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id, gender_id);
}

bool WorldDatabase::InsertCharacterStats(uint32_t character_id, uint8_t class_id, uint8_t race_id) {
	
	/* Blank record */
	Query("INSERT INTO `character_details` (`char_id`) VALUES (%u)", character_id);

	/* Using the class id and race id */
	Query("UPDATE character_details c, starting_details s SET c.max_hp = s.max_hp, c.hp = s.max_hp, c.max_power = s.max_power, c.power = s.max_power, c.str = s.str, c.sta = s.sta, c.agi = s.agi, c.wis = s.wis, c.intel = s.intel,c.heat = s.heat, c.cold = s.cold, c.magic = s.magic, c.mental = s.mental, c.divine = s.divine, c.disease = s.disease, c.poison = s.poison, c.coin_copper = s.coin_copper, c.coin_silver = s.coin_silver, c.coin_gold = s.coin_gold, c.coin_plat = s.coin_plat, c.status_points = s.status_points WHERE s.race_id = %d AND class_id = %d AND char_id = %u", race_id, class_id, character_id);
	if (AffectedRows() > 0)
		return true;

	/* Using the class id and race id = 255 */
	Query("UPDATE character_details c, starting_details s SET c.max_hp = s.max_hp, c.hp = s.max_hp, c.max_power = s.max_power, c.power = s.max_power, c.str = s.str, c.sta = s.sta, c.agi = s.agi, c.wis = s.wis, c.intel = s.intel,c.heat = s.heat, c.cold = s.cold, c.magic = s.magic, c.mental = s.mental, c.divine = s.divine, c.disease = s.disease, c.poison = s.poison, c.coin_copper = s.coin_copper, c.coin_silver = s.coin_silver, c.coin_gold = s.coin_gold, c.coin_plat = s.coin_plat, c.status_points = s.status_points WHERE s.race_id = 255 AND class_id = %d AND char_id = %u", class_id, character_id);
	if (AffectedRows() > 0)
		return true;

	/* Using class id = 255 and the race id */
	Query("UPDATE character_details c, starting_details s SET c.max_hp = s.max_hp, c.hp = s.max_hp, c.max_power = s.max_power, c.power = s.max_power, c.str = s.str, c.sta = s.sta, c.agi = s.agi, c.wis = s.wis, c.intel = s.intel,c.heat = s.heat, c.cold = s.cold, c.magic = s.magic, c.mental = s.mental, c.divine = s.divine, c.disease = s.disease, c.poison = s.poison, c.coin_copper = s.coin_copper, c.coin_silver = s.coin_silver, c.coin_gold = s.coin_gold, c.coin_plat = s.coin_plat, c.status_points = s.status_points WHERE s.race_id = %d AND class_id = 255 AND char_id = %u", race_id, character_id);
	if (AffectedRows() > 0)
		return true;

	/* Using class id = 255 and race id = 255 */
	Query("UPDATE character_details c, starting_details s SET c.max_hp = s.max_hp, c.hp = s.max_hp, c.max_power = s.max_power, c.power = s.max_power, c.str = s.str, c.sta = s.sta, c.agi = s.agi, c.wis = s.wis, c.intel = s.intel,c.heat = s.heat, c.cold = s.cold, c.magic = s.magic, c.mental = s.mental, c.divine = s.divine, c.disease = s.disease, c.poison = s.poison, c.coin_copper = s.coin_copper, c.coin_silver = s.coin_silver, c.coin_gold = s.coin_gold, c.coin_plat = s.coin_plat, c.status_points = s.status_points WHERE s.race_id = 255 AND class_id = 255 AND char_id = %u", character_id);
	if (AffectedRows() > 0)
		return true;

	return false;
}

uint8_t WorldDatabase::CheckNameFilter(const char* name) {
	// the minimum 4 is enforced by the client too
	if (!name || strlen(name) < 4 || strlen(name) > 15) // Even 20 char length is long...
		return BADNAMELENGTH_REPLY;
	unsigned char* checkname = (unsigned char*)name;
	for (uint8_t i = 0; i < strlen(name); i++) {
		if (!alpha_check(checkname[i]))
			return NAMEINVALID_REPLY;
	}

	DatabaseResult result;
	bool success;

	//LogWrite(WORLD__DEBUG, 0, "World", "Name check on: %s", name);
	success = Select(&result, "SELECT count(*) FROM characters WHERE name='%s'", name);
	if (success && result.Next()) {
		if (result.GetUInt32(0) > 0)
			return NAMETAKEN_REPLY;
	}
	else {
		//LogWrite(WORLD__ERROR, 0, "World", "Error in CheckNameFilter (name exist check) (Name query '%s': %s", query.GetQuery(), query.GetError());
	}

	//LogWrite(WORLD__DEBUG, 0, "World", "Name check on: %s (Bots table)", name);
	success = Select(&result, "SELECT count(*) FROM bots WHERE name='%s'", name);
	if (success && result.Next()) {
		if (result.GetUInt32(0) > 0)
			return NAMETAKEN_REPLY;
	}
	else {
		//LogWrite(WORLD__ERROR, 0, "World", "Error in CheckNameFilter (name exist check, bot table) (Name query '%s': %s", query3.GetQuery(), query3.GetError());
	}

	
	success = Select(&result, "SELECT count(*) FROM name_filter WHERE '%s' like name", name);
	if (success && result.Next()) {
		if (result.GetUInt32(0) > 0)
			return NAMEFILTER_REPLY;
		else
			return CREATESUCCESS_REPLY;
	}
	else {
		//LogWrite(WORLD__ERROR, 0, "World", "Error in CheckNameFilter (name_filter check) query '%s': %s", query.GetQuery(), query.GetError());
	}

	return UNKNOWNERROR_REPLY;
}