#pragma once
struct gamepacket_t
{
private:
	int index = 0, len = 0, capacity = 0;
	BYTE static_buffer[512];
	BYTE* packet_data = nullptr;

	void ensure_capacity(int needed) {
		if (needed > capacity) {
			int new_cap = capacity == 0 ? 512 : capacity;
			while (new_cap < needed) {
				new_cap *= 2;
			}
			BYTE* data = new BYTE[new_cap];
			if (packet_data && len > 0) {
				memcpy(data, packet_data, len);
			}
			if (packet_data != static_buffer) {
				delete[] packet_data;
			}
			packet_data = data;
			capacity = new_cap;
		}
	}

public:
	gamepacket_t(int delay = 0, int NetID = -1) {
		len = 61;
		capacity = 512;
		packet_data = static_buffer;
		int MessageType = 0x4, PacketType = 0x1, CharState = 0x8;
		memset(packet_data, 0, 61);
		memcpy(packet_data, &MessageType, 4);
		memcpy(packet_data + 4, &PacketType, 4);
		memcpy(packet_data + 8, &NetID, 4);
		memcpy(packet_data + 16, &CharState, 4);
		memcpy(packet_data + 24, &delay, 4);
	}
	~gamepacket_t() {
		if (packet_data != static_buffer) {
			delete[] packet_data;
		}
	}
	gamepacket_t(const gamepacket_t&) = delete;
	gamepacket_t& operator=(const gamepacket_t&) = delete;

	void Insert(std::string_view a) {
		int str_len = (int)a.length();
		int needed = len + 2 + str_len + 4;
		ensure_capacity(needed);
		packet_data[len] = index;
		packet_data[len + 1] = 0x2;
		memcpy(packet_data + len + 2, &str_len, 4);
		memcpy(packet_data + len + 6, a.data(), str_len);
		len = needed;
		index++;
		packet_data[60] = index;
	}
	void Insert(int a) {
		int needed = len + 2 + 4;
		ensure_capacity(needed);
		packet_data[len] = index;
		packet_data[len + 1] = 0x9;
		memcpy(packet_data + len + 2, &a, 4);
		len = needed;
		index++;
		packet_data[60] = index;
	}
	void Insert(unsigned int a) {
		int needed = len + 2 + 4;
		ensure_capacity(needed);
		packet_data[len] = index;
		packet_data[len + 1] = 0x5;
		memcpy(packet_data + len + 2, &a, 4);
		len = needed;
		index++;
		packet_data[60] = index;
	}
	void Insert(float a) {
		int needed = len + 2 + 4;
		ensure_capacity(needed);
		packet_data[len] = index;
		packet_data[len + 1] = 0x1;
		memcpy(packet_data + len + 2, &a, 4);
		len = needed;
		index++;
		packet_data[60] = index;
	}
	void Insert(float a, float b) {
		int needed = len + 2 + 8;
		ensure_capacity(needed);
		packet_data[len] = index;
		packet_data[len + 1] = 0x3;
		memcpy(packet_data + len + 2, &a, 4);
		memcpy(packet_data + len + 6, &b, 4);
		len = needed;
		index++;
		packet_data[60] = index;
	}
	void Insert(float a, float b, float c) {
		int needed = len + 2 + 12;
		ensure_capacity(needed);
		packet_data[len] = index;
		packet_data[len + 1] = 0x4;
		memcpy(packet_data + len + 2, &a, 4);
		memcpy(packet_data + len + 6, &b, 4);
		memcpy(packet_data + len + 10, &c, 4);
		len = needed;
		index++;
		packet_data[60] = index;
	}
	void CreatePacket(ENetPeer* peer) {
		if (!peer) return;
		ENetPacket* packet = enet_packet_create(packet_data, len, 1);
		if (enet_peer_send(peer, 0, packet) != 0) {
			enet_packet_destroy(packet);
		}
	}
	ENetPacket* get_packet() {
		return enet_packet_create(packet_data, len, 1);
	}
};

