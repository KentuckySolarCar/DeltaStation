/* date = July 09, 2025 5:35 PM */


#include "Config.h"

#include <iostream>
#include <iosfwd>

#include "Graph.h"

namespace DS {
    Config::Config(const std::string &filepath) {
        config = toml::parse_file(filepath);
        output_enabled = config["logger"]["status"].value_or(false);
        output_path = config["logger"]["output"].value_or("out.csv");

        std::cout << config << '\n';

        config["ds"].as_table()->for_each([this](const toml::key &key, const toml::table &val) {
            const auto id = val["id"].as_integer()->value_or(-1);
            if (id < 0 || !id) {
                std::cerr << "Invalid or missing id for key " << key << "!\n";
                exit(-1);
            }

            id_name_pairs[get_id(id).value()] = Entry{};

            std::cout << "Initializing key " << key << " to id #" << get_id(id).value() << ".\n";
            populate_buffer(static_cast<std::string>(key.str()), val, id_name_pairs[get_id(id).value()]);
        });

        config["graph"].as_table()->for_each([this](const toml::key &key, const toml::table &val) {
            const auto k = key.str();

            graphs.emplace_back(
                k.data(),
                val["expr"].as_string()->value_or(""),
                val["length"].as_floating_point()->value_or(0.0)
            );
        });
    }

    void Config::populate_buffer(const std::string &key, const toml::table &val, Entry &e) {
        // -1 to account for ID field
        std::vector<std::string> key_order{val.size() - 1};
        size_t buffer_size = 0;
        val.for_each([key, &buffer_size, &key_order, &e](const toml::key &fkey, const toml::table &field) {
            auto *type = field["type"].as_string();
            if (!type) {
                std::cerr << "Missing type for field " << key << "." << fkey << "\n";
                exit(-1);
            }
            const auto size = type_size(type->get());
            if (!size) {
                std::cerr << "Invalid type length " << type << " for field " << key << "." << fkey << "\n";
                exit(-1);
            }
            const std::optional<FieldType> ty = str_to_field_type(type->get());
            if (!ty) {
                std::cerr << "Invalid type string " << type << " for field " << key << "." << fkey << "\n";
                exit(-1);
            }

            const auto order = field["order"].as_integer();

            if (!order) {
                std::cerr << "Invalid order " << order << " for field " << key << "." << fkey << "\n";
                exit(-1);
            }
            const auto name = std::string(fkey.str());
            key_order[order->get()] = name;
            e.name_idx_pairs[name].size = static_cast<long>(*size);
            e.name_idx_pairs[name].ty = *ty;
            buffer_size += *size;
        });

        e.back = new uint8_t[buffer_size];
        e.size = buffer_size;

        // TODO: this is unnecessary debug.
        size_t offset = 0;
        for (const std::string &str: key_order) {
            auto &[e_offset, size, data, ty] = e.name_idx_pairs[str];
            e_offset = static_cast<long>(offset);
            offset += size;
            data = reinterpret_cast<char *>(e.back) + e_offset;
            std::cout << key << "." << str << ": " << size << "@" << e_offset << "\n";
        }
    }

    std::optional<size_t> Config::type_size(const std::string &type) {
        std::stringstream t_size;
        t_size << type.substr(1);
        size_t size;
        t_size >> size;
        if (t_size.fail()) {
            return std::nullopt;
        }
        return size / CHAR_BIT;
    }
} // DS
