#pragma once
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <vector>
#include <string>
#include <atomic>
#include <algorithm>

struct FloodEntry {
	long long first_packet_time = 0;
	int packet_count = 0;
	int violation_count = 0;
	long long last_violation_time = 0;
	long long ban_until = 0;
	bool is_banned = false;
};

struct ConnectionEntry {
	long long first_connect_time = 0;
	int connect_count = 0;
	long long last_connect_time = 0;
	long long ban_until = 0;
	bool is_banned = false;
};

struct AntiDDoSConfig {
	int max_packets_per_second = 80;
	int max_connect_per_minute = 15;
	int max_invalid_packets = 10;
	int max_text_packets_per_second = 30;
	int max_tank_packets_per_second = 60;
	int max_login_attempts_per_minute = 5;
	int violation_threshold = 3;
	int ban_duration_seconds = 300;
	int escalation_multiplier = 2;
	int max_ban_duration_seconds = 3600;
	int cleanup_interval_seconds = 120;
	int max_payload_size = 8192;
	int max_peers_per_ip = 3;
	bool enabled = true;
	bool log_violations = true;
};

class AntiDDoS {
private:
	static inline std::shared_mutex m_mutex;
	static inline std::unordered_map<uint32_t, FloodEntry> m_flood_table;
	static inline std::unordered_map<uint32_t, ConnectionEntry> m_connect_table;
	static inline std::unordered_map<uint32_t, FloodEntry> m_text_flood_table;
	static inline std::unordered_map<uint32_t, FloodEntry> m_tank_flood_table;
	static inline std::unordered_map<uint32_t, int> m_invalid_packet_table;
	static inline std::unordered_map<uint32_t, int> m_login_attempt_table;
	static inline std::unordered_map<uint32_t, int> m_peer_count_table;
	static inline AntiDDoSConfig m_config;
	static inline long long m_last_cleanup = 0;
	static inline std::atomic<long long> m_total_blocked{0};
	static inline std::atomic<long long> m_total_packets{0};

	static long long now_ms() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	static uint32_t get_ip(ENetPeer* peer) {
		if (!peer) return 0;
		return peer->address.host;
	}

	static std::string ip_to_string(uint32_t ip) {
		return std::to_string(ip & 0xFF) + "." +
		       std::to_string((ip >> 8) & 0xFF) + "." +
		       std::to_string((ip >> 16) & 0xFF) + "." +
		       std::to_string((ip >> 24) & 0xFF);
	}

	static int calculate_ban_duration(int violations) {
		int duration = m_config.ban_duration_seconds;
		for (int i = 1; i < violations && i < 5; i++) {
			duration *= m_config.escalation_multiplier;
		}
		if (duration > m_config.max_ban_duration_seconds) {
			duration = m_config.max_ban_duration_seconds;
		}
		return duration;
	}

	static bool check_flood_generic(std::unordered_map<uint32_t, FloodEntry>& table,
	                                 uint32_t ip, int max_per_second) {
		long long now = now_ms();
		auto& entry = table[ip];

		if (entry.is_banned) {
			if (now < entry.ban_until) return true;
			entry.is_banned = false;
			entry.violation_count = 0;
			entry.packet_count = 0;
		}

		if (now - entry.first_packet_time > 1000) {
			entry.first_packet_time = now;
			entry.packet_count = 1;
			return false;
		}

		entry.packet_count++;

		if (entry.packet_count > max_per_second) {
			entry.violation_count++;
			entry.last_violation_time = now;

			if (entry.violation_count >= m_config.violation_threshold) {
				int ban_secs = calculate_ban_duration(entry.violation_count);
				entry.ban_until = now + (ban_secs * 1000LL);
				entry.is_banned = true;
				m_total_blocked++;
			}
			return true;
		}

		return false;
	}

public:
	static AntiDDoSConfig& GetConfig() { return m_config; }

	static void Initialize() {
		m_flood_table.reserve(1024);
		m_connect_table.reserve(512);
		m_text_flood_table.reserve(1024);
		m_tank_flood_table.reserve(1024);
		m_invalid_packet_table.reserve(256);
		m_login_attempt_table.reserve(256);
		m_peer_count_table.reserve(256);
		m_last_cleanup = now_ms();
	}

	static bool CheckPacketFlood(ENetPeer* peer) {
		if (!m_config.enabled || !peer) return false;
		m_total_packets++;
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		return check_flood_generic(m_flood_table, ip, m_config.max_packets_per_second);
	}

	static bool CheckTextFlood(ENetPeer* peer) {
		if (!m_config.enabled || !peer) return false;
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		return check_flood_generic(m_text_flood_table, ip, m_config.max_text_packets_per_second);
	}

	static bool CheckTankPacketFlood(ENetPeer* peer) {
		if (!m_config.enabled || !peer) return false;
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		return check_flood_generic(m_tank_flood_table, ip, m_config.max_tank_packets_per_second);
	}

	static bool CheckConnectionFlood(ENetPeer* peer) {
		if (!m_config.enabled || !peer) return false;
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		long long now = now_ms();
		auto& entry = m_connect_table[ip];

		if (entry.is_banned) {
			if (now < entry.ban_until) return true;
			entry.is_banned = false;
			entry.connect_count = 0;
		}

		if (now - entry.first_connect_time > 60000) {
			entry.first_connect_time = now;
			entry.connect_count = 1;
			entry.last_connect_time = now;
			return false;
		}

		entry.connect_count++;
		entry.last_connect_time = now;

		if (entry.connect_count > m_config.max_connect_per_minute) {
			entry.ban_until = now + (m_config.ban_duration_seconds * 1000LL);
			entry.is_banned = true;
			m_total_blocked++;
			return true;
		}

		return false;
	}

