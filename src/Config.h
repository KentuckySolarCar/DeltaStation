/* date = July 09, 2025 5:35 PM */


#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <toml++/toml.hpp>

namespace DS {
    class Graph;

    // forward decl.
    namespace Expr {
        struct AST;
    }

    /**
     * Class for specifying user-based configuration of Delta Station.
     *
     * This class serves for three configuration functions:
     * - Buffer layout: configurable buffer layouts for flexible packet reordering on the fly.
     * - Logging config: options for specifying log file semantics.
     * - Expression graphing: objects for graphing custom expressions.
     */
    class Config {
    public:
        enum FieldType {
            I8,
            I16,
            I32,
            I64,
            U8,
            U16,
            U32,
            U64,
            F32,
            F64,
        };

        /**
         * A specific subregion of a buffer. It holds the offset in the parent buffer and the size of the data in that buffer.
         */
        struct Field {
            ptrdiff_t offset;
            ptrdiff_t size;
            void *data;
            FieldType ty;
        };

        /**
         * A buffer that an entire packet sent over telemetry can fit into.
         *
         * Each buffer is split up into Fields, much like how a class has fields.
         */
        class Entry {
        public:
            /**
             * Get a field based on the name provided in the config file.
             * @param str Name of queried field.
             * @return A slice of the buffer in a Field object, or std::nullopt if name not found.
             */
            std::optional<Field> get(const std::string &str) const {
                if (!name_idx_pairs.contains(str)) {
                    return std::nullopt;
                }
                const Field f = name_idx_pairs.at(str);
                return {f};
            }

            /**
             * Get the pointer to the buffer as a `uint8_t *`.
             * @return Raw pointer of buffer.
             */
            uint8_t *as_ptr() const {
                return back;
            }

            /**
             * Get the number of bytes the buffer can hold.
             * @return Size of buffer.
             */
            size_t get_size() const {
                return size;
            }

            /**
             * Return the value at the memory location of the field as specified in the config file.
             * @tparam T Type the data should be interpreted as.
             * @param str Name of the field you want the data of.
             * @return The value at the location of that field, or std::nullopt if name not found.
             */
            template<typename T>
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

        /**
         * Constructs a new Config object, with configuration file at `filepath`.
         * @param filepath Filepath for user-inputted configuration file.
         */
        explicit Config(const std::string &filepath);

        ~Config() = default;


        /**
         * Finds a buffer in the buffer parser based on its name.
         * @param id unique string name for buffer
         * @return The buffer with name `id`, or nullptr if not found.
         */
        Entry *get(const std::string &id) {
            if (!id_name_pairs.contains(id)) {
                return nullptr;
            }
            return &id_name_pairs.at(id);
        }

        /**
         * Finds a buffer in the buffer parser based on its name.
         * @param id unique string name for buffer
         * @return Const-qualified buffer with name `id`, or nullptr if not found.
         */
        [[nodiscard]] const Entry *get(const std::string &id) const {
            if (!id_name_pairs.contains(id)) {
                return nullptr;
            }
            return &id_name_pairs.at(id);
        }

        [[nodiscard]] std::optional<std::string> get_id(const size_t id) const {
            for (auto &[first, second]: *config["ds"].as_table()) {
                const auto buffer_table = second.as_table();
                if (!buffer_table)
                    continue;
                const auto field_value = buffer_table->get("id");
                if (!field_value)
                    continue;
                if (field_value->value<size_t>() == id) {
                    return std::string(first);
                }
            }
            return std::nullopt;
        }

        Entry &operator[](const std::string &id) {
            Entry *e = get(id);
            if (e) return *e;
            std::cerr << "Could not get buffer type with id '" << id << "'!\n";
            exit(-1);
        }

        const Entry &operator[](const std::string &str) const {
            const Entry *e = get(str);
            if (e) return *e;
            std::cerr << "Could not get buffer type with name " << str << "!\n";
            exit(-1);
        }

        /**
         * Match a typename as found in config file with a size in bytes for the type.
         *
         * For example, `type_size("f32") == 4` and `type_size("u16") = 2`.
         * @param type type string as found in config file.
         * @return Size of the type, or std::nullopt if type invalid.
         */
        static std::optional<size_t> type_size(const std::string &type);

    private:
        // Helper function for generating buffers
        static void populate_buffer(const std::string &key, const toml::table &val, Entry &e);

        static std::optional<const char *>field_type_to_str(const FieldType f) {
            switch (f) {
                case I8:
                    return "i8";
                case I16:
                    return "i16";
                case I32:
                    return "i32";
                case I64:
                    return "i64";
                case U8:
                    return "u8";
                case U16:
                    return "u16";
                case U32:
                    return "u32";
                case U64:
                    return "u64";
                case F32:
                    return "f32";
                case F64:
                    return "f64";
            }
            std::cout << "Invalid FieldType enum " << f << "!\n";
            return std::nullopt;
        }

        static std::optional<FieldType> str_to_field_type(const std::string &str) {
            if (str == "i8")
                return I8;
            if (str == "i16")
                return I16;
            if (str == "i32")
                return I32;
            if (str == "i64")
                return I64;
            if (str == "u8")
                return U8;
            if (str == "u16")
                return U16;
            if (str == "u32")
                return U32;
            if (str == "u64")
                return U64;
            if (str == "f32")
                return F32;
            if (str == "f64")
                return F64;
            std::cout << "Invalid type " << str << "for expression parser!\n";
            return std::nullopt;
        }

        std::string config_path;
        toml::table config;
        // packet management
        std::map<std::string, Entry> id_name_pairs;
        // output management
        std::string output_path;
        bool output_enabled = false;

        // graph management
        std::vector<Graph> graphs;

        friend class Dashboard;
    };
} // DS

#endif //CONFIG_H
