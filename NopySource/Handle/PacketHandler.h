#pragma once
#ifdef _WIN32
#elif defined(__linux__)
#define BYTE unsigned char
#define DWORD unsigned int
#define __int64 int64_t
#define __int32 int32_t
#define __int16 int16_t
#define __int8 int8_t
#endif
#include <chrono>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "NopySource/Handle/PlayerInfo.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <windows.h>
#include <wininet.h>
#include <lmcons.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

std::time_t start_time = std::time(nullptr);
void TryAutoConvert(ENetPeer* peer);

namespace systems {

	size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
		return size * nmemb;
	}

} // namespace systems
using namespace chrono;
constexpr std::string_view COLOR_RESET = "\033[0m";
constexpr std::string_view COLOR_BLACK = "\033[30m";
constexpr std::string_view COLOR_RED = "\033[31m";
constexpr std::string_view COLOR_GREEN = "\033[32m";
constexpr std::string_view COLOR_YELLOW = "\033[33m";
constexpr std::string_view COLOR_BLUE = "\033[34m";
constexpr std::string_view COLOR_MAGENTA = "\033[35m";
constexpr std::string_view COLOR_CYAN = "\033[36m";
constexpr std::string_view COLOR_WHITE = "\033[37m";
#pragma pack(push, 1)
typedef struct gametankpacket_t {
	int32_t m_type;
	char m_data;
} gametextpacket_t;
#pragma pack(pop)
enum {
	PACKET_STATE = 0,
	PACKET_CALL_FUNCTION,
	PACKET_UPDATE_STATUS,
	PACKET_TILE_CHANGE_REQUEST,
	PACKET_SEND_MAP_DATA,
	PACKET_SEND_TILE_UPDATE_DATA,
	PACKET_SEND_TILE_UPDATE_DATA_MULTIPLE,
	PACKET_TILE_ACTIVATE_REQUEST,
	PACKET_TILE_APPLY_DAMAGE,
	PACKET_SEND_INVENTORY_STATE,
	PACKET_ITEM_ACTIVATE_REQUEST,
	PACKET_ITEM_ACTIVATE_OBJECT_REQUEST,
	PACKET_SEND_TILE_TREE_STATE,
	PACKET_MODIFY_ITEM_INVENTORY,
	PACKET_ITEM_CHANGE_OBJECT,
	PACKET_SEND_LOCK,
	PACKET_SEND_ITEM_DATABASE_DATA,
	PACKET_SEND_PARTICLE_EFFECT,
	PACKET_SET_ICON_STATE,
	PACKET_ITEM_EFFECT,
	PACKET_SET_CHARACTER_STATE,
	PACKET_PING_REPLY,
	PACKET_PING_REQUEST,
	PACKET_GOT_PUNCHED,
	PACKET_APP_CHECK_RESPONSE,
	PACKET_APP_INTEGRITY_FAIL,
	PACKET_DISCONNECT,
	PACKET_BATTLE_JOIN,
	PACKET_BATTLE_EVENT,
	PACKET_USE_DOOR,
	PACKET_SEND_PARENTAL,
	PACKET_GONE_FISHIN,
	PACKET_STEAM,
	PACKET_PET_BATTLE,
	PACKET_NPC,
	PACKET_SPECIAL,
	PACKET_SEND_PARTICLE_EFFECT_V2,
	PACKET_ACTIVE_ARROW_TO_ITEM,
	PACKET_SELECT_TILE_INDEX,
	PACKET_SEND_PLAYER_TRIBUTE_DATA,
	PACKET_PVE_UNK1,
	PACKET_PVE_UNK2,
	PACKET_PVE_UNK3,
	PACKET_PVE_UNK4,
	PACKET_PVE_UNK5,
	PACKET_SET_EXTRA_MODS,
	PACKET_ON_STEP_ON_TILE_MOD,
	PACKET_MAXVAL,
	PACKET_PROXY_CHECK_RESPONSE = 76
};
namespace GUP {
#pragma pack(push, 1)
	struct TankUpdatePacket {
		int32_t m_type;
		char* m_data;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct GameUpdatePacket {
		uint8_t m_type = 0; //1

		union
		{
			uint8_t m_object_type = 0; //2
			uint8_t m_punch_id;
			uint8_t m_npc_type;
		};

		union
		{
			uint8_t m_count1 = 0; //3
			uint8_t m_jump_count;
			uint8_t m_build_range;
			uint8_t m_npc_id;
			uint8_t m_lost_item_count;
		};

		union
		{
			uint8_t m_count2 = 0;
			uint8_t m_animation_type; //4
			uint8_t m_punch_range;
			uint8_t m_npc_action;
			uint8_t m_particle_id;
			uint8_t m_gained_item_count;
			uint8_t m_dice_result;
			uint8_t m_fruit_count;
		};

		union
		{
			int32_t m_net_id = 0; //8
			int32_t m_owner_id;
			int32_t m_effect_flags_check;
			int32_t m_object_change_type;
			int32_t m_particle_emitter_id;
		};

		union
		{
			int32_t m_item = 0; //12
			int32_t m_ping_hash;
			int32_t m_item_net_id;
			int32_t m_pupil_color;
			int32_t m_target_net_id;
			int32_t m_tiles_length;
		};

		int32_t m_flags = 0; //16

		union
		{
			float m_float_var = 0; //20
			float m_water_speed;
			float m_obj_alt_count;
		};

		union
		{
			int32_t m_int_data = 0; //24
			int32_t m_ping_item;
			int32_t m_elapsed_ms;
			int32_t m_delay;
			int32_t m_tile_damage;
			int32_t m_item_id;
			int32_t m_item_speed;
			int32_t m_effect_flags;
			int32_t m_object_id;
			int32_t m_hash;
			int32_t m_verify_pos;
			int32_t m_client_hack_type;
		};

		union
		{
			float m_vec_x = 0; //28
			float m_pos_x;
			float m_acceleration;
			float m_punch_range_in;
		};

		union
		{
			float m_vec_y = 0; //32
			float m_pos_y;
			float m_build_range_in;
			float m_punch_strength;
		};

		union
		{
			float m_vec2_x = 0; //36
			float m_dest_x;
			float m_gravity_in;
			float m_speed_out;
			float m_velocity_x;
			float m_particle_variable;
			float m_pos2_x;
			int m_hack_type;
		};

		union
		{
			float m_vec2_y = 0; //40
			float m_dest_y;
			float m_speed_in;
			float m_gravity_out;
			float m_velocity_y;
			float m_particle_alt_id;
			float m_pos2_y;
			int m_hack_type2;
		};

		union
		{
			float m_particle_rotation = 0; //44
			float m_npc_speed;
		};

		union
		{
			uint32_t m_int_x = 0; //48
			uint32_t m_tile_pos_x;
			uint32_t m_item_id_alt;
			uint32_t m_hair_color;
		};

		union
		{
			uint32_t m_int_y = 0; //52
			uint32_t m_tile_pos_y;
			uint32_t m_item_count;
			uint32_t m_eyes_color;
			uint32_t m_npc_state;
			uint32_t m_particle_size_alt;
		};

		uint32_t m_data_size = 0; //56
		uint8_t m_data;
	};
#pragma pack(pop)

