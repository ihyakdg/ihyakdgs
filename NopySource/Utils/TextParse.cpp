#include <Utils/TextParse.hpp>

TextParse::TextParse() : m_data() {}
TextParse::TextParse(const std::string& string) { 
    this->Parse(string); 
}
TextParse::TextParse(const std::vector<std::pair<std::string, std::string>>& data) {
    m_data.reserve(data.size());
    for (const auto& it : data)
        this->Add(it.first, it.second);
}
TextParse::~TextParse() { m_data.clear(); }

std::vector<std::string> TextParse::StringTokenize(const std::string& string, const std::string& delimiter) {
    std::vector<std::string> tokens{};
    tokens.reserve(8);
    size_t previousPos = 0, currentPos = 0;
    do {
        currentPos = string.find(delimiter, previousPos);
        if (currentPos == std::string::npos) currentPos = string.length();
        std::string token = string.substr(previousPos, currentPos - previousPos);
        if (!token.empty()) tokens.push_back(token);
        previousPos = currentPos + delimiter.length();
    }
    while (currentPos < string.length() && previousPos < string.length());
    return tokens;
}

void TextParse::Parse(const std::string& string) {
    m_data = TextParse::StringTokenize(string, "\n");
    for (auto& iterator : m_data)
        iterator.erase(std::remove(iterator.begin(), iterator.end(), '\r'), iterator.end());
}

std::string TextParse::Get(const std::string& key, int index, const std::string& token, int key_index) {
    if (m_data.empty())
        return std::string{ "" };

    for (const auto& iterator : m_data) {
        if (iterator.empty())
            continue;

        if (key_index == 0 && !token.empty()) {
            if (iterator.compare(0, key.length(), key) == 0) {
                if (iterator.length() == key.length()) {
                    if (index == 0) return key;
                    continue;
                }
                if (iterator.compare(key.length(), token.length(), token) == 0) {
                    size_t start = key.length() + token.length();
                    int curIndex = 1;
                    while (curIndex < index) {
                        size_t nextPos = iterator.find(token, start);
                        if (nextPos == std::string::npos) break;
                        start = nextPos + token.length();
                        curIndex++;
                    }
                    if (curIndex == index && start <= iterator.length()) {
                        size_t endPos = iterator.find(token, start);
                        if (endPos == std::string::npos) endPos = iterator.length();
                        return iterator.substr(start, endPos - start);
                    }
                }
            }
        } else {
            std::vector<std::string> tokenize = TextParse::StringTokenize(iterator, token);
            if (key_index < 0 || static_cast<size_t>(key_index) >= tokenize.size())
                continue;
            if (tokenize[key_index] != key)
                continue;
            if (index < 0)
                return std::string{ "" };
            size_t target = static_cast<size_t>(key_index) + static_cast<size_t>(index);
            if (target >= tokenize.size())
                continue;
            return tokenize[target];
        }
    }
    return std::string{ "" };
}

template <typename T, typename std::enable_if_t<std::is_floating_point_v<T>, bool>>
T TextParse::Get(const std::string& key, int index, const std::string& token) {
    try {
        std::string val = this->Get(key, index, token);
        if (val.empty()) return static_cast<T>(0);
        if constexpr (std::is_same_v<T, double>)
            return std::stod(val);
        else if constexpr (std::is_same_v<T, long double>)
            return std::stold(val);
        else
            return std::stof(val);
    } catch (...) {
        return static_cast<T>(0);
    }
}

void TextParse::Set(const std::string& key, const std::string& value, const std::string& token) {
    if (m_data.empty())
        return;
    for (auto& iterator : m_data) {
        if (iterator.compare(0, key.length(), key) == 0) {
            if (iterator.length() == key.length()) {
                iterator = key + token + value;
                return;
            }
            if (iterator.compare(key.length(), token.length(), token) == 0) {
                iterator = key + token + value;
                return;
            }
        }
    }
    this->Add(key, value, token);
}
template <typename T, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool>>
void TextParse::Set(const std::string& key, const T& value, const std::string& token) {
    this->Set(key, std::to_string(value), token);
}

bool TextParse::IsEmpty() const {
    return m_data.empty();
}

size_t TextParse::GetSize() const {
    return m_data.size();
}
std::string TextParse::GetAsString() const {
    std::string ret{};
    size_t total_len = 0;
    for (const auto& line : m_data) total_len += line.length() + 1;
    ret.reserve(total_len);
    for (const auto& line : m_data) {
        ret.append(line);
        ret.push_back('\n');
    }
    return ret;
}
