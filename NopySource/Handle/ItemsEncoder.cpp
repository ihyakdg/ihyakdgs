#include "ItemsEncoder.h"

// KODE INI TIDAK UNTUK DIPERJUAL BELIKAN, DI CIPTAKAN OLEH GuckTubeYT DAN DI UBAH KE C++/CPP OLEH GROWPLUS DEVELOPER TEAM
std::string ItemsEncoder::items_secret_key = "PBG892FXX982ABC*";
json ItemsEncoder::data_json = {};
std::vector<uint8_t> ItemsEncoder::encoded_buffer_file = {};
std::string ItemsEncoder::msg = "";

std::vector<std::string> ItemsEncoder::byteToHex = {};

void ItemsEncoder::initByteToHex() {
    byteToHex.clear();
    for (int n = 0; n <= 0xff; ++n) {
        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0') << n;
        byteToHex.push_back(ss.str());
    }
}

std::string ItemsEncoder::hex(const std::vector<uint8_t>& arrayBuffer, bool is_without_space) {
    std::string result;
    result.reserve(arrayBuffer.size() * (is_without_space ? 2 : 3));
    for (size_t i = 0; i < arrayBuffer.size(); ++i) {
        result += byteToHex[arrayBuffer[i]];
        if (!is_without_space && i < arrayBuffer.size() - 1) {
            result.push_back(' ');
        }
    }
    return result;
}

int ItemsEncoder::read_buffer_number(const std::vector<uint8_t>& buffer, int pos, int len) {
    int value = 0;
    for (int a = 0; a < len; a++) {
        value += buffer[pos + a] << (a * 8);
    }
    return value;
}

void ItemsEncoder::write_buffer_number(int pos, int len, int value) {
    for (int a = 0; a < len; a++) {
        encoded_buffer_file[pos + a] = (value >> (a * 8)) & 255;
    }
}

void ItemsEncoder::write_buffer_string(int pos, int len, const std::string& value, bool using_key, int item_id) {
    for (int a = 0; a < len; a++) {
        if (using_key) {
            encoded_buffer_file[pos + a] = value[a] ^ (items_secret_key[(a + item_id) % items_secret_key.length()]);
        } else {
            encoded_buffer_file[pos + a] = value[a];
        }
    }
}

void ItemsEncoder::hash_buffer(const std::vector<uint8_t>& buffer, const std::string& element, const std::string& text) {
    unsigned int hash = 0x55555555;
    for (size_t a = 0; a < buffer.size(); a++) {
        hash = (hash >> 27) + (hash << 5) + buffer[a];
    }
    msg = text + std::to_string(hash);
}

void ItemsEncoder::hexStringToArrayBuffer(int pos, const std::string& hexString) {
    // remove the space
    std::string cleanHex = std::regex_replace(hexString, std::regex(" "), "");
    
    if (cleanHex.length() % 2 != 0) {
        std::cout << "WARNING: expecting an even number of characters in the hexString" << std::endl;
    }
    
    // check for some non-hex characters
    std::regex nonHexPattern("[G-Z\\s]", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_search(cleanHex, match, nonHexPattern)) {
        std::cout << "WARNING: found non-hex characters" << std::endl;
    }

    // convert the octets to integers
    std::regex hexPairPattern("[\\dA-F]{2}", std::regex_constants::icase);
    auto words_begin = std::sregex_iterator(cleanHex.begin(), cleanHex.end(), hexPairPattern);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::string hexPair = (*i).str();
        encoded_buffer_file[pos++] = std::stoi(hexPair, nullptr, 16);
    }
}

std::string ItemsEncoder::read_buffer_string(const std::vector<uint8_t>& buffer, int pos, int len, bool using_key, int item_id) {
    std::string result;
    if (len <= 0) return result;
    result.reserve(len);
    if (using_key) {
        size_t key_len = items_secret_key.length();
        for (int a = 0; a < len; a++) {
            result.push_back(static_cast<char>(buffer[a + pos] ^ items_secret_key[(item_id + a) % key_len]));
        }
    } else {
        for (int a = 0; a < len; a++) {
            result.push_back(static_cast<char>(buffer[a + pos]));
        }
    }
    return result;
}

bool ItemsEncoder::check_last_char(const std::string& dest, char src) {
    return !dest.empty() && dest[dest.length() - 1] == src;
}