	void send_packet(ENetPeer* peer, TankUpdatePacket tank_packet, uintmax_t data_size) {
		ENetPacket* packet = enet_packet_create(nullptr, data_size, ENET_PACKET_FLAG_RELIABLE);
		if (!packet)
			return;
		std::memcpy(packet->data, &tank_packet, data_size);
		if (enet_peer_send(peer, 0, packet) != 0)
			enet_packet_destroy(packet);
	}
	void send_packet(ENetPeer* peer, TankUpdatePacket* tank_packet, uintmax_t data_size) {
		GameUpdatePacket* update_packet = reinterpret_cast<GameUpdatePacket*>(&tank_packet->m_data);
		ENetPacket* packet = enet_packet_create(nullptr, data_size, ENET_PACKET_FLAG_RELIABLE);
		if (!packet || !update_packet)
			return;
		std::memcpy(packet->data, &tank_packet->m_type, 4);
		std::memcpy(packet->data + 4, update_packet, sizeof(GameUpdatePacket) + update_packet->m_data_size);

		if (enet_peer_send(peer, 0, packet) != 0)
			enet_packet_destroy(packet);
	}
	void send_packet(ENetPeer* peer, uint32_t type, const void* data, uintmax_t data_size, uint32_t flags)
	{
		ENetPacket* packet = enet_packet_create(nullptr, 5 + data_size, flags);
		if (!packet)
			return;

		std::memcpy(packet->data, &type, 4);
		packet->data[data_size + 4] = 0;

		if (data)
			std::memcpy(packet->data + 4, data, data_size);
		else
			std::cout << "Data is not valid\n";

		if (enet_peer_send(peer, 0, packet) != 0) {
			enet_packet_destroy(packet);
			std::cout << "Cannot send packet\n";
		}
	}
	GameUpdatePacket* DataToUpdatePacket(ENetPacket* packet) {
		if (packet->dataLength < sizeof(GameUpdatePacket) - 5)
			return nullptr;
		GameUpdatePacket* update_packet = reinterpret_cast<GameUpdatePacket*>(packet->data + 4);
		if (!(update_packet->m_flags & 8))
			return update_packet;
		if (packet->dataLength < update_packet->m_data_size + sizeof(GameUpdatePacket))
			return nullptr;
		return update_packet;
	}
}

void LoadExchangeData(std::unordered_map<int, int>& itemPrices) {
	std::string filePath = "database/json/price_exchange.json";
	if (!std::filesystem::exists(filePath)) {
		std::filesystem::create_directories("database/json");
		std::ofstream out(filePath);
		if (out.is_open()) {
			out << "{\n}\n";
			out.close();
		}
	}
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Gagal membuka file: " << filePath << "\n";
		return;
	}
	try {
		json j;
		file >> j;
		for (auto& [key, value] : j.items()) {
			if (!key.empty() && std::isdigit(static_cast<unsigned char>(key[0]))) {
				int id = std::stoi(key);
				itemPrices[id] = value.get<int>();
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error loading " << filePath << ": " << e.what() << "\n";
	}
}
int changeExchangeData(const std::string& id_str, int new_price) {
	const std::string filePath = "database/json/price_exchange.json";
	if (!std::filesystem::exists(filePath)) {
		std::filesystem::create_directories("database/json");
		std::ofstream out(filePath);
		if (out.is_open()) {
			out << "{\n}\n";
			out.close();
		}
	}
	std::ifstream file_in(filePath);
	if (!file_in) {
		std::cerr << "Failed open file JSON\n";
		return 1; 
	}
	json data;
	try {
		file_in >> data;
		file_in.close();
	}
	catch (...) {
		std::cerr << "Failed read/parse JSON\n";
		file_in.close();
		return 2; 
	}
	if (data.find(id_str) == data.end()) {
		std::cerr << "Item ID not Found on list\n";
		return 3;
	}
	data[id_str] = new_price;
	std::cout << "CHANGED PRICE ITEM ID " << id_str << " TO " << new_price << std::endl;
	std::ofstream file_out(filePath);
	if (!file_out) {
		std::cerr << "Failed writing file JSON\n";
		return 4; 
	}
	file_out << data.dump(4);
	file_out.close();
	return 0; 
}

class BinaryWriter2 {
public:
	BinaryWriter2() : m_data(nullptr), m_pos(0), m_size(0) {}

	~BinaryWriter2() {
		if (m_data && m_delete_after && !do_not_delete)
			std::free(m_data);
	}

	template <typename T>
	inline void write(const T& val) {
		ensureCapacity(sizeof(T));
		std::memcpy(m_data + m_pos, &val, sizeof(T));
		m_pos += sizeof(T);
	}

	inline void write(const std::string& val, const std::size_t data_length_size = 2) {
		std::size_t len = val.length();
		ensureCapacity(len + data_length_size);
		std::memcpy(m_data + m_pos, &len, data_length_size);
		std::memcpy(m_data + m_pos + data_length_size, val.c_str(), len);
		m_pos += len + data_length_size;
	}

	inline void write(const uint8_t* val, std::size_t len) {
		ensureCapacity(len);
		std::memcpy(m_data + m_pos, val, len);
		m_pos += len;
	}

	inline void set_pos(const std::size_t pos) {
		if (pos > m_size) throw std::out_of_range("Position out of range");
		m_pos = pos;
	}

	inline void skip_pos(std::size_t len) {
		std::size_t new_pos = m_pos + len;
		if (new_pos > m_size) throw std::out_of_range("Skipping out of buffer bounds");
		m_pos = new_pos;
	}

	inline void del_pos(std::size_t len) {
		if (len > m_pos) throw std::out_of_range("Deleting beyond start of buffer");
		m_pos -= len;
	}

	inline void clean() {
		if (m_data) {
			std::free(m_data);
			m_data = nullptr;
		}
		m_size = 0;
		m_pos = 0;
	}
	bool do_not_delete = false;

	uint8_t* get() {
		return m_data;
	}

	std::size_t get_pos() const {
		return m_pos;
	}

	std::size_t get_size() const {
		return m_size;
	}

private:
	uint8_t* m_data;
	std::size_t m_pos;
	std::size_t m_size;
	bool m_delete_after{ true };

	inline void ensureCapacity(std::size_t required) {
		std::size_t needed_size = m_pos + required;
		if (needed_size > m_size) {
			std::size_t new_size = std::max<std::size_t>(m_size * 2, needed_size);
			uint8_t* new_data = (uint8_t*)std::realloc(m_data, new_size);
			if (!new_data) throw std::bad_alloc();
			m_data = new_data;
			m_size = new_size;
		}
	}
};

class BinaryWriter
{
public:
	BinaryWriter(uint8_t* data, const std::size_t& pos = 0) : m_data(data) {
		this->m_pos = pos;
		this->m_delete_after = false;
	}
	BinaryWriter(const std::size_t& size) {
		this->m_pos = 0;
		this->m_size = size;

		m_data = (uint8_t*)std::malloc(size);
		std::memset(m_data, 0, size);
	}
	~BinaryWriter() {
		if (this->m_delete_after)
			std::free(m_data);
	}

	template<typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = true>
	void write(T val) {
		std::memcpy(m_data + m_pos, &val, sizeof(T));
		m_pos += sizeof(T);
	}
	template<typename T, typename std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
	void write(T val) {
		std::memcpy(m_data + m_pos, &val, sizeof(T));
		m_pos += sizeof(T);
	}
	void write(const std::string& val, const uint16_t& len_size = 2) {
		if (len_size == 2) {
			uint16_t len = (uint16_t)val.length();

			std::memcpy(m_data + m_pos, &len, len_size);
			std::memcpy(m_data + m_pos + len_size, val.c_str(), len);
			m_pos += len + len_size;
			return;
		}
		uint32_t len = (uint32_t)val.length();

		std::memcpy(m_data + m_pos, &len, len_size);
		std::memcpy(m_data + m_pos + len_size, val.c_str(), len);
		m_pos += len + len_size;
	}
	void write(const uint8_t* val, std::size_t len) {
		std::memcpy(m_data + m_pos, val, len);
		m_pos += len;
	}
	void write(const char* val, std::size_t len) {
		std::memcpy(m_data + m_pos, val, len);
		m_pos += len;
	}
	void set_pos(const std::size_t& pos) {
		this->m_pos = pos;
	}
	void skip_pos(size_t len) {
		this->m_pos += len;
	}

	[[nodiscard]] uint8_t* get() {
		return m_data;
	}
	[[nodiscard]] std::size_t get_pos() const {
		return m_pos;
	}
	[[nodiscard]] std::size_t get_size() const {
		return m_size;
	}
private:
	uint8_t* m_data;
	std::size_t m_pos;
	std::size_t m_size;

	bool m_delete_after{ true };
};
enum { NET_MESSAGE_UNKNOWN = 0, NET_MESSAGE_SERVER_HELLO, NET_MESSAGE_GENERIC_TEXT, NET_MESSAGE_GAME_MESSAGE, NET_MESSAGE_GAME_PACKET, NET_MESSAGE_ERROR, NET_MESSAGE_TRACK, NET_MESSAGE_CLIENT_LOG_REQUEST, NET_MESSAGE_CLIENT_LOG_RESPONSE, };
enum packet_flags : uint32_t { PACKET_FLAGS_FLYING = 1 << 1, PACKET_FLAGS_UPDATE = 1 << 2, PACKET_FLAGS_EXTENDED = 1 << 3, PACKET_FLAGS_FACINGLEFT = 1 << 4, PACKET_FLAGS_ONGROUND = 1 << 5, PACKET_FLAGS_LAVA = 1 << 6, PACKET_FLAGS_JUMPSOUND = 1 << 7, PACKET_FLAGS_DEATH = 1 << 8, PACKET_FLAGS_PUNCH = 1 << 9, PACKET_FLAGS_PLACE = 1 << 10, PACKET_FLAGS_TILEACTION = 1 << 11, PACKET_FLAGS_KNOCKBACK = 1 << 12, PACKET_FLAGS_RESPAWN = 1 << 13, PACKET_FLAGS_PICKUPSOUND = 1 << 14, PACKET_FLAGS_TRAMPOLINE = 1 << 15, PACKET_FLAGS_CACTUS = 1 << 16, PACKET_FLAGS_SLIDING = 1 << 17, PACKET_FLAGS_JUMPPEAK = 1 << 18, PACKET_FLAGS_FALLING_SLOWLY = 1 << 19, PACKET_FLAGS_SWIM_SOUND = 1 << 20, PACKET_FLAGS_WALLHANG = 1 << 21, PACKET_FLAGS_RAYMAN_START = 1 << 22, PACKET_FLAGS_RAYMAN_END = 1 << 23, PACKET_FLAGS_RAYMAN_LOAD = 1 << 24, PACKET_FLAGS_FORCE_RING = 1 << 25, PACKET_FLAGS_CACTUS_RAPE = 1 << 26, PACKET_FLAGS_ACID = 1 << 28 };
#pragma pack(push, 1)
struct GameUpdatePacketSimple {
	uint32_t data_size;
	int32_t netid, item, flags;
	int32_t int_data, int_x, int_y;
	uint8_t type, objtype, count1, count2;
	float float1, vec_x, vec_y, vec2_x, vec2_y, float2;
};
#pragma pack(pop)
struct gamepacket_t
{
private:
	int index = 0, len = 0;
	BYTE* packet_data = new BYTE[61];
public:
	gamepacket_t(int delay = 0, int NetID = -1) {
		len = 61;
		int MessageType = 0x4, PacketType = 0x1, CharState = 0x8;
		Memory_Set(packet_data, 0, 61);
		Memory_Copy(packet_data, &MessageType, 4);
		Memory_Copy(packet_data + 4, &PacketType, 4);
		Memory_Copy(packet_data + 8, &NetID, 4);
		Memory_Copy(packet_data + 16, &CharState, 4);
		Memory_Copy(packet_data + 24, &delay, 4);
	};
	~gamepacket_t() {
		delete[] packet_data;
	}
	void Insert(string a) {
		BYTE* data = new BYTE[len + 2 + a.length() + 4];
		Memory_Copy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x2;
		int str_len = (int)a.length();
		Memory_Copy(data + len + 2, &str_len, 4);
		Memory_Copy(data + len + 6, a.data(), str_len);
		len = len + 2 + (int)a.length() + 4;
		index++;
		if (len > 60) {
			packet_data[60] = index;
		}
		else {
			std::cerr << "Error: packet_data does not have enough space for the 61st element." << std::endl;
		}
	}
	void Insert(int a) {
		BYTE* data = new BYTE[len + 2 + 4];
		Memory_Copy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x9;
		Memory_Copy(data + len + 2, &a, 4);
		len = len + 2 + 4;
		index++;
		if (len > 60) {
			packet_data[60] = index;
		}
		else {
			std::cerr << "Error: packet_data does not have enough space for the 61st element." << std::endl;
		}
	}
	void Insert(unsigned int a) {
		BYTE* data = new BYTE[len + 2 + 4];
		Memory_Copy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x5;
		Memory_Copy(data + len + 2, &a, 4);
		len = len + 2 + 4;
		index++;
		if (len > 60) {
			packet_data[60] = index;
		}
		else {
			std::cerr << "Error: packet_data does not have enough space for the 61st element." << std::endl;
		}
	}
	void Insert(float a) {
		BYTE* data = new BYTE[len + 2 + 4];
		Memory_Copy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x1;
		Memory_Copy(data + len + 2, &a, 4);
		len = len + 2 + 4;
		index++;
		if (len > 60) {
			packet_data[60] = index;
		}
		else {
			std::cerr << "Error: packet_data does not have enough space for the 61st element." << std::endl;
		}
	}
	void Insert(float a, float b) {
		BYTE* data = new BYTE[len + 2 + 8];
		Memory_Copy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x3;
		Memory_Copy(data + len + 2, &a, 4);
		Memory_Copy(data + len + 6, &b, 4);
		len = len + 2 + 8;
		index++;
		if (len > 60) {
			packet_data[60] = index;
		}
		else {
			std::cerr << "Error: packet_data does not have enough space for the 61st element." << std::endl;
		}
	}
	void Insert(float a, float b, float c) {
		BYTE* data = new BYTE[len + 2 + 12];
		Memory_Copy(data, packet_data, len);
		delete[] packet_data;
		packet_data = data;
		data[len] = index;
		data[len + 1] = 0x4;
		Memory_Copy(data + len + 2, &a, 4);
		Memory_Copy(data + len + 6, &b, 4);
		Memory_Copy(data + len + 10, &c, 4);
		len = len + 2 + 12;
		index++;
		if (len > 60) {
			packet_data[60] = index;
		}
		else {
			std::cerr << "Error: packet_data does not have enough space for the 61st element." << std::endl;
		}
	}
	void CreatePacket(ENetPeer* peer) {
		ENetPacket* packet = enet_packet_create(packet_data, len, 1);
		if (enet_peer_send(peer, 0, packet) != 0) {
			enet_packet_destroy(packet);
		}
	}
};
inline PlayerMoving unpackPlayerMoving(BYTE* data) {
	PlayerMoving dataStruct;
	if (data != nullptr) {
		Memory_Copy(&dataStruct.packetType, data, 4);
		Memory_Copy(&dataStruct.netID, data + 4, 4);
		Memory_Copy(&dataStruct.effect_flags_check, data + 8, 4);  // m_target_net_id (attacker/target netID)
		Memory_Copy(&dataStruct.characterState, data + 12, 4);
		Memory_Copy(&dataStruct.plantingTree, data + 20, 4);
		Memory_Copy(&dataStruct.x, data + 24, 4);
		Memory_Copy(&dataStruct.y, data + 28, 4);
		Memory_Copy(&dataStruct.XSpeed, data + 32, 4);
		Memory_Copy(&dataStruct.YSpeed, data + 36, 4);
		Memory_Copy(&dataStruct.punchX, data + 44, 4);
		Memory_Copy(&dataStruct.punchY, data + 48, 4);
	}
	return dataStruct;
}
void SendPacketRaw112(int a1, void* packetData, size_t packetDataSize, void* a4, ENetPeer* peer, int packetFlag) {
	ENetPacket* p;
	if (peer) {
		if (a1 == 4 && *((BYTE*)packetData + 12) & 8) {
			p = enet_packet_create(0, packetDataSize + *((DWORD*)packetData + 13) + 5, packetFlag);
			int four = 4;
			memcpy(p->data, &four, 4);
			memcpy((char*)p->data + 4, packetData, packetDataSize);
			memcpy((char*)p->data + packetDataSize + 4, a4, *((DWORD*)packetData + 13));
			if (enet_peer_send(peer, 0, p) != 0) {
				enet_packet_destroy(p);
			}
		}
		else {
			p = enet_packet_create(0, packetDataSize + 5, packetFlag);
			memcpy(p->data, &a1, 4);
			memcpy((char*)p->data + 4, packetData, packetDataSize);
			if (enet_peer_send(peer, 0, p) != 0) {
				enet_packet_destroy(p);
			}
		}
	}
	delete[] (char*)packetData;
}


void SendPacketRaw1(int a1, void* packetData, size_t packetDataSize, void* a4, ENetPeer* peer, int packetFlag, int delay) {
	ENetPacket* p;
	if (peer) {
		if (a1 == 4 && *((BYTE*)packetData + 12) & 8) {
			p = enet_packet_create(0, packetDataSize + *((DWORD*)packetData + 13) + 5, packetFlag);
			int four = 4;
			memcpy(p->data, &four, 4);
			memcpy((char*)p->data + 4, packetData, packetDataSize);
			memcpy((char*)p->data + packetDataSize + 4, a4, *((DWORD*)packetData + 13));
			int deathFlag = 0x19;
			memcpy(p->data + 24, &delay, 4);
			memcpy(p->data + 56, &deathFlag, 4);
			if (enet_peer_send(peer, 0, p) != 0) {
				enet_packet_destroy(p);
			}
		}
		else {
			p = enet_packet_create(0, packetDataSize + 5, packetFlag);
			memcpy(p->data, &a1, 4);
			memcpy((char*)p->data + 4, packetData, packetDataSize);
			int deathFlag = 0x19;
			memcpy(p->data + 24, &delay, 4);
			memcpy(p->data + 56, &deathFlag, 4);
			if (enet_peer_send(peer, 0, p) != 0) {
				enet_packet_destroy(p);
			}
		}
	}
	delete[] (char*)packetData;
}
BYTE* GetStructPointerFromTankPacket(ENetPacket* packet) {
	const unsigned int packetLenght = (unsigned int)packet->dataLength;
	BYTE* result = NULL;
	if (packetLenght >= 0x3C) {
		BYTE* packetData = packet->data;
		result = packetData + 4;
		if (*static_cast<BYTE*>(packetData + 16) & 8) {
			if (packetLenght < *(int*)(packetData + 56) + 60) {
				cout << "[!] Packet too small for extended packet to be valid" << endl;
				cout << "[!] Sizeof float is 4.  TankUpdatePacket size: 56" << endl;
				result = 0;
			}
		}
		else {
			int zero = 0;
			Memory_Copy(packetData + 56, &zero, 4);
		}
	}
	return result;
}
std::string dialogrequest(ENetPeer* p) {
	pInfo(p)->isindialog = true;
	return "OnDialogRequest";
}
// FIX EXPLOIT: parse `end_dialog|<name>|...` dari text dialog yang dikirim server,
// lalu tambahkan name ke pending_dialogs dengan TTL. Action::dialog_return akan cek
// nama yang dikembalikan client harus ada di set ini.
//
// Format dialog Growtopia: baris dengan prefix `end_dialog|<dialog_name>|` (kadang
// juga `embed_data|dialog_name|<name>` untuk dialog tertentu). Kita parse keduanya.
inline void track_pending_dialog(ENetPeer* peer, const std::string& text) {
	if (!peer || !peer->data) return;
	const long long now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
	const long long ttl_ms = 60LL * 1000; // dialog valid maks 60 detik
	auto& pd = pInfo(peer)->pending_dialogs;

	// Garbage-collect dialog lama (cap maksimal 32 entry untuk hindari unbounded growth).
	for (auto it = pd.begin(); it != pd.end(); ) {
		if (it->second < now_ms) it = pd.erase(it); else ++it;
	}
	if (pd.size() > 64) pd.clear();

	auto add_name = [&](std::string name) {
		// trim
		while (!name.empty() && (name.back() == '\r' || name.back() == '\n' || name.back() == ' ')) name.pop_back();
		while (!name.empty() && (name.front() == ' ' || name.front() == '\t')) name.erase(name.begin());
		if (name.empty() || name.size() > 64) return;
		pd[name] = now_ms + ttl_ms;
	};

	// Iterate baris-per-baris cari "end_dialog|NAME|..." atau "embed_data|dialog_name|NAME".
	size_t pos = 0;
	while (pos < text.size()) {
		size_t nl = text.find('\n', pos);
		std::string line = (nl == std::string::npos) ? text.substr(pos) : text.substr(pos, nl - pos);
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (line.rfind("end_dialog|", 0) == 0) {
			size_t p1 = 11; // after "end_dialog|"
			size_t p2 = line.find('|', p1);
			if (p2 == std::string::npos) p2 = line.size();
			add_name(line.substr(p1, p2 - p1));
		} else if (line.rfind("embed_data|dialog_name|", 0) == 0) {
			add_name(line.substr(23));
		}
		if (nl == std::string::npos) break;
		pos = nl + 1;
	}
}
inline bool consume_pending_dialog(ENetPeer* peer, const std::string& name) {
	if (!peer || !peer->data) return false;
	auto& pd = pInfo(peer)->pending_dialogs;
	auto it = pd.find(name);
	if (it == pd.end()) return false;
	const long long now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
	if (it->second < now_ms) { pd.erase(it); return false; }
	// Dialog terverifikasi. Tidak kita hapus secara default (beberapa dialog bisa
	// di-respon berkali-kali, mis. saat scroll/refresh) tapi expire otomatis via TTL.
	return true;
}
std::string SetColor(ENetPeer* peer) {
	if (pInfo(peer)->border_color == "Default" or pInfo(peer)->border_color.empty()) return "";
	else return "set_border_color|255,255,255,255|" + pInfo(peer)->border_color + "\nset_bg_color|" + pInfo(peer)->bg_color + "";
}
inline void SendParticleEffect(ENetPeer* peer, int x, int y, int size, int id, int delay) {
	PlayerMoving datx{};
	datx.packetType = 0x11;
	datx.x = x;
	datx.y = y;
	datx.YSpeed = id;
	datx.XSpeed = size;
	datx.plantingTree = delay;
	BYTE* raw = packPlayerMoving(&datx);
	send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
	delete[]raw;
}
void Add_Piggy_Bank(ENetPeer* peer, int amount) {
	if (pInfo(peer)->pg_bank) {
		if (pInfo(peer)->Banked_Piggy < 350000) {
			pInfo(peer)->Banked_Piggy += amount;
			if (pInfo(peer)->Banked_Piggy >= 350000) pInfo(peer)->Banked_Piggy = 350000;
			gamepacket_t p;
			p.Insert("OnEventButtonDataSet");
			p.Insert("PiggyBankButton");
			p.Insert(1);
			p.Insert("{\"active\":false,\"buttonAction\":\"openPiggyBank\",\"buttonState\":" + a + (pInfo(peer)->Banked_Piggy > 350000 ? "2" : "0") + ",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":0,\"name\":\"PiggyBankButton\",\"notification\":0,\"order\":2,\"rcssClass\":\"piggybank\",\"text\":\"" + (pInfo(peer)->Banked_Piggy > 350000 ? "350K" : ConvertToK(pInfo(peer)->Banked_Piggy) + "/350K") + "\"}");
			p.CreatePacket(peer);
		}
	}
}
bool complete_gpass_task(ENetPeer* peer, string task) {
	if (find(pInfo(peer)->growpass_quests.begin(), pInfo(peer)->growpass_quests.end(), task) == pInfo(peer)->growpass_quests.end()) {
		pInfo(peer)->growpass_quests.push_back(task);
		Add_Piggy_Bank(peer, 10000);
		int get_points = 10;
		if (task == "Growtoken") get_points = 40;
		else if (task == "Claim 4,000 gems") get_points = 150, pInfo(peer)->gems += 4000;
		else get_points = 10;
		if (pInfo(peer)->growpass_points < 5400) {
			string text = "`9Completed Grow Pass Task '" + task + "' and received " + to_string(get_points) + " points!``";
			if (not pInfo(peer)->world.empty()) {
				gamepacket_t p;
				p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert(text), p.Insert(0), p.Insert(0), p.CreatePacket(peer);
				PlayerMoving data_{};
				data_.packetType = 17, data_.netID = 198, data_.YSpeed = 198, data_.x = pInfo(peer)->x + 16, data_.y = pInfo(peer)->y + 16;
				BYTE* raw = packPlayerMoving(&data_);
				send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
				delete[] raw;
			}
			gamepacket_t p1;
			p1.Insert("OnConsoleMessage"), p1.Insert(text), p1.CreatePacket(peer);
			pInfo(peer)->growpass_points += get_points;
			return true;
		}
		else return false;
	}
	else return false;
}
class CAction {
public:
	static void Effect(ENetPeer* peer, int id, int x, int y, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnParticleEffect"), p.Insert(id), p.Insert(x, y), p.CreatePacket(peer);
	}
	static void Effect_V2(ENetPeer* peer, int id, int x, int y, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnParticleEffectV2"), p.Insert(id), p.Insert(x, y), p.CreatePacket(peer);
	}
	static void Positioned(ENetPeer* peer, int netID, string file, int delay = 0) {
		gamepacket_t p(delay, netID);
		p.Insert("OnPlayPositioned");
		p.Insert(file);
		p.CreatePacket(peer);
	}
	static void ScreenShotMode(ENetPeer* peer) {
		gamepacket_t p;
		p.Insert("OnPlayerScreenShotMode");
		p.CreatePacket(peer);
	}
	static void Log(ENetPeer* p_, string t_, string l_ = "", string w_ = "google.com") {
		if (l_ != "") t_ = "action|log\nmsg|" + t_;
		int y_ = 3;
		BYTE z_ = 0;
		BYTE* const d_ = new BYTE[5 + t_.length()];
		Memory_Copy(d_, &y_, 4);
		Memory_Copy(d_ + 4, t_.c_str(), t_.length());
		Memory_Copy(d_ + 4 + t_.length(), &z_, 1);
		ENetPacket* const p = enet_packet_create(d_, 5 + t_.length(), ENET_PACKET_FLAG_RELIABLE);
		if (enet_peer_send(p_, 0, p) != 0) {
			enet_packet_destroy(p);
		}
		delete[]d_;
		if (l_ != "") {
			l_ = "action|set_url\nurl|" + w_ + "\nlabel|" + l_ + "\n";
			BYTE* const u_ = new BYTE[5 + l_.length()];
			Memory_Copy(u_, &y_, 4);
			Memory_Copy(u_ + 4, l_.c_str(), l_.length());
			Memory_Copy(u_ + 4 + l_.length(), &z_, 1);
			ENetPacket* const p3 = enet_packet_create(u_, 5 + l_.length(), ENET_PACKET_FLAG_RELIABLE);
			if (enet_peer_send(p_, 0, p3) != 0) {
				enet_packet_destroy(p3);
			}
			delete[]u_;
		}
	}
};
class Logger {
public:
	static void	Info(string type, string text) {
		struct tm newtime;
		time_t now = time(0);
#ifdef _WIN32
		localtime_s(&newtime, &now);
#elif defined(__linux__)
		localtime_r(&now, &newtime);
#endif
		if (type != "LOGIN" && type != "DISCONNECT" && type != "INFO" && type != "ERROR") return;

		std::string_view color = COLOR_GREEN;
		if (type == "LOGIN") color = COLOR_CYAN;
		else if (type == "INFO") color = COLOR_GREEN;
		else if (type == "DISCONNECT" || type == "ERROR") color = COLOR_RED;

		char time_str[16];
		snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", newtime.tm_hour, newtime.tm_min, newtime.tm_sec);

		std::cout << COLOR_RESET << "[" << time_str << "] " << color << " [" << type << "]: " << color << text << COLOR_RESET << std::endl;
	}
};
class VarList {
public:
	static void OnConsoleMessage(ENetPeer* peer, string text, bool all = false, int dly = 0) {
		gamepacket_t p(dly);
		p.Insert("OnConsoleMessage");
		p.Insert("`o" + text);
		if (!all) p.CreatePacket(peer);
		else {
			for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
				p.CreatePacket(currentPeer);
			}
		}
	}
	static void OnCountdownUpdate(ENetPeer* peer, int netID, int score) {
		gamepacket_t p(-1, netID);
		p.Insert("OnCountdownUpdate"), p.Insert(score);
		p.CreatePacket(peer);
	}
	static void OnCountdownStart(ENetPeer* peer, int netID, int time, int score) {
		gamepacket_t p(0, netID);
		if (score == -1) {
			p.Insert("OnCountdownStart"), p.Insert(time);
		}
		else {
			p.Insert("OnCountdownStart"), p.Insert(time), p.Insert(score);
		}
		p.CreatePacket(peer);
	}
	static void OnSuperMainStartAcceptLogon(ENetPeer* peer, string link, string path, string proto_) {
		gamepacket_t p;
		p.Insert("OnSuperMainStartAcceptLogonHrdxs47254722215a");
		p.Insert(Environment()->ItemsDat_Hash);
		p.Insert(link);
		p.Insert(path);
		p.Insert("cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster");
		p.Insert("proto=" + proto_ + "|choosemusic=audio/ogg/theme_lobby.ogg|active_holiday=0|wing_week_day=0|ubi_week_day=0|server_tick=23802433|clash_active=1|drop_lavacheck_faster=1|isPayingUser=" + a + (pInfo(peer)->supp == 1 ? "1" : pInfo(peer)->supp == 2 ? "2" : "0") + "|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1|seed_diary_hash=" + to_string(Environment()->ItemsDat_Hash) + "|m_clientBits=" + (Environment()->Enable_Joystick ? "3072" : "1024") + "|eventButtons={\"EventButtonData\":["
			"{\"active\":false,\"buttonAction\":\"dailychallengemenu\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":23,\"name\":\"DailyChallenge\",\"order\":0,\"rcssClass\":\"daily_challenge\",\"text\":\"\"},"
			"{\"active\":false,\"buttonAction\":\"showdungeonsui\",\"buttonTemplate\":\"DungeonEventButton\",\"counter\":0,\"counterMax\":20,\"name\":\"ScrollsPurchaseButton\",\"order\":2,\"rcssClass\":\"scrollbank\",\"text\":\"\"},"
			"{\"active\":false,\"buttonAction\":\"winter_bingo_ui\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"name\":\"BingoButton\",\"order\":2,\"rcssClass\":\"wf-bingo\",\"text\":\"\"},"
			"{\"active\":true,\"buttonAction\":\"toprichmenu\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":14406,\"name\":\"TopRichButton\",\"order\":30,\"rcssClass\":\"clash-event\",\"text\":\"Top\\nRich\"},"
			"{\"active\":true,\"buttonAction\":\"open_marketplace\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":13812,\"name\":\"MarketplaceButton\",\"order\":29,\"rcssClass\":\"clash-event\",\"text\":\"Buy/Sell\\nMarket\"},"
			"{\"active\":true,\"buttonAction\":\"open_autoptht\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":9726,\"name\":\"AutoPthtButton\",\"order\":28,\"rcssClass\":\"clash-event\",\"text\":\"Farm\\nSettings\"},"
			"{\"active\":true,\"buttonAction\":\"exchange_fish\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":3000,\"name\":\"SellFishButton\",\"order\":27,\"rcssClass\":\"clash-event\",\"text\":\"Fish\\nExchange\"},"
			"{\"active\":true,\"buttonAction\":\"event_clash_rewards\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":12158,\"name\":\"ClashExchangeButton\",\"order\":26,\"rcssClass\":\"clash-event\",\"text\":\"Event\\nExchange\"},"
			"{\"active\":true,\"buttonAction\":\"game_menu\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":758,\"name\":\"GameMenuButton\",\"order\":25,\"rcssClass\":\"clash-event\",\"text\":\"Game\\nMenu\"},"
			"{\"active\":true,\"buttonAction\":\"open_autocv\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":1796,\"name\":\"AutoConvertButton\",\"order\":24,\"rcssClass\":\"clash-event\",\"text\":\"Auto\\nConvert\"},"
			"{\"active\":false,\"buttonAction\":\"guild_event\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":15856,\"name\":\"GuildEventButton\",\"order\":25,\"rcssClass\":\"clash-event\",\"text\":\"Guild Event\"},"
			"{\"active\":false,\"buttonAction\":\"winterrallymenu\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"name\":\"WinterRallyButton\",\"order\":2,\"rcssClass\":\"winter-rally\",\"text\":\"\"},"
			"{\"active\":false,\"buttonAction\":\"leaderboardBtnClicked\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"name\":\"AnniversaryLeaderboardButton\",\"order\":5,\"rcssClass\":\"anniversary-leaderboard\",\"text\":\"\"},"
			"{\"active\":false,\"buttonAction\":\"euphoriaBtnClicked\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"name\":\"AnniversaryEuphoriaButton\",\"order\":5,\"rcssClass\":\"anniversary-euphoria\",\"text\":\"\"},"
			"{\"active\":false,\"buttonAction\":\"openLnySparksPopup\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":5,\"name\":\"LnyButton\",\"order\":5,\"rcssClass\":\"cny\",\"text\":\"\"},"
			"{\"active\":false,\"buttonAction\":\"donationservermenu\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":1796,\"name\":\"DonationServerButton\",\"order\":26,\"rcssClass\":\"donation-server\",\"text\":\"Donation Server\"}"
			"]}"
		);
		p.Insert("2436624451");
		p.CreatePacket(peer);
		pInfo(peer)->bypass = true;
	}
	static void OnEventButtonDataSet(ENetPeer* peer, string a, int c, string btn) {
		gamepacket_t p;
		p.Insert("OnEventButtonDataSet");
		p.Insert(a);
		p.Insert(c);
		p.Insert(btn);
		p.CreatePacket(peer);
	}
	static void OnBuxGems(ENetPeer* peer, int amount = 0, int delay = 0) {
		if (pInfo(peer)->gp) {
			if (amount >= 30) {
				if (complete_gpass_task(peer, "Gems")) amount += 3;
			}
		}
		if (amount > 0) {
			UpdatePairProgress(peer, 3, amount);
			if (!pInfo(peer)->world.empty()) {
				std::string msg = "`oCollected `$" + std::to_string(amount) + " `oGems(gem)";
				VarList::OnTalkBubble(peer, pInfo(peer)->netID, msg, 0, false, 0);
			}
		}
		gamepacket_t p(delay);
		p.Insert("OnSetBux");
		p.Insert(pInfo(peer)->gems += amount);
		p.Insert(0);
		p.Insert((pInfo(peer)->supp >= 1 || pInfo(peer)->subscriber ? 1 : 0));
		if (pInfo(peer)->supp >= 2 || pInfo(peer)->subscriber) p.Insert((float)33796, (float)1, (float)0);
		p.CreatePacket(peer);
		if (amount > 0) {
			TryAutoConvert(peer);
		}
	}
	static void OnMinGems(ENetPeer* peer, int amount = 0, int delay = 0) {
		if (pInfo(peer)->gp) {
			if (amount >= 30) {
				if (complete_gpass_task(peer, "Gems")) amount += 3;
			}
		}
		gamepacket_t p(delay);
		p.Insert("OnSetBux");
		p.Insert(pInfo(peer)->gems -= amount);
		p.Insert(0);
		p.Insert((pInfo(peer)->supp >= 1 || pInfo(peer)->subscriber ? 1 : 0));
		if (pInfo(peer)->supp >= 2 || pInfo(peer)->subscriber) p.Insert((float)33796, (float)1, (float)0);
		p.CreatePacket(peer);
	}
	static void OnSetPos(ENetPeer* peer, int x, int y, int instant = 0, bool cooldown = false) {
		if (cooldown == false) pInfo(peer)->anticheat_cooldown = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
		gamepacket_t p(instant, pInfo(peer)->netID);
		p.Insert("OnSetPos");
		p.Insert(float(x), float(y));
		p.CreatePacket(peer);
		pInfo(peer)->temp_x = x;
		pInfo(peer)->temp_y = y;
	}
	static void OnSetPos_2(ENetPeer* peer, const CL_Vec2f& pos, int netID) {
		gamepacket_t packet(0, netID);
		packet.Insert("OnSetPos");
		packet.Insert(pos.x, pos.y);
		packet.CreatePacket(peer);
	}
	static void OnSendToServer(ENetPeer* peer, int id, string name) {
		gamepacket_t p;
		p.Insert("OnSendToServer");
		p.Insert(Environment()->Server_Port);
		p.Insert("GrowPlus");
		p.Insert(id);
		p.Insert(Environment()->server_ip + "|0|-1");
		p.Insert(1);
		p.Insert(name);
		p.CreatePacket(peer);
	}
	static void OnCountdownEnd(ENetPeer* peer, int netid) {
		gamepacket_t p(0, netid);
		p.Insert("OnCountdownEnd");
		p.CreatePacket(peer);
	}
	static void SetTapjoyTags(ENetPeer* peer, string a) {
		gamepacket_t p;
		p.Insert("SetTapjoyTags");
		p.Insert(a);
		p.CreatePacket(peer);
	}
	static void OnSetRoleSkinsAndIcons(ENetPeer* peer, int netid, int roleskin, int roleicon) {
		gamepacket_t p(0, netid);
		p.Insert("OnSetRoleSkinsAndIcons"), p.Insert(pInfo(peer)->roleSkin), p.Insert(pInfo(peer)->roleIcon), p.Insert(0);
		p.CreatePacket(peer);
	}
	static void ShowCurrency(ENetPeer* peer) {
		gamepacket_t pb;
		pb.Insert("ShowPearlCurrency");
		pb.CreatePacket(peer);
	}
	static void HideCurrency(ENetPeer* peer) {
		gamepacket_t pb;
		pb.Insert("HidePearlCurrency");
		pb.CreatePacket(peer);
	}
	static void SetCurrency(ENetPeer* peer, int amount = 0) {
		gamepacket_t pb2;
		pb2.Insert("OnSetPearl"), pb2.Insert(amount), pb2.Insert(0);
		pb2.CreatePacket(peer);
	}
	static void OnSetDungeonSouls(ENetPeer* peer, int amount = 0) {
		gamepacket_t pb2;
		pb2.Insert("OnSetDungeonSouls");
		pb2.Insert(amount);
		pb2.CreatePacket(peer);
	}
	static void OnSetVouchers(ENetPeer* peer, int amount = 0) {
		gamepacket_t p;
		p.Insert("OnSetVouchers");
		p.Insert(pInfo(peer)->voucher += amount);
		p.CreatePacket(peer);
	}
	static void CrashTheGameClient(ENetPeer* peer) {
		gamepacket_t p;
		p.Insert("CrashTheGameClient");
		p.CreatePacket(peer);
	}
	static void OnRequestWorldSelectMenu(ENetPeer* peer, string output) {
		gamepacket_t p;
		p.Insert("OnRequestWorldSelectMenu");
		p.Insert(output);
		p.CreatePacket(peer);
	}
	static void OnCommunityHubRequest(ENetPeer* peer, string CMNTY) {
		gamepacket_t p(500, -1);
		p.Insert("OnCommunityHubRequest");
		p.Insert(CMNTY);
		p.CreatePacket(peer);
	}
	static void OnSendLog(ENetPeer* enetPeer, string text, int type) {
		if (enetPeer) {
			ENetPacket* v3 = enet_packet_create(0, text.length() + 5, 1);
			Memory_Copy(v3->data, &type, 4);
			Memory_Copy((v3->data) + 4, text.c_str(), text.length());
			if (enet_peer_send(enetPeer, 0, v3) != 0) {
				enet_packet_destroy(v3);
			}
		}
	}
	static void OnAddNotification(ENetPeer* peer, string text, string interfaces, string audio, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnAddNotification");
		p.Insert(interfaces);
		p.Insert(text);
		p.Insert(audio);
		p.CreatePacket(peer);
	}
	static void OnTalkBubble(ENetPeer* peer, int netID, string text, int chatColor = 0, bool overlay = false, int delay = 0, bool overlay2 = false) {
		gamepacket_t p(delay);
		p.Insert("OnTalkBubble");
		p.Insert(netID);
		p.Insert(text);
		p.Insert(chatColor == 2 ? 2 : (overlay2 == true ? 1 : 0));
		p.Insert((overlay == true ? 1 : 0));
		p.CreatePacket(peer);
	}
	static void OnTextOverlay(ENetPeer* peer, string text, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnTextOverlay");
		p.Insert(text);
		p.CreatePacket(peer);
	}
	static void OnDialogRequest(ENetPeer* peer, string text, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert(dialogrequest(peer).c_str());
		p.Insert(text);
		p.CreatePacket(peer);
		track_pending_dialog(peer, text); // FIX EXPLOIT: catat dialog name yang valid
	}
	static void OnDialogRequestRML(ENetPeer* peer, const std::string& text, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnDialogRequestRML");
		p.Insert(text);
		p.CreatePacket(peer);
		track_pending_dialog(peer, text); // FIX EXPLOIT
	}
	static void OnDailyRewardRequest(ENetPeer* peer, string text, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnDailyRewardRequest");
		p.Insert(text);
		p.CreatePacket(peer);
	}
	static void OnStorePurchaseResult(ENetPeer* peer, string output) {
		gamepacket_t p;
		p.Insert("OnStorePurchaseResult");
		p.Insert(output);
		p.CreatePacket(peer);
	}
	static void SetHasAccountSecured(ENetPeer* peer, bool secured = false) {
		gamepacket_t p(0);
		p.Insert("SetHasAccountSecured");
		p.Insert(secured ? 1 : 0);
		p.CreatePacket(peer);
	}
	static void OnChangePureBeingMode(ENetPeer* peer, int netID, int mode) {
		gamepacket_t p(0, netID);
		p.Insert("OnChangePureBeingMode");
		p.Insert(mode);
		p.CreatePacket(peer);
	}
	static void OnAction(ENetPeer* peer, int netID, string action, int delay = 0) {
		gamepacket_t p(delay, netID);
		p.Insert("OnAction");
		p.Insert(action);
		p.CreatePacket(peer);
	}
	static void OnSetMissionTimer(ENetPeer* peer, int times_, int delay = 0) {
		gamepacket_t p3(delay);
		p3.Insert("OnSetMissionTimer"), p3.Insert(times_);
		p3.CreatePacket(peer);
	}
	static void OnSetCurrentWeather(ENetPeer* peer, int id, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnSetCurrentWeather");
		p.Insert(id);
		p.CreatePacket(peer);
	}
	static void OnPlaySound(ENetPeer* peer, string file, int delay = 0) {
		OnSendLog(peer, "action|play_sfx\nfile|" + file + "\ndelayMS|" + to_string(delay), 3);
	}
	static void OnParticleEffect(ENetPeer* peer, int x, int y, int size, int id, int delay) {
		PlayerMoving datx{};
		datx.packetType = 0x11;
		datx.x = x;
		datx.y = y;
		datx.YSpeed = id;
		datx.XSpeed = size;
		datx.plantingTree = delay;
		BYTE* raw = packPlayerMoving(&datx);
		send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
		delete[]raw;
	}
	static void OnSetPearl(ENetPeer* peer, int remove = 0) {
		gamepacket_t p;
		p.Insert("OnSetPearl"), p.Insert(pInfo(peer)->pearl += remove), p.Insert(0);
		p.CreatePacket(peer);
	}
	static void OnPlanterActivated(ENetPeer* p_, int id, int x_, int y_) {
		gamepacket_t p3;
		p3.Insert("OnPlanterActivated");
		p3.Insert(id);
		p3.Insert(x_);
		p3.Insert(y_);
		p3.CreatePacket(p_);
	}
	static void OnRemove(ENetPeer* peer, int netid, int pId, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnRemove"), p.Insert("netID|" + to_string(netid) + "\n"), p.Insert("pId|" + to_string(pId) + "\n"), p.CreatePacket(peer);
	}
	static void OnFailedToEnterWorld(ENetPeer* peer) {
		gamepacket_t p;
		p.Insert("OnFailedToEnterWorld"), p.CreatePacket(peer);
	}
	static void OnSetFreezeState(ENetPeer* peer, int netID, int delay, int num) {
		gamepacket_t p(delay, netID);
		p.Insert("OnSetFreezeState"), p.Insert(num), p.CreatePacket(peer);
	}
	static void OnZoomCamera(ENetPeer* peer, int delay) {
		gamepacket_t p(delay);
		p.Insert("OnZoomCamera"), p.Insert((float)10000.000000), p.Insert(1000), p.CreatePacket(peer);
	}
	static void OnKilled(ENetPeer* peer, int netID, int delay) {
		gamepacket_t p(delay, netID);
		p.Insert("OnKilled");
		p.CreatePacket(peer);
	}
	static void SetRespawnPos(ENetPeer* peer, int netID, int d, int delay) {
		gamepacket_t p(delay, netID);
		p.Insert("SetRespawnPos");
		p.Insert(d);
		p.CreatePacket(peer);
	}
	static void OnPaw2018SkinColor1Changed(ENetPeer* peer, int d) {
		gamepacket_t p;
		p.Insert("OnPaw2018SkinColor1Changed");
		p.Insert(d);
		p.CreatePacket(peer);
	}
	static void OnPaw2018SkinColor2Changed(ENetPeer* peer, int d) {
		gamepacket_t p;
		p.Insert("OnPaw2018SkinColor2Changed");
		p.Insert(d);
		p.CreatePacket(peer);
	}
	static void OnAchievementCompleted(ENetPeer* peer, int number) {
		gamepacket_t p;
		p.Insert("OnAchievementCompleted"), p.Insert(number), p.CreatePacket(peer);
	}
	static void OnPlayerLeveledUp(ENetPeer* peer, int number) {
		gamepacket_t p;
		p.Insert("OnPlayerLeveledUp"), p.Insert(number), p.CreatePacket(peer);
	}
	static void OnMagicCompassTrackingItemIDChanged(ENetPeer* peer, int d) {
		gamepacket_t p;
		p.Insert("OnMagicCompassTrackingItemIDChanged"), p.Insert(d), p.CreatePacket(peer);
	}
	static void UpdateMainMenuTheme(ENetPeer* peer, int d, int d1, int d2, string a) {
		gamepacket_t p;
		p.Insert("UpdateMainMenuTheme"), p.Insert(d), p.Insert(int(d1)), p.Insert(int(d2)), p.Insert(a), p.CreatePacket(peer);
	}
	static void SetHasGrowID(ENetPeer* peer, int d, string id, string pw) {
		gamepacket_t p;
		p.Insert("SetHasGrowID"), p.Insert(d), p.Insert(id), p.Insert(pw), p.CreatePacket(peer);
	}
	static void OnSetRoleSkinsAndTitles(ENetPeer* peer, string set_skins, string set_titles) {
		gamepacket_t p;
		p.Insert("OnSetRoleSkinsAndTitles");
		p.Insert(set_skins);
		p.Insert(set_titles);
		p.CreatePacket(peer);
	}
	static void OnProgressUISet(ENetPeer* peer, int id, int prog, int req) {
		gamepacket_t p;
		p.Insert("OnProgressUISet"), p.Insert(1), p.Insert(id), p.Insert(prog), p.Insert(req), p.Insert(""), p.Insert(1);
		p.CreatePacket(peer);
	}
	static void OnProgressUIUpdateValue(ENetPeer* peer, int id, int d) {
		gamepacket_t p;
		p.Insert("OnProgressUIUpdateValue"), p.Insert(id), p.Insert(d), p.CreatePacket(peer);
	}
	static void OnSpawn(ENetPeer* peer, string s) {
		gamepacket_t p;
		p.Insert("OnSpawn"), p.Insert(s), p.CreatePacket(peer);
	}
	static void OnEndMission(ENetPeer* peer) {
		gamepacket_t p;
		p.Insert("OnEndMission"), p.CreatePacket(peer);
	}
	static void OnSDBroadcast(ENetPeer* peer, string id, int d) {
		gamepacket_t p;
		p.Insert("OnSDBroadcast"), p.Insert(id), p.Insert(d), p.CreatePacket(peer);
	}
	static void OnForceTradeEnd(ENetPeer* peer) {
		gamepacket_t p;
		p.Insert("OnForceTradeEnd");
		p.CreatePacket(peer);
	}
	static void OnSendFavItemsList(ENetPeer* peer, std::string a, int aa) {
		gamepacket_t p;
		p.Insert("OnSendFavItemsList");
		p.Insert(a), p.Insert(aa);
		p.CreatePacket(peer);
	}
	static void OnFavItemUpdated(ENetPeer* peer, int a, int aa) {
		gamepacket_t p;
		p.Insert("OnFavItemUpdated");
		p.Insert(a), p.Insert(aa);
		p.CreatePacket(peer);
	}
	static void OnTradeStatus(ENetPeer* peer, int netid, string a, string aa, string aaa) {
		gamepacket_t p;
		p.Insert("OnTradeStatus");
		p.Insert(netid), p.Insert(a), p.Insert(aa), p.Insert(aaa);
		p.CreatePacket(peer);
	}
};
namespace variants {
	void on_bux_gems(ENetPeer* peer, int amount = 0) {
		if (pInfo(peer)->gp) {
			if (amount >= 30) {
				if (complete_gpass_task(peer, "Gems")) amount += 3;
			}
		}
		gamepacket_t p;
		p.Insert("OnSetBux");
		p.Insert(pInfo(peer)->gems += amount);
		p.Insert(0);
		p.Insert((pInfo(peer)->supp >= 1 || pInfo(peer)->subscriber ? 1 : 0));
		if (pInfo(peer)->supp >= 2 || pInfo(peer)->subscriber) p.Insert((float)33796, (float)1, (float)0);
		p.CreatePacket(peer);
		if (amount > 0) {
			TryAutoConvert(peer);
		}
	}
	void on_min_gems(ENetPeer* peer, int amount = 0) {
		if (pInfo(peer)->gp) {
			if (amount >= 30) {
				if (complete_gpass_task(peer, "Gems")) amount += 3;
			}
		}
		gamepacket_t p;
		p.Insert("OnSetBux");
		p.Insert(pInfo(peer)->gems -= amount);
		p.Insert(0);
		p.Insert((pInfo(peer)->supp >= 1 || pInfo(peer)->subscriber ? 1 : 0));
		if (pInfo(peer)->supp >= 2 || pInfo(peer)->subscriber) p.Insert((float)33796, (float)1, (float)0);
		p.CreatePacket(peer);
	}
	void on_set_event(ENetPeer* peer, int amount = 0) {
		gamepacket_t p;
		p.Insert("OnProgressUIUpdateValue"), p.Insert(pInfo(peer)->egg_carton += amount), p.Insert(0);
		p.CreatePacket(peer);
	}
	void on_set_voucher(ENetPeer* peer, int amount = 0) {
		gamepacket_t p;
		p.Insert("OnSetVouchers");
		p.Insert(pInfo(peer)->voucher += amount);
		p.CreatePacket(peer);
	}
	void barrel(ENetPeer* peer, int netid, int x, int y, int delay) {
		PlayerMoving data;
		data.packetType = 17;
		data.netID = netid;
		data.x = x;
		data.y = y;
		data.characterState = 0;
		data.plantingTree = 0;
		data.XSpeed = 4;
		data.YSpeed = 1;
		data.punchX = 0;
		data.punchY = 0;
		SendPacketRaw1(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE, delay);
	}
	void CrashTheGameClient(ENetPeer* peer) {
		gamepacket_t p;
		p.Insert("CrashTheGameClient");
		p.CreatePacket(peer);
	}
	void OnRequestWorldSelectMenu(ENetPeer* peer, string output) {
		gamepacket_t p;
		p.Insert("OnRequestWorldSelectMenu");
		p.Insert(output);
		p.CreatePacket(peer);
	}
	void OnParticleEffect(ENetPeer* peer, float x, float y, int id, bool all = false, string name = "", int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnParticleEffect");
		p.Insert(id);
		p.Insert(x + 5, y + 5);
		if (all) {
			for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer)
			{
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
				if (pInfo(currentPeer)->world == name) {
					p.CreatePacket(currentPeer);
				}
			}
		}
		else p.CreatePacket(peer);
	}
	void OnSetPos(ENetPeer* peer, int netID, float x, float y, int delay = 0) {
		// Kalau player ini sedang di-freeze di worldnya, skip kirim posnya
		if (pInfo(peer)->isFrozen) return;

		PlayerMoving data;
		data.packetType = 0;
		data.characterState = 0;
		data.netID = netID;
		data.x = x;
		data.y = y;
		data.punchX = -1;
		data.punchY = -1;
		data.plantingTree = -1;

		SendPacketRaw112(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
		gamepacket_t p(delay, netID);
		p.Insert("OnSetPos");
		p.Insert(x, y);
		p.CreatePacket(peer);
	}
	void OnSendLog(ENetPeer* enetPeer, string text, int type) {
		if (enetPeer) {
			ENetPacket* v3 = enet_packet_create(0, text.length() + 5, 1);
			memcpy(v3->data, &type, 4);
			memcpy((v3->data) + 4, text.c_str(), text.length());
			if (enet_peer_send(enetPeer, 0, v3) != 0) {
				enet_packet_destroy(v3);
			}
		}
	}
	void on_notif(ENetPeer* peer, string text, string interfaces, string audio, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnAddNotification");
		p.Insert(interfaces);
		p.Insert(text);
		p.Insert(audio);
		p.CreatePacket(peer);
	}
	void on_bubble(ENetPeer* peer, int netID, string text, int chatColor = 0, bool overlay = false, int delay = 0, bool overlay2 = false) {
		gamepacket_t p(delay);
		p.Insert("OnTalkBubble");
		p.Insert(netID);
		p.Insert(text);
		p.Insert(chatColor == 2 ? 2 : (overlay2 == true ? 1 : 0));
		p.Insert((overlay == true ? 1 : 0));
		p.CreatePacket(peer);
	}
	void on_overlay(ENetPeer* peer, string text, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnTextOverlay");
		p.Insert(text);
		p.CreatePacket(peer);
	}
	void on_dialog(ENetPeer* peer, string text, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnDialogRequest");
		p.Insert(text);
		p.CreatePacket(peer);
		track_pending_dialog(peer, text); // FIX EXPLOIT
	}
	void SetHasAccountSecured(ENetPeer* peer, bool secured = false) {
		gamepacket_t p(0);
		p.Insert("SetHasAccountSecured");
		p.Insert(secured ? 1 : 0);
		p.CreatePacket(peer);
	}
	void OnSendPingRequest(ENetPeer* peer) {
		int intdata = rand() % 100000;
		PlayerMoving data;
		data.packetType = 22;
		data.plantingTree = intdata;
		SendPacketRaw112(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
	}
	void OnSendPingReply(ENetPeer* peer, PlayerMoving* datas) {
		int intdata = datas->plantingTree;
		PlayerMoving data;
		data.packetType = 22;
		data.plantingTree = intdata;
		SendPacketRaw112(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
	}
	void OnSpawn(ENetPeer* peer, string name, string country, int netID, int userID, float x, float y, int invis, int mstate, int smstate, bool local, int level = 1, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnSpawn");
		p.Insert("spawn|avatar|\nnetID|" + to_string(netID) + "\nuserID|" + to_string(userID) + "\ncolrect|0|0|20|30|\nposXY|" + to_string(x) + "|" + to_string(y) + "\nname|````" + name + " `w(`2" + to_string(level) + "`w)""\ncountry|" + country + "\ninvis|" + to_string(invis) + "\nmstate|" + to_string(mstate) + "\nsmstate|" + to_string(smstate) + (local == true ? "\nonlineID|\ntype|local" : "\n"));
		p.CreatePacket(peer);
	}
	void OnChangePureBeingMode(ENetPeer* peer, int netID, int mode) {
		gamepacket_t p(0, netID);
		p.Insert("OnChangePureBeingMode");
		p.Insert(mode);
		p.CreatePacket(peer);
	}
	void on_play(ENetPeer* peer, int netID, string file, int delay = 0) {
		gamepacket_t p(delay, netID);
		p.Insert("OnPlayPositioned");
		p.Insert(file);
		p.CreatePacket(peer);
	}
	void OnNameChanged(ENetPeer* peer, int netID, string name, bool all = false) {
		gamepacket_t p(0, netID);
		p.Insert("OnNameChanged");
		p.Insert(name);
		p.CreatePacket(peer);
		if (all) {
			for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world.empty() or pInfo(currentPeer)->tankIDName.empty()) continue;
				if (pInfo(peer)->world == pInfo(currentPeer)->world)
					p.CreatePacket(currentPeer);

			}
		}
	}
	void on_msg(ENetPeer* peer, string text, bool all = false, int dly = 0) {
		gamepacket_t p(dly);
		p.Insert("OnConsoleMessage");
		p.Insert("`o" + text);
		if (!all) p.CreatePacket(peer);
		else {
			for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
				p.CreatePacket(currentPeer);
			}
		}
	}
	void OnPlaySound(ENetPeer* peer, string file, int delay = 0) {
		variants::OnSendLog(peer, "action|play_sfx\nfile|" + file + "\ndelayMS|" + to_string(delay), 3);
	}
	void Sendtoplayer(ENetPeer* peer, string message, bool console = false, bool bubble = false, bool overlay = false) {
		if (console) {
			gamepacket_t packet;
			packet.Insert("OnConsoleMessage");
			packet.Insert(message);
			packet.CreatePacket(peer);
		}
		if (bubble) {
			gamepacket_t p;
			p.Insert("OnTalkBubble");
			p.Insert(pInfo(peer)->netID);
			p.Insert(message);
			p.Insert(2), p.Insert(0);
			p.CreatePacket(peer);
		}
		if (overlay) {
			gamepacket_t packet;
			packet.Insert("OnTextOverlay");
			packet.Insert(message);
			packet.CreatePacket(peer);
		}
	}
	void OnParticleEffect(ENetPeer* peer, int effect, int size, int netid, int x, int y, int delay) {
		PlayerMoving data;
		data.packetType = 17;
		data.netID = netid;
		data.x = x;
		data.y = y;
		data.characterState = 0;
		data.plantingTree = 0;
		data.XSpeed = size;
		data.YSpeed = effect;
		data.punchX = 0;
		data.punchY = 0;
		SendPacketRaw1(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE, delay);
	}
}
class SetEvent {
public:
	static void Gems(ENetPeer* peer, const string& error = "", const string& a = "", const string& aa = "") {
		VarList::OnDialogRequest(peer, "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wSet x Gems Event|left|14590|" + (error.empty() ? "" : "\nadd_spacer|small|\nadd_smalltext|`4" + error + "|left|") + "\nadd_spacer|small|\nadd_smalltext|`oX Gems (1-1000):|left|\nadd_text_input|count_x|`oCount:|" + a + "|5|\nadd_smalltext|`oTime: if you input 1 it is equal to 1 hours.|left|\nadd_text_input|time|`oTime:|" + aa + "|5|\nadd_spacer|small|\nend_dialog|Set_Gems_Event|Cancel|Update|");
	}
	static void Xp(ENetPeer* peer, const string& error = "", const string& a = "", const string& aa = "") {
		VarList::OnDialogRequest(peer, "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wSet x Xp Event|left|9590|" + (error.empty() ? "" : "\nadd_spacer|small|\nadd_smalltext|`4" + error + "|left|") + "\nadd_spacer|small|\nadd_smalltext|`oX Xp (1-1000):|left|\nadd_text_input|count_x|`oCount:|" + a + "|5|\nadd_smalltext|`oTime: if you input 1 it is equal to 1 hours.|left|\nadd_text_input|time|`oTime:|" + aa + "|5|\nadd_spacer|small|\nend_dialog|Set_Xp_Event|Cancel|Update|");
	}
	static void Exchange(ENetPeer* peer, const string& error = "", const string& a = "", const string& aa = "") {
		VarList::OnDialogRequest(peer, "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`wSet x Exchange Event|left|9922|" + (error.empty() ? "" : "\nadd_spacer|small|\nadd_smalltext|`4" + error + "|left|") + "\nadd_spacer|small|\nadd_smalltext|`oX Xp (1-1000):|left|\nadd_text_input|count_x|`oCount:|" + a + "|5|\nadd_smalltext|`oTime: if you input 1 it is equal to 1 hours.|left|\nadd_text_input|time|`oTime:|" + aa + "|5|\nadd_spacer|small|\nend_dialog|Set_Exchange_Event|Cancel|Update|");
	}
};
int GenerateSpinNumber(ENetPeer* peer) {
	Player* player_data = pInfo(peer);
	long long ms_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	if (player_data->has_csn_buff && ms_time >= player_data->csn_buff_end_time) {
		player_data->has_csn_buff = false;
		player_data->csn_buff_end_time = 0;
		VarList::OnConsoleMessage(peer, "4Buff Konsumsi CSN telah berakhir.");
	}

	int default_chance_special = 5;
	int default_chance_others = 1;

	int chance_7_9 = player_data->has_csn_buff ? 50 : default_chance_special;
	int chance_0_19_28 = player_data->has_csn_buff ? 30 : default_chance_special;
	int chance_others = player_data->has_csn_buff ? 20 : default_chance_others;

	int total_weight = (chance_7_9 * 3) + (chance_0_19_28 * 3) + (chance_others * 31);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(1, total_weight);

	int random_roll = distrib(gen);
	int current_weight_sum = 0;

	current_weight_sum += (chance_7_9 * 3);
	if (random_roll <= current_weight_sum) {
		std::uniform_int_distribution<> d(7, 9);
		return d(gen);
	}

	current_weight_sum += (chance_0_19_28 * 3);
	if (random_roll <= current_weight_sum) {
		std::vector<int> special_nums = { 0, 19, 28 };
		std::uniform_int_distribution<> d(0, special_nums.size() - 1);
		return special_nums[d(gen)];
	}

	std::vector<int> other_nums;
	for (int i = 0; i <= 36; ++i) {
		if (i != 0 && !(i >= 7 && i <= 9) && i != 19 && i != 28) {
			other_nums.push_back(i);
		}
	}
	std::uniform_int_distribution<> d(0, other_nums.size() - 1);
	return other_nums[d(gen)];
}
int calcBanDuration(const long long banDuration) {
	auto duration = 0;
	duration = banDuration - GetCurrentTimeInternalSeconds();
	if (duration <= 0) return 0;
	else return duration;
}
inline int64_t currentTimestamp() {
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
double calculateDistance(int x1, int y1, int x2, int y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}
class PlayerDB {
public:
	static void RegisAndLogin_Page(ENetPeer* peer) {
		VarList::OnDialogRequest(peer,
			"set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\n"
			"add_label_with_icon|big|`4In-Game Registration Removed|left|6016|\n"
			"add_spacer|small|\n"
			"add_textbox|`oIn-game registration is disabled. Please create your GrowID through our official `2Dashboard / Website``.|left|\n"
			"add_spacer|small|\n"
			"add_url_button|comment|`5Open Dashboard``|noflags|" + Environment()->website_url + "|Do you want to open the registration page?|0|0|\n"
			"add_spacer|small|\n"
			"end_dialog|registration_notice||OK|"
		);
	}
	static void Aap_Page(ENetPeer* peer, const string& error = "") {
		VarList::OnDialogRequest(peer, "set_default_color|""|\nadd_label|big|`wAdvanced Account Protection|left|\nadd_smalltext|`4Advanced Account Protection: `oYou tried to log in from the new Device and IP`o, enter the pin that you created if you forgot your PIN, Please Report to the Developer or chat `2Dava `oon WhatsApp. And include proof that it is your account|left|" + (error.empty() ? "" : "\nadd_smalltext|`4OOPS:`` `o" + error + "``|") + "\nadd_text_input_password|verificationcode|`5Enter your Pin:``||4|\nadd_button|verify_2fa|`wSubmit|noflags|0|0|\nend_dialog|Login_Regis_Handle|||");
	}
	static string Regis_Dialog(const string& r_ = "", const string& a_ = "", const string& b_ = "", const string& c_ = "", const string& d_ = "") {
		return "set_default_color|`o\nset_bg_color|0,52,102,178|\nset_border_color|255,255,255,255|\n\nadd_label_with_icon|big|`4In-Game Registration Removed|left|6016|\nadd_spacer|small|\nadd_textbox|`oIn-game registration is disabled. Please create your GrowID through our official `2Dashboard / Website``.|left|\nadd_spacer|small|\nadd_url_button|comment|`5Open Dashboard``|noflags|" + Environment()->website_url + "|Do you want to open the registration page?|0|0|\nadd_spacer|small|\nend_dialog|registration_notice||OK|";
	}
};