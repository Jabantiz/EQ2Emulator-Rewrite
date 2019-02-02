#pragma once

#include "../EQ2Packet.h"
#include "../PacketElements/PacketElements.h"
#include "OpcodeManager.h"

class OP_LoginReplyMsg_Packet : public EQ2Packet {
public:
	OP_LoginReplyMsg_Packet(uint16_t version)
		: EQ2Packet(version) {
		RegisterElements();

	}

	uint8_t Response;
	std::string Unknown;
	uint8_t ParentalControlFlag;
	uint32_t ParentalControlTimer;
	uint8_t Unknown2;					//<Data ElementName = "unknown2" Type = "int8" Size = "8" / >
	uint32_t AccountID;
	std::string Unknown3;
	uint8_t ResetAppearance;
	uint8_t DoNotForceSoga;
	uint8_t Unknown4;
	uint16_t Unknown5;
	uint8_t Unknown6;					//<Data ElementName = "unknown6" Type = "int8" Size = "5" / >
	uint32_t Unknown7;
	uint8_t RaceUnknown;
	uint8_t Unknown8;					//<Data ElementName = "unknown8" Type = "int8" Size = "3" / > < !--possibly related to rave_unknown but can't confirm-->
	uint8_t Unknown9;
	uint8_t Unknown10;
	uint8_t NumClassItems;				//<Data ElementName = "num_class_items" Type = "int8" IfVariableSet = "unknown10" Size = "1" / >
	struct ClassItem : public PacketSubstruct {
		uint8_t ClassID;
		struct StartingItem : public PacketSubstruct {
			uint16_t ModelID;
			uint8_t SlotID;
			uint8_t UseColor;
			uint8_t UseHighlightColor;
			EQ2Color ModelColor;
			EQ2Color ModelHighlightColor;

			StartingItem() {
				RegisterUInt16(ModelID);
				RegisterUInt8(SlotID);
				RegisterUInt8(UseColor);
				RegisterUInt8(UseHighlightColor);
				RegisterEQ2Color(ModelColor);
				RegisterEQ2Color(ModelHighlightColor);
			}
		};
		std::vector<StartingItem> StartingItems;

		ClassItem() {
			RegisterUInt8(ClassID);
			RegisterArray(StartingItems, StartingItem);
		}
	};
	std::vector<ClassItem> ClassItems;
	uint8_t UnknownArraySize;
	struct UnknownArray : public PacketSubstruct {
		uint32_t Array2Unknown;

		UnknownArray() {
			RegisterElements();
		}

		void RegisterElements() {
			RegisterUInt32(Array2Unknown);
		}
	};
	std::vector<UnknownArray> UnknownArray2;
	uint32_t Unknown11;
	uint32_t SubscriptionLevel;
	uint32_t RaceFlag;
	uint32_t ClassFlag;
	std::string Password;
	std::string Username;

private:
	void RegisterElements() {
		RegisterUInt8(Response);
		Register16String(Unknown);
		RegisterUInt8(ParentalControlFlag);
		RegisterUInt32(ParentalControlTimer);
		RegisterUInt8(Unknown2)->SetCount(8);
		RegisterUInt32(AccountID);
		Register16String(Unknown3);
		RegisterUInt8(ResetAppearance);
		RegisterUInt8(DoNotForceSoga);
		RegisterUInt8(Unknown4);
		RegisterUInt16(Unknown5);
		RegisterUInt8(Unknown6)->SetCount(5);
		RegisterUInt32(Unknown7);
		RegisterUInt8(RaceUnknown);
		RegisterUInt8(Unknown8)->SetCount(3);					//<Data ElementName = "unknown8" Type = "int8" Size = "3" / > < !--possibly related to rave_unknown but can't confirm-->
		RegisterUInt8(Unknown9);
		RegisterUInt8(Unknown10);
		RegisterUInt8(NumClassItems)->SetMyArray(RegisterArray(ClassItems, ClassItem));
		RegisterUInt8(UnknownArraySize)->SetMyArray(RegisterArray(UnknownArray2, UnknownArray));
		RegisterUInt32(Unknown11);
		RegisterUInt32(SubscriptionLevel);
		RegisterUInt32(RaceFlag);
		RegisterUInt32(ClassFlag);
		Register16String(Password);
		Register16String(Username);
	}
};