void ItemsEncoder::process_item_encoder(const json& result) {
    int mem_pos = 6;

    write_buffer_number(0, 2, result["version"]);
    write_buffer_number(2, 4, result["item_count"]);
    
    for (int a = 0; a < result["item_count"]; a++) {
        const json& item = result["items"][a];
        
        write_buffer_number(mem_pos, 4, item["item_id"]);
        mem_pos += 4;
        encoded_buffer_file[mem_pos++] = item["editable_type"];
        encoded_buffer_file[mem_pos++] = item["item_category"];
        encoded_buffer_file[mem_pos++] = item["action_type"];
        encoded_buffer_file[mem_pos++] = item["hit_sound_type"];
        
        std::string name = item["name"];
        write_buffer_number(mem_pos, 2, name.length());
        mem_pos += 2;
        write_buffer_string(mem_pos, name.length(), name, true, item["item_id"]);
        mem_pos += name.length();
        
        std::string texture = item["texture"];
        write_buffer_number(mem_pos, 2, texture.length());
        mem_pos += 2;
        write_buffer_string(mem_pos, texture.length(), texture);
        mem_pos += texture.length();
        
        write_buffer_number(mem_pos, 4, item["texture_hash"]);
        mem_pos += 4;
        encoded_buffer_file[mem_pos++] = item["item_kind"];
        write_buffer_number(mem_pos, 4, item["val1"]);
        mem_pos += 4;
        encoded_buffer_file[mem_pos++] = item["texture_x"];
        encoded_buffer_file[mem_pos++] = item["texture_y"];
        encoded_buffer_file[mem_pos++] = item["spread_type"];
        encoded_buffer_file[mem_pos++] = item["is_stripey_wallpaper"];
        encoded_buffer_file[mem_pos++] = item["collision_type"];

        std::string break_hits_str = item["break_hits"].dump();
        if (check_last_char(break_hits_str, 'r')) {
            encoded_buffer_file[mem_pos++] = std::atoi(break_hits_str.substr(0, break_hits_str.length() - 2).c_str());
        } else {
            encoded_buffer_file[mem_pos++] = std::atoi(break_hits_str.c_str()) * 6;
        }

        write_buffer_number(mem_pos, 4, item["drop_chance"]);
        mem_pos += 4;
        encoded_buffer_file[mem_pos++] = item["clothing_type"];
        write_buffer_number(mem_pos, 2, item["rarity"]);
        mem_pos += 2;
        
        encoded_buffer_file[mem_pos++] = item["max_amount"];
        
        std::string extra_file = item["extra_file"];
        write_buffer_number(mem_pos, 2, extra_file.length());
        mem_pos += 2;
        write_buffer_string(mem_pos, extra_file.length(), extra_file);
        mem_pos += extra_file.length();
        
        write_buffer_number(mem_pos, 4, item["extra_file_hash"]);
        mem_pos += 4;
        write_buffer_number(mem_pos, 4, item["audio_volume"]);
        mem_pos += 4;
        
        std::string pet_name = item["pet_name"];
        write_buffer_number(mem_pos, 2, pet_name.length());
        mem_pos += 2;
        write_buffer_string(mem_pos, pet_name.length(), pet_name);
        mem_pos += pet_name.length();
        
        std::string pet_prefix = item["pet_prefix"];
        write_buffer_number(mem_pos, 2, pet_prefix.length());
        mem_pos += 2;
        write_buffer_string(mem_pos, pet_prefix.length(), pet_prefix);
        mem_pos += pet_prefix.length();
        
        std::string pet_suffix = item["pet_suffix"];
        write_buffer_number(mem_pos, 2, pet_suffix.length());
        mem_pos += 2;
        write_buffer_string(mem_pos, pet_suffix.length(), pet_suffix);
        mem_pos += pet_suffix.length();
        
        std::string pet_ability = item["pet_ability"];
        write_buffer_number(mem_pos, 2, pet_ability.length());
        mem_pos += 2;
        write_buffer_string(mem_pos, pet_ability.length(), pet_ability);
        mem_pos += pet_ability.length();
        
        encoded_buffer_file[mem_pos++] = item["seed_base"];
        encoded_buffer_file[mem_pos++] = item["seed_overlay"];
        encoded_buffer_file[mem_pos++] = item["tree_base"];
        encoded_buffer_file[mem_pos++] = item["tree_leaves"];
        encoded_buffer_file[mem_pos++] = item["seed_color"]["a"];
        encoded_buffer_file[mem_pos++] = item["seed_color"]["r"];
        encoded_buffer_file[mem_pos++] = item["seed_color"]["g"];
        encoded_buffer_file[mem_pos++] = item["seed_color"]["b"];
        encoded_buffer_file[mem_pos++] = item["seed_overlay_color"]["a"];
        encoded_buffer_file[mem_pos++] = item["seed_overlay_color"]["r"];
        encoded_buffer_file[mem_pos++] = item["seed_overlay_color"]["g"];
        encoded_buffer_file[mem_pos++] = item["seed_overlay_color"]["b"];
        
        write_buffer_number(mem_pos, 4, 0); // skipping ingredients
        mem_pos += 4;
        write_buffer_number(mem_pos, 4, item["grow_time"]);
        mem_pos += 4;
        write_buffer_number(mem_pos, 2, item["val2"]);
        mem_pos += 2;
        write_buffer_number(mem_pos, 2, item["is_rayman"]);
        mem_pos += 2;
        
        std::string extra_options = item["extra_options"];
        write_buffer_number(mem_pos, 2, extra_options.length());
        mem_pos += 2;
        write_buffer_string(mem_pos, extra_options.length(), extra_options);
        mem_pos += extra_options.length();
        
        std::string texture2 = item["texture2"];
        write_buffer_number(mem_pos, 2, texture2.length());
        mem_pos += 2;
        write_buffer_string(mem_pos, texture2.length(), texture2);
        mem_pos += texture2.length();
        
        std::string extra_options2 = item["extra_options2"];
        write_buffer_number(mem_pos, 2, extra_options2.length());
        mem_pos += 2;
        write_buffer_string(mem_pos, extra_options2.length(), extra_options2);
        mem_pos += extra_options2.length();
        
        hexStringToArrayBuffer(mem_pos, item["data_position_80"]);
        mem_pos += 80;
        
        if (result["version"] >= 11) {
            std::string punch_options = item["punch_options"];
            write_buffer_number(mem_pos, 2, punch_options.length());
            mem_pos += 2;
            write_buffer_string(mem_pos, punch_options.length(), punch_options);
            mem_pos += punch_options.length();
        }
        
        if (result["version"] >= 12) {
            hexStringToArrayBuffer(mem_pos, item["data_version_12"]);
            mem_pos += 13;
        }
        
        if (result["version"] >= 13) {
            write_buffer_number(mem_pos, 4, item["int_version_13"]);
            mem_pos += 4;
        }
        
        if (result["version"] >= 14) {
            write_buffer_number(mem_pos, 4, item["int_version_14"]);
            mem_pos += 4;
        }
        
        if (result["version"] >= 15) {
            hexStringToArrayBuffer(mem_pos, item["data_version_15"]);
            mem_pos += 25;
            
            std::string str_version_15 = item["str_version_15"];
            write_buffer_number(mem_pos, 2, str_version_15.length());
            mem_pos += 2;
            write_buffer_string(mem_pos, str_version_15.length(), str_version_15);
            mem_pos += str_version_15.length();
        }
        
        if (result["version"] >= 16) {
            std::string str_version_16 = item["str_version_16"];
            write_buffer_number(mem_pos, 2, str_version_16.length());
            mem_pos += 2;
            write_buffer_string(mem_pos, str_version_16.length(), str_version_16);
            mem_pos += str_version_16.length();
        }
        
        if (result["version"] >= 17) {
            write_buffer_number(mem_pos, 4, item["int_version_17"]);
            mem_pos += 4;
        }
        
        if (result["version"] >= 18) {
            write_buffer_number(mem_pos, 4, item["int_version_18"]);
            mem_pos += 4;
        }
        
        if (result["version"] >= 19) {
            write_buffer_number(mem_pos, 4, item["int_version_19"]);
            mem_pos += 9;
        }
    }
}

