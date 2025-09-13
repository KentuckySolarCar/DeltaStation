/* date = July 09, 2025 5:35 PM */


#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <toml++/toml.hpp>

#include "BufferParser.h"

namespace DS {
    namespace Expr {
        struct AST;
    }

    class Config {
    public:
        struct Field {
            ptrdiff_t offset;
            ptrdiff_t size;
            void *data;
        };

        class Entry {
        public:
            std::optional<Field> get(const std::string &str) const {
                if (!name_idx_pairs.contains(str)) {
                    return std::nullopt;
                }
                const Field f = name_idx_pairs.at(str);
                return {f};
            }

            uint8_t *as_ptr() const {
                return back;
            }

            size_t get_size() const {
                return size;
            }

            template <typename T>
            std::optional<T> get_value(const std::string &str) const {
                auto f = get(str);
                if (!f.has_value()) {
                    return std::nullopt;
                }
                return std::optional(*static_cast<T *>(f.value().data));
            }

        private:
            uint8_t *back{};
            size_t size{};
            std::map<std::string, Field> name_idx_pairs;

            friend class Config;
        };

        Config() = default;

        explicit Config(const std::string &filepath);

        ~Config() = default;


        Entry *get(const BufferParser::BufferType ty) {
            if (!id_name_pairs.contains(ty)) {
                return nullptr;
            }
            return &id_name_pairs.at(ty);
        }

        const Entry *get(const BufferParser::BufferType ty) const {
            if (!id_name_pairs.contains(ty)) {
                return nullptr;
            }
            return &id_name_pairs.at(ty);
        }

        Entry &operator[](const BufferParser::BufferType ty) {
            Entry *e = get(ty);
            if (e) return *e;
            std::cerr << "Could not get buffer type with id " << ty << "!\n";
            exit(-1);
        }

        Entry &operator[](const std::string &str) {
            if (auto e = get(BufferParser::from_str(str))) return *e;
            std::cerr << "Could not get buffer type with name " << str << "!\n";
            exit(-1);
        }

        const Entry &operator[](const std::string &str) const {
            const Entry *e = get(
                BufferParser::from_id(
                    config[str]["id"].as_integer()->value_or(0)
                ).value()
            );
            if (e) return *e;
            std::cerr << "Could not get buffer type with name " << str << "!\n";
            exit(-1);
        }

        static void populate_buffer(const std::string &key, const toml::table &val, Entry &e);

        static std::optional<size_t> type_size(const std::string &type);

    private:
        toml::table config;
        // packet management
        std::map<BufferParser::BufferType, Entry> id_name_pairs;
        // output management
        std::string output_path;
        bool output_enabled = false;

        // graph management
        std::vector<Graph> graphs;

        friend class Dashboard;
    };
} // DS

#endif //CONFIG_H