inline PlayerMoving unpackPlayerMoving(BYTE* data) {
	PlayerMoving dataStruct;
	if (data != nullptr) {
		memcpy(&dataStruct.packetType, data, 4);
		memcpy(&dataStruct.netID, data + 4, 4);
		memcpy(&dataStruct.effect_flags_check, data + 8, 4);
		memcpy(&dataStruct.characterState, data + 12, 4);
		memcpy(&dataStruct.plantingTree, data + 20, 4);
		memcpy(&dataStruct.x, data + 24, 4);
		memcpy(&dataStruct.y, data + 28, 4);
		memcpy(&dataStruct.XSpeed, data + 32, 4);
		memcpy(&dataStruct.YSpeed, data + 36, 4);
		memcpy(&dataStruct.punchX, data + 44, 4);
		memcpy(&dataStruct.punchY, data + 48, 4);
	}
	return dataStruct;
}
BYTE* get_struct(ENetPacket* packet) {
	const unsigned int packetLenght = (unsigned int)packet->dataLength;
	BYTE* result = nullptr;
	if (packetLenght >= 0x3C) {
		BYTE* packetData = packet->data;
		result = packetData + 4;
		if (*static_cast<BYTE*>(packetData + 16) & 8) {
			if (packetLenght < (unsigned int)*reinterpret_cast<int*>(packetData + 56) + 60)
				result = nullptr;
		}
		else {
			int zero = 0;
			memcpy(packetData + 56, &zero, 4);
		}
	}
	return result;
}

