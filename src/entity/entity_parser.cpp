#include "entity_parser.h"
#include "../test.h"
#include "entity_types.h"

#include <cstring>
#include <functional>
#include <cxxabi.h>
#include <limits>

//
// # <type>
// <field> <value?>
//

EntityType string_to_entity_type(const char *str) {
    for (int t = 0; t < LEN(entity_type_names); t++) {
        if (std::strcmp(str, entity_type_names[t]) == 0) {
            return EntityType(t);
        }
    }
    return EntityType::NUM_ENTITY_TYPES;
}

struct FileParser {

    FileParser(const char *filename, const char *data)
        : filename(filename)
        , data(data) {}

    const char *filename;
    const char *data;

    int line = 1; // Lines start at 1 for some reason.
    int column = 0;
    int head = 0;

    // Show the current symbol.
    char peek() {
        return data[head];
    }

    // Return the current symbol and skipp to the next one.
    char eat() {
        char c = peek();
        if (c) {
            column++;
            if (c == '\n') {
                line++;
                column = 0;
            }
            return data[head++];
        }
        return peek();
    }

    // Return if we are looking at whitespace.
    bool is_whitespace() {
        char c = peek();
        return c == '\n' || c == ' ';
    }

    void skipp_whitespace() {
        while (is_whitespace()) { eat(); }
    }

    void skipp_line() {
        char c;
        do {
            c = eat();
        } while (c != '\n' && c != '\0');
    }

    void error(const char *message, const char *extra = nullptr) {
        char faulty_line[128] = {};

        u32 sol = head;
        while (data[sol] != '\n' && sol != 0) { sol--; }
        sol++;
        for (int i = 0;
             i < LEN(faulty_line)
             && data[sol + i] != '\n'
             && data[sol + i] != '\0';
             i++) {
            faulty_line[i] = data[sol + i];
        }

        if (extra) {
            ERR("\n" RED "== ENTITY PARSE ERROR ==" RESET
                "\n{} ({}:{}): " BLUE "{} " RESET "'{}'"
                "\n" BLUE ">>>" RESET " {}",
                filename, line, column, message, extra,
                faulty_line);
        } else {
            ERR("\n" RED "== ENTITY PARSE ERROR ==" RESET
                "\n{} ({}:{}): " BLUE "{}" RESET
                "\n" BLUE ">>>" RESET " {}",
                filename, line, column, message,
                faulty_line);
        }
    }

    // Writes until next whitespace into the buffer, or until
    // the buffer runs out.
    u32 eat_word(char *buff, u32 len) {
        u32 i = 0;
        for (; !is_whitespace() && peek(); i++) {
            if (i == len) {
                error("Buffer too small!");
                buff[i - 1] = '\0';
                return i;
            }
            buff[i] = eat();
        }
        buff[i + 1] = '\0';
        return i;
    }

    // Parses an entity type name, returns NUM_ENTITY_TYPES on
    // error.
    EntityType parse_entity_type() {
        skipp_whitespace();
        if (peek() == '#') {
            eat();
            skipp_whitespace();

            FileParser before_read = *this;

            char typestring[32] = {};
            eat_word(typestring, LEN(typestring));
            EntityType type = string_to_entity_type(typestring);

            if (type == EntityType::NUM_ENTITY_TYPES) {
                before_read.error("Unknown entity type!", typestring);
                return EntityType::NUM_ENTITY_TYPES;
            }
            return type;
        } else if (!peek()) {
            return EntityType::NUM_ENTITY_TYPES;
        }
        error("Unexpected symbol, expected new entity.");
        skipp_line();
        return EntityType::NUM_ENTITY_TYPES;
    }

    Field *parse_field(FieldList fields) {
        FileParser before_read = *this;

        char fieldname[32] = {};
        eat_word(fieldname, LEN(fieldname));

        for (int i = 0; i < fields.num_fields; i++) {
            Field f = fields.list[i];
            if (std::strcmp(fieldname, f.name) == 0) {
                if (f.internal) {
                    before_read.error("Field is marked as internal thus cannot be set.", fieldname);
                    skipp_line();
                    return nullptr;
                }
                return fields.list + i;
            }
        }

        before_read.error("No such field.", fieldname);
        skipp_line();
        return nullptr;
    }

    f64 parse_float() {
        char floatstr[32] = {};
        eat_word(floatstr, LEN(floatstr));
        try {
            return std::stod(floatstr);
        } catch (std::invalid_argument *i) {
            error("Failed to parse as float", floatstr);
            return 0;
        }
    }