RegisterWorldStruct("OP_LoginReplyMsg", OP_LoginReplyMsg_Packet);

/*
<Struct Name = "LS_LoginReplyMsg" ClientVersion = "1" OpcodeName = "OP_LoginReplyMsg">
<Data ElementName = "login_response" Type = "int8" Size = "1" / >
<Data ElementName = "unknown" Type = "EQ2_16Bit_String" Size = "1" / >
<Data ElementName = "parental_control_flag" Type = "int8" Size = "1" / >
<Data ElementName = "parental_control_timer" Type = "int32" Size = "1" / >
<Data ElementName = "unknown2" Type = "int8" Size = "8" / >
<Data ElementName = "account_id" Type = "int32" Size = "1" / >
<Data ElementName = "unknown3" Type = "EQ2_16Bit_String" Size = "1" / >
<Data ElementName = "reset_appearance" Type = "int8" Size = "1" / >
<Data ElementName = "do_not_force_soga" Type = "int8" Size = "1" / >
<Data ElementName = "unknown5" Type = "int16" Size = "1" / >
<Data ElementName = "unknown6" Type = "int8" Size = "1" / >
<Data ElementName = "unknown7" Type = "int32" Size = "1" / >
<Data ElementName = "unknown8" Type = "int8" Size = "2" / >
<Data ElementName = "unknown10" Type = "int8" Size = "1" / >
<Data ElementName = "num_class_items" Type = "int8" IfVariableSet = "unknown10" Size = "1" / >
<Data ElementName = "class_items" Type = "Array" ArraySizeVariable = "num_class_items" IfVariableSet = "unknown10" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
		<Data ElementName = "model_id" Type = "int16" / >
		<Data ElementName = "slot_id" Type = "int8" / >
		<Data ElementName = "use_color" Type = "int8" / >
		<Data ElementName = "use_highlight_color" Type = "int8" / >
		<Data ElementName = "model_color" Type = "EQ2_Color" / >
		<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
< / Data>
<Data ElementName = "unknown_array2_size" Type = "int8" Size = "1" / >
<Data ElementName = "unknown_array2" Type = "Array" ArraySizeVariable = "unknown_array2_size" >
	<Data ElementName = "array2_unknown" Type = "int32" Size = "1" / >
< / Data>
< / Struct>

<Struct Name = "LS_LoginReplyMsg" ClientVersion = "843" OpcodeName = "OP_LoginReplyMsg">
<Data ElementName = "login_response" Type = "int8" Size = "1" / >
<Data ElementName = "unknown" Type = "EQ2_16Bit_String" Size = "1" / >
<Data ElementName = "parental_control_flag" Type = "int8" Size = "1" / >
<Data ElementName = "parental_control_timer" Type = "int32" Size = "1" / >
<Data ElementName = "unknown2" Type = "int8" Size = "8" / >
<Data ElementName = "account_id" Type = "int32" Size = "1" / >
<Data ElementName = "unknown3" Type = "EQ2_16Bit_String" Size = "1" / >
<Data ElementName = "reset_appearance" Type = "int8" Size = "1" / >
<Data ElementName = "do_not_force_soga" Type = "int8" Size = "1" / >
<Data ElementName = "unknown4" Type = "int8" Size = "1" / >
<Data ElementName = "unknown5" Type = "int16" Size = "1" / >
<Data ElementName = "unknown6" Type = "int8" Size = "1" / >
<Data ElementName = "unknown7" Type = "int32" Size = "1" / >
<Data ElementName = "race_unknown" Type = "int8" Size = "1" / >
<Data ElementName = "unknown8" Type = "int8" Size = "3" / >
<Data ElementName = "unknown9" Type = "int8" Size = "1" / >
<Data ElementName = "unknown10" Type = "int8" Size = "1" / >
<Data ElementName = "num_class_items" Type = "int8" IfVariableSet = "unknown10" Size = "1" / >
<Data ElementName = "class_items" Type = "Array" ArraySizeVariable = "num_class_items" IfVariableSet = "unknown10" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
		<Data ElementName = "model_id" Type = "int16" / >
		<Data ElementName = "slot_id" Type = "int8" / >
		<Data ElementName = "use_color" Type = "int8" / >
		<Data ElementName = "use_highlight_color" Type = "int8" / >
		<Data ElementName = "model_color" Type = "EQ2_Color" / >
		<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
< / Data>
<Data ElementName = "unknown_array2_size" Type = "int8" Size = "1" / >
<Data ElementName = "unknown_array2" Type = "Array" ArraySizeVariable = "unknown_array2_size" >
	<Data ElementName = "array2_unknown" Type = "int32" Size = "1" / >
< / Data>
< / Struct>

<Struct Name = "LS_LoginReplyMsg" ClientVersion = "1096" OpcodeName = "OP_LoginReplyMsg">
<Data ElementName = "login_response" Type = "int8" Size = "1" / >
<Data ElementName = "unknown" Type = "EQ2_16Bit_String" Size = "1" / > <!--possibly parental control password-->
<Data ElementName = "parental_control_flag" Type = "int8" Size = "1" / >
<Data ElementName = "parental_control_timer" Type = "int32" Size = "1" / >
<Data ElementName = "unknown2" Type = "int8" Size = "8" / >
<Data ElementName = "account_id" Type = "int32" Size = "1" / >
<Data ElementName = "unknown3" Type = "EQ2_16Bit_String" Size = "1" / > <!--assume this is a string, could be an array though-->
<Data ElementName = "reset_appearance" Type = "int8" Size = "1" / >
<Data ElementName = "do_not_force_soga" Type = "int8" Size = "1" / >
<Data ElementName = "unknown4" Type = "int8" Size = "1" / >
<Data ElementName = "unknown5" Type = "int16" Size = "1" / >
<Data ElementName = "unknown6" Type = "int8" Size = "5" / >
<Data ElementName = "unknown7" Type = "int32" Size = "1" / > <!--expansion info maybe-->
<Data ElementName = "race_unknown" Type = "int8" Size = "1" / >
<Data ElementName = "unknown8" Type = "int8" Size = "3" / > < !--possibly related to rave_unknown but can't confirm-->
<Data ElementName = "unknown9" Type = "int8" Size = "1" / > <!--screws with models, possibly related to that skeletal revamp that was never finished-->
<Data ElementName = "unknown10" Type = "int8" Size = "1" / >
<Data ElementName = "num_class_items" Type = "int8" IfVariableSet = "unknown10" Size = "1" / >
<Data ElementName = "class_items" Type = "Array" ArraySizeVariable = "num_class_items" IfVariableSet = "unknown10" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
		<Data ElementName = "model_id" Type = "int16" / >
		<Data ElementName = "slot_id" Type = "int8" / >
		<Data ElementName = "use_color" Type = "int8" / >
		<Data ElementName = "use_highlight_color" Type = "int8" / >
		<Data ElementName = "model_color" Type = "EQ2_Color" / >
		<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
< / Data>
<Data ElementName = "unknown_array2_size" Type = "int8" Size = "1" / >
<Data ElementName = "unknown_array2" Type = "Array" ArraySizeVariable = "unknown_array2_size" >
	<Data ElementName = "array2_unknown" Type = "int32" Size = "1" / >
< / Data>
<Data ElementName = "unknown11" Type = "int32" Size = "1" / >
<Data ElementName = "sub_level" Type = "int32" Size = "1" / >
<Data ElementName = "race_flag" Type = "int32" Size = "1" / >
<Data ElementName = "class_flag" Type = "int32" Size = "1" / >
<!--Both of the following are encrypted, password is correct guessed that the second is username, neither need to be set-->
<Data ElementName = "password" Type = "EQ2_16Bit_String" Size = "1" / >
<Data ElementName = "username" Type = "EQ2_16bit_String" Size = "1" / >
< / Struct>


	<Struct Name = "LS_LoginReplyMsg" ClientVersion = "1142" OpcodeName = "OP_LoginReplyMsg">
	<Data ElementName = "login_response" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "parental_control_flag" Type = "int8" Size = "1" / >
	<Data ElementName = "parental_control_timer" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown2" Type = "int8" Size = "8" / >
	<Data ElementName = "account_id" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown3" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "reset_appearance" Type = "int8" Size = "1" / >
	<Data ElementName = "do_not_force_soga" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown4" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown5" Type = "int16" Size = "1" / >
	<Data ElementName = "unknown6" Type = "int8" Size = "5" / >
	<Data ElementName = "unknown7" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown7a" Type = "int32" Size = "1" / >
	<Data ElementName = "race_unknown" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown8" Type = "int8" Size = "3" / >
	<Data ElementName = "unknown9" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown10" Type = "int8" Size = "1" / >
	<Data ElementName = "num_class_items" Type = "int8" IfVariableSet = "unknown10" Size = "1" / >
	<Data ElementName = "class_items" Type = "Array" ArraySizeVariable = "num_class_items" IfVariableSet = "unknown10" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
	<Data ElementName = "model_id" Type = "int16" / >
	<Data ElementName = "slot_id" Type = "int8" / >
	<Data ElementName = "use_color" Type = "int8" / >
	<Data ElementName = "use_highlight_color" Type = "int8" / >
	<Data ElementName = "model_color" Type = "EQ2_Color" / >
	<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
	< / Data>
	<Data ElementName = "unknown_array2_size" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown_array2" Type = "Array" ArraySizeVariable = "unknown_array2_size" >
	<Data ElementName = "array2_unknown" Type = "int32" Size = "1" / >
	< / Data>
	<Data ElementName = "unknown11" Type = "int32" Size = "1" / >
	<Data ElementName = "sub_level" Type = "int32" Size = "1" / >
	<Data ElementName = "race_flag" Type = "int32" Size = "1" / >
	<Data ElementName = "class_flag" Type = "int32" Size = "1" / >
	<Data ElementName = "password" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "username" Type = "EQ2_16bit_String" Size = "1" / >
	< / Struct>
	<Struct Name = "LS_LoginReplyMsg" ClientVersion = "1188" OpcodeName = "OP_LoginReplyMsg">
	<Data ElementName = "login_response" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "parental_control_flag" Type = "int8" Size = "1" / >
	<Data ElementName = "parental_control_timer" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown2" Type = "int8" Size = "8" / >
	<Data ElementName = "account_id" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown3" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "reset_appearance" Type = "int8" Size = "1" / >
	<Data ElementName = "do_not_force_soga" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown4" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown5" Type = "int16" Size = "1" / >
	<Data ElementName = "unknown6" Type = "int8" Size = "5" / >
	<Data ElementName = "unknown7" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown7a" Type = "int32" Size = "1" / >
	<Data ElementName = "race_unknown" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown8" Type = "int8" Size = "3" / >
	<Data ElementName = "unknown9" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown10" Type = "int8" Size = "1" / >
	<Data ElementName = "num_class_items" Type = "int8" IfVariableSet = "unknown10" Size = "1" / >
	<Data ElementName = "class_items" Type = "Array" ArraySizeVariable = "num_class_items" IfVariableSet = "unknown10" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
	<Data ElementName = "model_id" Type = "int16" / >
	<Data ElementName = "slot_id" Type = "int8" / >
	<Data ElementName = "use_color" Type = "int8" / >
	<Data ElementName = "use_highlight_color" Type = "int8" / >
	<Data ElementName = "model_color" Type = "EQ2_Color" / >
	<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
	< / Data>
	<Data ElementName = "unknown_array2_size" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown_array2" Type = "Array" ArraySizeVariable = "unknown_array2_size" >
	<Data ElementName = "array2_unknown" Type = "int32" Size = "1" / >
	< / Data>
	<Data ElementName = "unknown11" Type = "int32" Size = "1" / >
	<Data ElementName = "sub_level" Type = "int32" Size = "1" / >
	<Data ElementName = "race_flag" Type = "int32" Size = "1" / >
	<Data ElementName = "class_flag" Type = "int32" Size = "1" / >
	<Data ElementName = "password" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "username" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "unknown12" Type = "EQ2_16bit_String" Size = "1" / >
	< / Struct>
	<Struct Name = "LS_LoginReplyMsg" ClientVersion = "57080" OpcodeName = "OP_LoginReplyMsg">
	<Data ElementName = "login_response" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "parental_control_flag" Type = "int8" Size = "1" / >
	<Data ElementName = "parental_control_timer" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown2" Type = "int8" Size = "8" / >
	<Data ElementName = "account_id" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown3" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "reset_appearance" Type = "int8" Size = "1" / >
	<Data ElementName = "do_not_force_soga" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown4" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown5" Type = "int16" Size = "1" / >
	<Data ElementName = "unknown6" Type = "int8" Size = "5" / >
	<Data ElementName = "unknown7" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown7a" Type = "int32" Size = "1" / >
	<Data ElementName = "race_unknown" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown8" Type = "int8" Size = "3" / >
	<Data ElementName = "unknown9" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown10" Type = "int8" Size = "1" / >
	<Data ElementName = "num_class_items" Type = "int8" IfVariableSet = "unknown10" Size = "1" / >
	<Data ElementName = "class_items" Type = "Array" ArraySizeVariable = "num_class_items" IfVariableSet = "unknown10" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
	<Data ElementName = "model_id" Type = "int32" / >
	<Data ElementName = "slot_id" Type = "int8" / >
	<Data ElementName = "use_color" Type = "int8" / >
	<Data ElementName = "use_highlight_color" Type = "int8" / >
	<Data ElementName = "model_color" Type = "EQ2_Color" / >
	<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
	< / Data>
	<Data ElementName = "unknown_array2_size" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown_array2" Type = "Array" ArraySizeVariable = "unknown_array2_size" >
	<Data ElementName = "array2_unknown" Type = "int32" Size = "1" / >
	< / Data>
	<Data ElementName = "unknown11" Type = "int32" Size = "1" / >
	<Data ElementName = "sub_level" Type = "int32" Size = "1" / >
	<Data ElementName = "race_flag" Type = "int32" Size = "1" / >
	<Data ElementName = "class_flag" Type = "int32" Size = "1" / >
	<Data ElementName = "password" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "username" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "service" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "web1" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "web2" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "web3" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "web4" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "web5" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "web6" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "web7" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "web8" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "web9" Type = "EQ2_16bit_String" Size = "1" / >
	<!--Seems to be a repeat of the first array-->
	<Data ElementName = "unknown12" Type = "int8" Size = "1" / >
	<Data ElementName = "lvl90_num_class_items" Type = "int8" IfVariableSet = "unknown10" Size = "1" / >
	<Data ElementName = "lvl90_class_items" Type = "Array" ArraySizeVariable = "lvl90_num_class_items" IfVariableSet = "unknown10" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
	<Data ElementName = "model_id" Type = "int32" / >
	<Data ElementName = "slot_id" Type = "int8" / >
	<Data ElementName = "use_color" Type = "int8" / >
	<Data ElementName = "use_highlight_color" Type = "int8" / >
	<Data ElementName = "model_color" Type = "EQ2_Color" / >
	<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
	< / Data>
	<Data ElementName = "unknown13" Type = "int8" Size = "5" / >
	< / Struct>
	<Struct Name = "LS_LoginReplyMsg" ClientVersion = "60100" OpcodeName = "OP_LoginReplyMsg">
	<Data ElementName = "login_response" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "parental_control_flag" Type = "int8" Size = "1" / >
	<Data ElementName = "parental_control_timer" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown2" Type = "int8" Size = "8" / >
	<Data ElementName = "account_id" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown3" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "reset_appearance" Type = "int8" Size = "1" / >
	<Data ElementName = "do_not_force_soga" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown4" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown5" Type = "int16" Size = "1" / >
	<Data ElementName = "unknown6" Type = "int8" Size = "5" / >
	<Data ElementName = "unknown7" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown7a" Type = "int16" Size = "1" / >
	<Data ElementName = "race_unknown" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown8" Type = "int8" Size = "3" / >
	<Data ElementName = "unknown9" Type = "int8" Size = "3" / >
	<Data ElementName = "unknown10" Type = "int8" Size = "1" / >
	<Data ElementName = "num_class_items" Type = "int8" IfVariableSet = "unknown10" Size = "1" / >
	<Data ElementName = "class_items" Type = "Array" ArraySizeVariable = "num_class_items" IfVariableSet = "unknown10" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
	<Data ElementName = "model_id" Type = "int32" / >
	<Data ElementName = "slot_id" Type = "int8" / >
	<Data ElementName = "use_color" Type = "int8" / >
	<Data ElementName = "use_highlight_color" Type = "int8" / >
	<Data ElementName = "model_color" Type = "EQ2_Color" / >
	<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
	< / Data>
	<Data ElementName = "unknown_array2_size" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown_array2" Type = "Array" ArraySizeVariable = "unknown_array2_size" >
	<Data ElementName = "array2_unknown" Type = "int32" Size = "1" / >
	< / Data>
	<Data ElementName = "unknown11" Type = "int32" Size = "1" / >
	<Data ElementName = "sub_level" Type = "int32" Size = "1" / >
	<Data ElementName = "race_flag" Type = "int32" Size = "1" / >
	<Data ElementName = "class_flag" Type = "int32" Size = "1" / >
	<Data ElementName = "password" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "username" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "service" Type = "EQ2_16bit_String" Size = "1" / >
	<!--Seems to be a repeat of the first array-->
	<Data ElementName = "unknown12" Type = "int8" Size = "1" / >
	<Data ElementName = "lvl90_num_class_items" Type = "int8" IfVariableSet = "unknown12" Size = "1" / >
	<Data ElementName = "lvl90_class_items" Type = "Array" ArraySizeVariable = "lvl90_num_class_items" IfVariableSet = "unknown12" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
	<Data ElementName = "model_id" Type = "int32" / >
	<Data ElementName = "slot_id" Type = "int8" / >
	<Data ElementName = "use_color" Type = "int8" / >
	<Data ElementName = "use_highlight_color" Type = "int8" / >
	<Data ElementName = "model_color" Type = "EQ2_Color" / >
	<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
	< / Data>
	<Data ElementName = "unknown13" Type = "int8" Size = "1" / >
	<Data ElementName = "time_locked_num_class_items" Type = "int8" IfVariableSet = "unknown13" Size = "1" / >
	<Data ElementName = "time_locked_class_items" Type = "Array" ArraySizeVariable = "time_locked_num_class_items" IfVariableSet = "unknown13" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
	<Data ElementName = "model_id" Type = "int32" / >
	<Data ElementName = "slot_id" Type = "int8" / >
	<Data ElementName = "use_color" Type = "int8" / >
	<Data ElementName = "use_highlight_color" Type = "int8" / >
	<Data ElementName = "model_color" Type = "EQ2_Color" / >
	<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
	< / Data>
	<Data ElementName = "unknown14" Type = "int8" Size = "13" / >
	< / Struct>
	<Struct Name = "LS_LoginReplyMsg" ClientVersion = "63181" OpcodeName = "OP_LoginReplyMsg">
	<Data ElementName = "login_response" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "parental_control_flag" Type = "int8" Size = "1" / >
	<Data ElementName = "parental_control_timer" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown2" Type = "int8" Size = "8" / >
	<Data ElementName = "account_id" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown3" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "reset_appearance" Type = "int8" Size = "1" / >
	<Data ElementName = "do_not_force_soga" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown4" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown5" Type = "int16" Size = "1" / >
	<Data ElementName = "unknown6" Type = "int8" Size = "5" / >
	<Data ElementName = "unknown6a" Type = "int8" Size = "8" / >
	<Data ElementName = "unknown7" Type = "int32" Size = "1" / >
	<Data ElementName = "unknown7a" Type = "int16" Size = "1" / >
	<Data ElementName = "race_unknown" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown8" Type = "int8" Size = "3" / >
	<Data ElementName = "unknown9" Type = "int8" Size = "3" / >
	<Data ElementName = "unknown10" Type = "int8" Size = "1" / >
	<Data ElementName = "num_class_items" Type = "int8" IfVariableSet = "unknown10" Size = "1" / >
	<Data ElementName = "class_items" Type = "Array" ArraySizeVariable = "num_class_items" IfVariableSet = "unknown10" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
	<Data ElementName = "model_id" Type = "int32" / >
	<Data ElementName = "slot_id" Type = "int8" / >
	<Data ElementName = "use_color" Type = "int8" / >
	<Data ElementName = "use_highlight_color" Type = "int8" / >
	<Data ElementName = "model_color" Type = "EQ2_Color" / >
	<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
	< / Data>
	<Data ElementName = "unknown_array2_size" Type = "int8" Size = "1" / >
	<Data ElementName = "unknown_array2" Type = "Array" ArraySizeVariable = "unknown_array2_size" >
	<Data ElementName = "array2_unknown" Type = "int32" Size = "1" / >
	< / Data>
	<Data ElementName = "unknown11" Type = "int32" Size = "1" / >
	<Data ElementName = "sub_level" Type = "int32" Size = "1" / >
	<Data ElementName = "race_flag" Type = "int32" Size = "1" / >
	<Data ElementName = "class_flag" Type = "int32" Size = "1" / >
	<Data ElementName = "password" Type = "EQ2_16Bit_String" Size = "1" / >
	<Data ElementName = "username" Type = "EQ2_16bit_String" Size = "1" / >
	<Data ElementName = "service" Type = "EQ2_16bit_String" Size = "1" / >
	<!--Seems to be a repeat of the first array-->
	<Data ElementName = "unknown12" Type = "int8" Size = "1" / >
	<Data ElementName = "lvl90_num_class_items" Type = "int8" IfVariableSet = "unknown12" Size = "1" / >
	<Data ElementName = "lvl90_class_items" Type = "Array" ArraySizeVariable = "lvl90_num_class_items" IfVariableSet = "unknown12" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
	<Data ElementName = "model_id" Type = "int32" / >
	<Data ElementName = "slot_id" Type = "int8" / >
	<Data ElementName = "use_color" Type = "int8" / >
	<Data ElementName = "use_highlight_color" Type = "int8" / >
	<Data ElementName = "model_color" Type = "EQ2_Color" / >
	<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
	< / Data>
	<Data ElementName = "unknown13" Type = "int8" Size = "1" / >
	<Data ElementName = "time_locked_num_class_items" Type = "int8" IfVariableSet = "unknown13" Size = "1" / >
	<Data ElementName = "time_locked_class_items" Type = "Array" ArraySizeVariable = "time_locked_num_class_items" IfVariableSet = "unknown13" >
	<Data ElementName = "class_id" Type = "int8" Size = "1" / >
	<Data ElementName = "num_items" Type = "int8" Size = "1" / >
	<Data ElementName = "starting_items" Type = "Array" ArraySizeVariable = "num_items" >
	<Data ElementName = "model_id" Type = "int32" / >
	<Data ElementName = "slot_id" Type = "int8" / >
	<Data ElementName = "use_color" Type = "int8" / >
	<Data ElementName = "use_highlight_color" Type = "int8" / >
	<Data ElementName = "model_color" Type = "EQ2_Color" / >
	<Data ElementName = "model_highlight_color" Type = "EQ2_Color" / >
	< / Data>
	< / Data>
	<Data ElementName = "unknown14" Type = "int8" Size = "9" / >
	< / Struct>
		*/