void SendPacketRaw112(int a1, void* packetData, size_t packetDataSize, void* a4, ENetPeer* peer, int packetFlag)
{
	ENetPacket* p;

	if (peer) // check if we have it setup
	{
		if (a1 == 4 && *((BYTE*)packetData + 12) & 8)
		{
			p = enet_packet_create(0, packetDataSize + *((DWORD*)packetData + 13) + 5, packetFlag);
			int four = 4;
			memcpy(p->data, &four, 4);
			memcpy((char*)p->data + 4, packetData, packetDataSize);
			memcpy((char*)p->data + packetDataSize + 4, a4, *((DWORD*)packetData + 13));
			if (enet_peer_send(peer, 0, p) != 0) {
				enet_packet_destroy(p);
			}
		}
		else
		{
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
void SendPacketRaw1(int a1, void* packetData, size_t packetDataSize, void* a4, ENetPeer* peer, int packetFlag, int delay)
{
	ENetPacket* p;

	if (peer) // check if we have it setup
	{
		if (a1 == 4 && *((BYTE*)packetData + 12) & 8)
		{

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
		else
		{
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

namespace global {
	std::vector<ENetPeer*> autoFarming;
	std::vector<ENetPeer*> autoSpamming;
}
namespace world_::data {
	template<typename Func>
	void players(Func function) {
		for (ENetPeer* cp_ = server->peers; cp_ < &server->peers[server->peerCount]; cp_++) {
			if (cp_->state != ENET_PEER_STATE_CONNECTED) continue;
			function(cp_);
		}
	}
	template<typename Func>
	void world(std::string const& world, Func func) {
		world_::data::players([&](ENetPeer* cp_) {
			if (cp_ == nullptr || cp_->data == nullptr) return;
			if (pInfo(cp_)->world == world) func(cp_);
			});
	}
	int get(const std::string& nameWorld) {
		int defaults = 1;
		world_::data::world(nameWorld, [&](ENetPeer* peer) {
			if (pInfo(peer)->show_pets && pInfo(peer)->pet_type != -1) {
				if (pInfo(peer)->netID >= defaults) defaults = pInfo(peer)->netID + 1 + name_to_number(to_lower(pInfo(peer)->tankIDName));
				if (pInfo(peer)->pet_netID >= defaults) defaults = pInfo(peer)->pet_netID + 1 + name_to_number(to_lower(pInfo(peer)->tankIDName));
			}
			else if (pInfo(peer)->netID >= defaults) defaults = pInfo(peer)->netID + 1 + name_to_number(to_lower(pInfo(peer)->tankIDName));
			});
		return defaults;
	}
};
class CAction {
public:
	static void Effect(ENetPeer* peer, int id, int x, int y) {
		gamepacket_t p;
		p.Insert("OnParticleEffect"), p.Insert(id), p.Insert(x, y), p.CreatePacket(peer);
	}
	static void Effect_V2(ENetPeer* peer, int id, int x, int y, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnParticleEffectV2"), p.Insert(id), p.Insert(x, y), p.CreatePacket(peer);
	}
	static void Positioned(ENetPeer* peer, int netID, const string& file, int delay = 0) {
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
		size_t len1 = 5 + t_.length();
		BYTE stack_d[512];
		BYTE* d_ = (len1 <= 512) ? stack_d : new BYTE[len1];
		memcpy(d_, &y_, 4);
		memcpy(d_ + 4, t_.c_str(), t_.length());
		memcpy(d_ + 4 + t_.length(), &z_, 1);
		ENetPacket* const p = enet_packet_create(d_, len1, ENET_PACKET_FLAG_RELIABLE);
		if (enet_peer_send(p_, 0, p) != 0) {
			enet_packet_destroy(p);
		}
		if (d_ != stack_d) delete[] d_;
		if (l_ != "") {
			l_ = "action|set_url\nurl|" + w_ + "\nlabel|" + l_ + "\n";
			size_t len2 = 5 + l_.length();
			BYTE stack_u[512];
			BYTE* u_ = (len2 <= 512) ? stack_u : new BYTE[len2];
			memcpy(u_, &y_, 4);
			memcpy(u_ + 4, l_.c_str(), l_.length());
			memcpy(u_ + 4 + l_.length(), &z_, 1);
			ENetPacket* const p3 = enet_packet_create(u_, len2, ENET_PACKET_FLAG_RELIABLE);
			if (enet_peer_send(p_, 0, p3) != 0) {
				enet_packet_destroy(p3);
			}
			if (u_ != stack_u) delete[] u_;
		}
	}
};
namespace vz {
	static void OnRemove(ENetPeer* peer, int netid, int pId) {
		gamepacket_t p;
		p.Insert("OnRemove"), p.Insert("netID|" + to_string(netid) + "\n"), p.Insert("pId|" + to_string(pId) + "\n"), p.CreatePacket(peer);
	}
	void CrashTheGameClient(ENetPeer* peer) {
		gamepacket_t p;
		p.Insert("CrashTheGameClient");
		p.CreatePacket(peer);
	}
	static void OnCountdownStart(ENetPeer* peer, int netID, int time, int score) {
		gamepacket_t p(0, netID);
		if (score == -1) {
			p.Insert("OnCountdownStart"), p.Insert(time);
		}
		else
		{
			p.Insert("OnCountdownStart"), p.Insert(time), p.Insert(time);
		}
		p.CreatePacket(peer);
	}
	static void OnAddNotification(ENetPeer* peer, const string& text, const string& interfaces, const string& audio, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnAddNotification");
		p.Insert(interfaces);
		p.Insert(text);
		p.Insert(audio);
		p.CreatePacket(peer);
	}
	static void OnConsoleMessage(ENetPeer* peer, const string& text, bool all = false, int dly = 0) {
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
	static void OnKilled(ENetPeer* peer, int netID) {
		gamepacket_t p(0, netID);
		p.Insert("OnKilled");
		p.CreatePacket(peer);
	}
	static void OnConsoleMessageWithTime(ENetPeer* peer, const string& message, int ms) {
		gamepacket_t p(ms);
		p.Insert("OnConsoleMessage"), p.Insert(message);
		p.CreatePacket(peer);
	}
	static void OnSetPos(ENetPeer* peer, int netID, int x, int y, int ms) {
		gamepacket_t p(0, netID);
		p.Insert("OnSetPos"), p.Insert(x, y);
		p.CreatePacket(peer);
	}
	static void OnNameChanged(ENetPeer* peer, int netID, const string& name) {
		gamepacket_t p(0, netID);
		p.Insert("OnNameChanged");
		if (name.find("``") != string::npos) {
			p.Insert(name);
		}
		else {
			p.Insert(name + "``");
		}
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
			if (pInfo(peer)->world == pInfo(currentPeer)->world) p.CreatePacket(currentPeer);
		}
	}
	static void OnSetFreezeState(ENetPeer* peer, int state, int netID) {
		gamepacket_t p(0, netID);
		p.Insert("OnSetFreezeState"), p.Insert(state);
		p.CreatePacket(peer);
	}
	static void OnFailedToEnterWorld(ENetPeer* peer) {
		gamepacket_t p;
		p.Insert("OnFailedToEnterWorld"), p.Insert(1);
		p.CreatePacket(peer);
	}

	static void OnZoomCamera(ENetPeer* peer, float value1, int value2) {
		gamepacket_t p;
		p.Insert("OnZoomCamera"), p.Insert(value1), p.Insert(value2);
		p.CreatePacket(peer);
	}

	static void OnAddNotification(ENetPeer* peer, const string& text, const string& audiosound, const string& interfaceimage)
	{
		gamepacket_t p;
		p.Insert("OnAddNotification"), p.Insert(interfaceimage), p.Insert(text), p.Insert(audiosound), p.Insert(0);
		p.CreatePacket(peer);
	}

	static void SmoothZoom(ENetPeer* peer) {
		gamepacket_t p;
		p.Insert("OnZoomCamera"), p.Insert(10000.0f), p.Insert(1000);
		p.CreatePacket(peer);
	}
	static void SetRespawnPos(ENetPeer* peer, int posX, int posY, int netID) {
		gamepacket_t p(0, netID);
		p.Insert("SetRespawnPos"), p.Insert(posX + posY); // (world->width * posY)
		p.CreatePacket(peer);
	}

	static void OnTradeStatus(ENetPeer* peer, int netID, const string& statustext, const string& items, const string& locked)
	{
		gamepacket_t p;
		p.Insert("OnTradeStatus");
		p.Insert(netID);
		p.Insert(items);
		p.Insert(statustext);
		p.Insert(locked);
		p.CreatePacket(peer);
	}
	static void OnChangeSkin(ENetPeer* peer, int skinColor, int netID) {
		gamepacket_t p(0, netID);
		p.Insert("OnChangeSkin"), p.Insert(skinColor);
		p.CreatePacket(peer);
	}
	static void OnSpawn(ENetPeer* peer, const string& s) {
		gamepacket_t p;
		p.Insert("OnSpawn"), p.Insert(s), p.CreatePacket(peer);
	}


	inline void OnStartTrade(ENetPeer* peer, const string& displayName, int netID) {
		gamepacket_t p;
		p.Insert("OnStartTrade");
		p.Insert(displayName);
		p.Insert(netID);
		p.CreatePacket(peer);
	}

	static void OnForceTradeEnd(ENetPeer* peer)
	{
		gamepacket_t p;
		p.Insert("OnForceTradeEnd");
		p.CreatePacket(peer);
	}
	static void OnTextOverlay(ENetPeer* peer, const string& text) {
		gamepacket_t p;
		p.Insert("OnTextOverlay");
		p.Insert(text);
		p.CreatePacket(peer);
	}

	static void OnSetBux(ENetPeer* peer, int gems, int accountstate)
	{
		gamepacket_t p;
		p.Insert("OnSetBux");
		p.Insert(gems), p.Insert(accountstate);
		p.CreatePacket(peer);
	}

	static void OnDialogRequest(ENetPeer* peer, string message, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnDialogRequest");
		p.Insert(message);
		p.Insert(4);
		p.CreatePacket(peer);
	}
	static void OnSendToServer(ENetPeer* peer, int userID, int token, string ip, int port, string doorId, int lmode)
	{
		gamepacket_t p;
		p.Insert("OnSendToServer"), p.Insert(port), p.Insert(token), p.Insert(userID), p.Insert(ip + "|" + doorId), p.Insert(lmode);
		p.CreatePacket(peer);
	}
	static void OnConsoleMessageAll(string message) {
		gamepacket_t p;
		p.Insert("OnConsoleMessage"), p.Insert(message);
		ENetPeer* currentPeer;
		for (currentPeer = server->peers;
			currentPeer < &server->peers[server->peerCount];
			++currentPeer)
		{
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
				continue;
			p.CreatePacket(currentPeer);
		}
	}

	static void PlayAudio(ENetPeer* peer, string audioFile, int delayMS)
	{
		string text = "action|play_sfx\nfile|" + audioFile + "\ndelayMS|" + to_string(delayMS) + "\n";
		BYTE* data = new BYTE[5 + text.length()];
		BYTE zero = 0;
		int type = 3;
		memcpy(data, &type, 4);
		memcpy(data + 4, text.c_str(), text.length());
		memcpy(data + 4 + text.length(), &zero, 1);
		ENetPacket* packet = enet_packet_create(data,
			5 + text.length(),
			ENET_PACKET_FLAG_RELIABLE);

		if (enet_peer_send(peer, 0, packet) != 0) {
			enet_packet_destroy(packet);
		}
		delete[] data;
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
};
namespace variants {
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
	static void OnCountdownStart(ENetPeer* peer, int netID, int time, int score) {
		gamepacket_t p(0, netID);
		if (score == -1) {
			p.Insert("OnCountdownStart"), p.Insert(time);
		}
		else
		{
			p.Insert("OnCountdownStart"), p.Insert(time), p.Insert(time);
		}
		p.CreatePacket(peer);
	}
	void OnSetCurrentWeather(ENetPeer* peer, int const& weatherId) {
		gamepacket_t p;
		p.Insert("OnSetCurrentWeather");
		p.Insert(weatherId);
		p.CreatePacket(peer);
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
		PlayerMoving data;
		data.packetType = 0;
		data.characterState = 0;
		data.netID = netID;
		data.x = x;
		data.y = x;
		data.punchX = -1;
		data.punchY = -1;
		data.plantingTree = -1;
		SendPacketRaw112(4, packPlayerMoving(&data), 56, 0, peer, ENET_PACKET_FLAG_RELIABLE);
		gamepacket_t p(delay, netID);
		p.Insert("OnSetPos");
		p.Insert(x, y);
		p.CreatePacket(peer);
	}
	void OnSendLog(ENetPeer* enetPeer, string text, int type)
	{
		if (enetPeer)
		{
			ENetPacket* v3 = enet_packet_create(0, text.length() + 5, 1);
			memcpy(v3->data, &type, 4);
			memcpy((v3->data) + 4, text.c_str(), text.length());
			if (enet_peer_send(enetPeer, 0, v3) != 0) {
				enet_packet_destroy(v3);
			}
		}
	}
	void OnAddNotification(ENetPeer* peer, string text, string interfaces, string audio, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnAddNotification");
		p.Insert(interfaces);
		p.Insert(text);
		p.Insert(audio);
		p.CreatePacket(peer);
	}
	void OnTalkBubble(ENetPeer* peer, int netID, string text, int chatColor = 0, bool overlay = false, int delay = 0, bool overlay2 = false) {
		gamepacket_t p(delay);
		p.Insert("OnTalkBubble");
		p.Insert(netID);
		p.Insert(text);
		p.Insert(chatColor == 2 ? 2 : (overlay2 == true ? 1 : 0));
		p.Insert((overlay == true ? 1 : 0));
		p.CreatePacket(peer);
	}
	void SetHasAccountSecured(ENetPeer* peer, bool secured = false) {
		gamepacket_t p(0);
		p.Insert("SetHasAccountSecured");
		p.Insert(secured ? 1 : 0);
		p.CreatePacket(peer);
	}
	void OnDialogRequest(ENetPeer* peer, string text, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnDialogRequest");
		p.Insert("" + text);//last pos
		p.CreatePacket(peer);
	}
	void OnTextOverlay(ENetPeer* peer, string text, int delay = 0) {
		gamepacket_t p(delay);
		p.Insert("OnTextOverlay");
		p.Insert(text);
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
		p.Insert(
			"spawn|avatar"
			"\nnetID|" + to_string(netID) +
			"\nuserID|" + to_string(userID) +
			"\ncolrect|0|0|20|30"
			"\nposXY|" + to_string(x) + "|" + to_string(y) +
			"\nname|````" + name + " `w(`2" + to_string(level) + "`w)"
			"\ncountry|" + country +
			"\ninvis|" + to_string(invis) +
			"\nmstate|" + to_string(mstate) +
			"\nsmstate|" + to_string(smstate) +
			(local == true ? "\nonlineID|\ntype|local" : "\n"));
		p.CreatePacket(peer);
	}
	void OnChangePureBeingMode(ENetPeer* peer, int netID, int mode) {
		gamepacket_t p(0, netID);
		p.Insert("OnChangePureBeingMode");
		p.Insert(mode);
		p.CreatePacket(peer);
	}
	void OnPlayPositioned(ENetPeer* peer, int netID, string file, int delay = 0)
	{
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
	void OnConsoleMessage(ENetPeer* peer, string text, bool all = false, int dly = 0) {
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

inline int retRandomValues(int rangeMin, int rangeMax) {
	if (rangeMin > rangeMax) std::swap(rangeMin, rangeMax);
	thread_local static std::mt19937 rng(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
	std::uniform_int_distribution<int> udist(rangeMin, rangeMax);
	return udist(rng);
}

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