    i64 parse_int() {
        char floatstr[32] = {};
        eat_word(floatstr, LEN(floatstr));
        try {
            return std::stoll(floatstr);
        } catch (std::invalid_argument *i) {
            error("Failed to parse as int", floatstr);
            return 0;
        }
    }
};

using ParseFunc = std::function<void(FileParser, void *)>;
std::unordered_map<std::size_t, ParseFunc> parse_funcs;

namespace ParseFuncs {

void parse_empty(FileParser p, void *d) {}

template <typename T>
concept Ints = std::numeric_limits<T>().is_integer;

template <Ints T>
void parse(FileParser p, void *d) {
    auto limit = std::numeric_limits<T>();
    FileParser before_read = p;

    T *out = (T *)d;
    // Assumes all values we want to store fit in a 64 bit integer.
    i64 value = p.parse_int();
    if (limit.min() > value && limit.max() < value) {
        char buffer[128];
        sntprint(buffer, LEN(buffer),
                 "Won't fit. {} not in [{},{}]. Default to 0.",
                 value, limit.min(), limit.max());
        before_read.error(buffer);
        *out = 0;
    }
    *out = (T)value;
}

template <typename T>
concept Reals = std::numeric_limits<T>().epsilon() != 0;

template <Reals T>
void parse(FileParser p, void *d) {
    T *out = (T *)d;
    p.skipp_whitespace();
    *out = p.parse_float();
}

template <VectorType T>
void parse(FileParser p, void *d) {
    T *out = (T *)d;
    for (int i = 0; i < DIM<T>(); i++) {
        // NOTE(ed): Thought about restricting colors to
        // be in range [0, 1].
        p.skipp_whitespace();
        out->_[i] = p.parse_float();
    }
}

template <>
void parse<bool>(FileParser p, void *d) {
    TRACE("Parsing bool!");
    bool *out = (bool *)d;
    p.skipp_whitespace();
    *out = p.parse_int() != 0;
}
}

void initalize_parse_funcs() {
#define F(T) parse_funcs[typeid(T).hash_code()] = ParseFuncs::parse<T>;
    // clang-format off
    F(i64) F(i32) F(i16) F(i8)
    F(u64) F(u32) F(u16) F(u8)
    F(f32) F(f64)
    F(bool)
    F(Vec2) F(Vec3) F(Vec4)
    // clang-format on
#undef F
}

std::vector<BaseEntity *>
parse_entities(const char *data) {
    std::vector<BaseEntity *> entities;

    initalize_parse_funcs();

    FileParser parser("UNKOWN", data);
    while (parser.peek()) {
        EntityType type = parser.parse_entity_type();
        if (type == EntityType::NUM_ENTITY_TYPES) continue;

        u8 *entity = new u8[MAX_ENTITY_SIZE];
        entities.push_back((BaseEntity *)entity);

        emplace_entity((void *)entity, type);
        FieldList fields = get_fields_for(type);

        parser.skipp_whitespace();
        for (; parser.peek() && parser.peek() != '#'; parser.skipp_whitespace()) {
            Field *field = parser.parse_field(fields);
            if (!field) continue;

            std::size_t hash = field->typeinfo.hash_code();
            if (parse_funcs.contains(hash)) {
                parse_funcs[hash](parser, (void *)(entity + field->offset));
            } else {
                const char *name = field->typeinfo.name();
                int status;
                char *demangled = abi::__cxa_demangle(name, 0, 0, &status);
                ASSERT_EQ(status, 0);
                ERR("Failed to 'parse' field '{}' with unsupported type '{}'",
                    field->name, demangled);
                parse_funcs[hash] = ParseFuncs::parse_empty;
                delete[] demangled;
            }

            // TODO(ed): Parse the actual data and set it.
            parser.skipp_line();
        }
    }

    return entities;
}

TEST_CASE("parser-simple", {
    Entity::BaseEntity *e = parse_entities("# Light\nposition 1 2 3\ndraw_as_point 1\n")[0];
    ASSERT_EQ(e->type, EntityType::LIGHT);
    Light *l = (Light *)e;
    ASSERT_LT(length(l->position - Vec3(1, 2, 3)), 0.01);
    ASSERT_EQ(l->draw_as_point, true);
    return true;
});

TEST_CASE("parser-multiple", {
    auto es = parse_entities("# Light\ndraw_as_point 1\n# Light\ndraw_as_point 1\n");
    ASSERT_EQ(es.size(), 2);
    ASSERT(es[0] != es[1], "Should not return same entity twice");
    for (auto e : es) {
        ASSERT_EQ(e->type, EntityType::LIGHT);
        Light *l = (Light *)e;
        ASSERT_EQ(l->draw_as_point, true);
    }
    return true;
});