	static bool CheckInvalidPacket(ENetPeer* peer) {
		if (!m_config.enabled || !peer) return false;
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		m_invalid_packet_table[ip]++;

		if (m_invalid_packet_table[ip] > m_config.max_invalid_packets) {
			auto& entry = m_flood_table[ip];
			entry.is_banned = true;
			entry.ban_until = now_ms() + (m_config.ban_duration_seconds * 1000LL);
			m_total_blocked++;
			return true;
		}
		return false;
	}

	static bool CheckLoginFlood(ENetPeer* peer) {
		if (!m_config.enabled || !peer) return false;
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		m_login_attempt_table[ip]++;

		if (m_login_attempt_table[ip] > m_config.max_login_attempts_per_minute) {
			auto& entry = m_connect_table[ip];
			entry.ban_until = now_ms() + (m_config.ban_duration_seconds * 1000LL);
			entry.is_banned = true;
			m_total_blocked++;
			return true;
		}
		return false;
	}

	static bool CheckPayloadSize(ENetPacket* packet) {
		if (!m_config.enabled || !packet) return false;
		return (int)packet->dataLength > m_config.max_payload_size;
	}

	static bool CheckMaxPeersPerIP(ENetPeer* peer) {
		if (!m_config.enabled || !peer) return false;
		std::shared_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		auto it = m_peer_count_table.find(ip);
		if (it == m_peer_count_table.end()) return false;
		return it->second >= m_config.max_peers_per_ip;
	}

	static void OnPeerConnect(ENetPeer* peer) {
		if (!peer) return;
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		m_peer_count_table[ip]++;
	}

	static void OnPeerDisconnect(ENetPeer* peer) {
		if (!peer) return;
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		if (m_peer_count_table[ip] > 0) {
			m_peer_count_table[ip]--;
		}
		if (m_peer_count_table[ip] == 0) {
			m_peer_count_table.erase(ip);
		}
	}

	static bool IsBanned(ENetPeer* peer) {
		if (!m_config.enabled || !peer) return false;
		std::shared_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		long long now = now_ms();

		auto it = m_flood_table.find(ip);
		if (it != m_flood_table.end() && it->second.is_banned) {
			if (now < it->second.ban_until) return true;
		}

		auto it2 = m_connect_table.find(ip);
		if (it2 != m_connect_table.end() && it2->second.is_banned) {
			if (now < it2->second.ban_until) return true;
		}

		return false;
	}

	static void BanIP(ENetPeer* peer, int duration_seconds) {
		if (!peer) return;
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		uint32_t ip = get_ip(peer);
		auto& entry = m_flood_table[ip];
		entry.is_banned = true;
		entry.ban_until = now_ms() + (duration_seconds * 1000LL);
		m_total_blocked++;
	}

	static void UnbanIP(uint32_t ip) {
		std::unique_lock<std::shared_mutex> lock(m_mutex);
		m_flood_table.erase(ip);
		m_connect_table.erase(ip);
		m_text_flood_table.erase(ip);
		m_tank_flood_table.erase(ip);
		m_invalid_packet_table.erase(ip);
		m_login_attempt_table.erase(ip);
	}

	static void Cleanup() {
		long long now = now_ms();
		if (now - m_last_cleanup < m_config.cleanup_interval_seconds * 1000LL) return;
		m_last_cleanup = now;

		std::unique_lock<std::shared_mutex> lock(m_mutex);

		auto cleanup_flood = [&](std::unordered_map<uint32_t, FloodEntry>& table) {
			for (auto it = table.begin(); it != table.end(); ) {
				if (!it->second.is_banned && (now - it->second.first_packet_time > 60000)) {
					it = table.erase(it);
				} else if (it->second.is_banned && now >= it->second.ban_until) {
					it = table.erase(it);
				} else {
					++it;
				}
			}
		};

		cleanup_flood(m_flood_table);
		cleanup_flood(m_text_flood_table);
		cleanup_flood(m_tank_flood_table);

		for (auto it = m_connect_table.begin(); it != m_connect_table.end(); ) {
			if (!it->second.is_banned && (now - it->second.last_connect_time > 120000)) {
				it = m_connect_table.erase(it);
			} else if (it->second.is_banned && now >= it->second.ban_until) {
				it = m_connect_table.erase(it);
			} else {
				++it;
			}
		}

		m_invalid_packet_table.clear();
		m_login_attempt_table.clear();
	}

	static std::string GetStats() {
		std::shared_lock<std::shared_mutex> lock(m_mutex);
		long long total_b = m_total_blocked.load();
		long long total_p = m_total_packets.load();
		int active_bans = 0;
		long long now = now_ms();

		for (auto& [ip, entry] : m_flood_table) {
			if (entry.is_banned && now < entry.ban_until) active_bans++;
		}
		for (auto& [ip, entry] : m_connect_table) {
			if (entry.is_banned && now < entry.ban_until) active_bans++;
		}

		return "`2[Anti-DDoS] `wStats:\n"
		       "`wTotal Packets: `2" + std::to_string(total_p) + "\n"
		       "`wBlocked: `4" + std::to_string(total_b) + "\n"
		       "`wActive Bans: `4" + std::to_string(active_bans) + "\n"
		       "`wTracked IPs: `w" + std::to_string(m_flood_table.size()) + "\n"
		       "`wConnections Tracked: `w" + std::to_string(m_connect_table.size());
	}

	static void ResetStats() {
		m_total_blocked = 0;
		m_total_packets = 0;
	}
};
