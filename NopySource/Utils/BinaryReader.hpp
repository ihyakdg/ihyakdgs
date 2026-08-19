#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <stdexcept>
#include <limits>

// FIX EXPLOIT/CRASH:
// Versi lama tidak menyimpan panjang buffer dan tidak melakukan bounds-check.
// `Read<T>` melakukan memcpy tanpa cek → OOB read trivial dari packet client.
// `ReadStringU16/U32` membaca length dari stream lalu membuat std::string sepanjang
// itu — kalau client kirim length 0xFFFFFFFF, server crash.
//
// Sekarang:
//   * Constructor wajib (atau sangat dianjurkan) menerima `length`.
//   * Read* mengembalikan default-constructed T saat OOB dan memajukan position
//     ke akhir buffer (mencegah looping tak terhingga di caller).
//   * ReadStringU16/U32 mem-clamp panjang string ke sisa buffer.
//   * Semua operasi melakukan saturated arithmetic (anti-overflow di m_position).
class BinaryReader {
public:
    BinaryReader(void* pData, std::size_t length)
        : m_pData(static_cast<uint8_t*>(pData)), m_position(0), m_length(length) {}

    // Backward-compatible constructor (length tidak diketahui).
    // Tidak aman terhadap input untrusted — gunakan overload di atas bila bisa.
    explicit BinaryReader(void* pData)
        : m_pData(static_cast<uint8_t*>(pData)), m_position(0),
          m_length(std::numeric_limits<std::size_t>::max()) {}

    ~BinaryReader() = default;

    template <typename T>
    T Read() {
        T val{};
        if (!m_pData || m_position > m_length || sizeof(T) > m_length - m_position) {
            m_position = m_length; // saturate
            return val;
        }
        std::memcpy(&val, m_pData + m_position, sizeof(T));
        m_position += sizeof(T);
        return val;
    }

    std::string ReadStringU16() {
        uint16_t valLength = this->Read<uint16_t>();
        return ReadStringRaw(valLength);
    }

    std::string ReadStringU32() {
        uint32_t valLength = this->Read<uint32_t>();
        // Hard-cap untuk mencegah alokasi gila walaupun masih dalam buffer.
        if (valLength > (1u << 24)) valLength = (1u << 24); // 16 MB max
        return ReadStringRaw(valLength);
    }

    void Skip(std::size_t length) {
        if (length > m_length - m_position) m_position = m_length;
        else m_position += length;
    }
    std::size_t GetPosition() const { return m_position; }
    std::size_t GetRemaining() const {
        return (m_position > m_length) ? 0 : (m_length - m_position);
    }
    bool Eof() const { return m_position >= m_length; }

private:
    std::string ReadStringRaw(std::size_t valLength) {
        if (!m_pData || m_position > m_length) return std::string{};
        std::size_t avail = m_length - m_position;
        if (valLength > avail) {
            // Truncate ke buffer yang tersedia. Caller dapat verifikasi via GetRemaining().
            valLength = avail;
        }
        std::string val{ reinterpret_cast<char*>(m_pData + m_position), valLength };
        m_position += valLength;
        return val;
    }

    uint8_t* m_pData;
    std::size_t m_position;
    std::size_t m_length;
};