std::vector<uint8_t> ItemsEncoder::item_encoder(const std::string& file_content) {
    encoded_buffer_file.clear();
    
    // Ensure buffer is large enough
    encoded_buffer_file.resize(10000000); // Pre-allocate a large buffer
    
    json result = json::parse(file_content);
    
    process_item_encoder(result);
    
    hash_buffer(encoded_buffer_file, "items_dat_hash_1", "Encoded Items dat Hash: ");
    
    // Resize to actual used size
    size_t actual_size = 0;
    for (size_t i = 0; i < encoded_buffer_file.size(); i++) {
        if (encoded_buffer_file[i] != 0) {
            actual_size = i + 1;
        }
    }
    encoded_buffer_file.resize(actual_size);
    
    return encoded_buffer_file;
}

void ItemsEncoder::encode_items_dat(const std::string& input_file, const std::string& output_file) {
    try {
        // Read input file
        std::ifstream file(input_file, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open input file: " << input_file << std::endl;
            return;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        // Initialize byte to hex conversion
        initByteToHex();
        
        // Encode the data
        std::vector<uint8_t> jsonData = item_encoder(buffer.str());
        
        // Create output directory if it doesn't exist
        fs::path output_path(output_file);
        fs::create_directories(output_path.parent_path());
        
        // Write output file
        std::ofstream outfile(output_file, std::ios::binary);
        if (!outfile.is_open()) {
            std::cerr << "Failed to open output file: " << output_file << std::endl;
            return;
        }
        
        outfile.write(reinterpret_cast<const char*>(jsonData.data()), jsonData.size());
        outfile.close();
        
        std::cout << msg << std::endl;
        std::cout << "Successfully encoded items.dat to: " << output_file << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error processing JSON: " << e.what() << std::endl;
    }

    items_secret_key.clear();
    items_secret_key.shrink_to_fit();

    data_json.clear();
    json().swap(data_json);

    encoded_buffer_file.clear();
    encoded_buffer_file.shrink_to_fit();

    msg.clear();
    msg.shrink_to_fit();
    byteToHex.shrink_to_fit();
}