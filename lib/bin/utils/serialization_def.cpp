#pragma once
#include <cstdlib>
#include <cstring>
#include <istream>
#include <ostream>

#include "../types/unit.h"
#include "../types/texture.h"
#include "../types/modifier.h"

/// Serializes a Unit object, and writes it to a stream.
void serialize_unit(const Gmeng::Unit& unit, std::ostream& stream) {
    stream.write(reinterpret_cast<const char*>(&unit.color), sizeof(unit.color));
    stream.write(reinterpret_cast<const char*>(&unit.collidable), sizeof(unit.collidable));
    stream.write(reinterpret_cast<const char*>(&unit.transparent), sizeof(unit.transparent));
    stream.write(reinterpret_cast<const char*>(&unit.special), sizeof(unit.special));
    stream.write(reinterpret_cast<const char*>(&unit.special_clr), sizeof(unit.special_clr));

    stream.write(&unit.special_c_unit, sizeof(unit.special_c_unit));
}

/// Reads a Unit object from a stream.
void deserialize_unit(Gmeng::Unit& unit, std::istream& stream) {
    stream.read(reinterpret_cast<char*>(&unit.color), sizeof(unit.color));
    stream.read(reinterpret_cast<char*>(&unit.collidable), sizeof(unit.collidable));
    stream.read(reinterpret_cast<char*>(&unit.transparent), sizeof(unit.transparent));
    stream.read(reinterpret_cast<char*>(&unit.special), sizeof(unit.special));
    stream.read(reinterpret_cast<char*>(&unit.special_clr), sizeof(unit.special_clr));


    ///size_t special_c_unit_size;
    ///stream.read(reinterpret_cast<char*>(&special_c_unit_size), sizeof(special_c_unit_size));
    ///std::string abc;
    ///abc.resize(special_c_unit_size);
    ///stream.read(&abc[0], special_c_unit_size);
    stream.read(&unit.special_c_unit, sizeof(unit.special_c_unit));
}


/// Writes a Texture object to a stream.
void serialize_texture(const Gmeng::texture& tex, std::ostream& stream) {
    stream.write(reinterpret_cast<const char*>(&tex.width), sizeof(tex.width));
    stream.write(reinterpret_cast<const char*>(&tex.height), sizeof(tex.height));
    stream.write(reinterpret_cast<const char*>(&tex.collidable), sizeof(tex.collidable));

    size_t units_size = tex.units.size();
    stream.write(reinterpret_cast<const char*>(&units_size), sizeof(units_size));
    for (const auto& unit : tex.units) {
        serialize_unit(unit, stream);
    }

    size_t name_size = tex.name.size();
    stream.write(reinterpret_cast<const char*>(&name_size), sizeof(name_size));
    stream.write(tex.name.c_str(), name_size);
}

/// Reads a Texture object from a stream.
void deserialize_texture(Gmeng::texture& tex, std::istream& stream) {
    stream.read(reinterpret_cast<char*>(&tex.width), sizeof(tex.width));
    stream.read(reinterpret_cast<char*>(&tex.height), sizeof(tex.height));
    stream.read(reinterpret_cast<char*>(&tex.collidable), sizeof(tex.collidable));

    size_t units_size;
    stream.read(reinterpret_cast<char*>(&units_size), sizeof(units_size));
    tex.units.resize(units_size);
    for (auto& unit : tex.units) {
        deserialize_unit(unit, stream);
    }

    size_t name_size;
    stream.read(reinterpret_cast<char*>(&name_size), sizeof(name_size));
    tex.name.resize(name_size);
    stream.read(&tex.name[0], name_size);
};


/// Writes a Modifier object to a stream.
void serialize_modifier(const Gmeng::modifier& mod, std::ostream& out) {
    size_t name_length = mod.name.size();
    out.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
    out.write(mod.name.c_str(), name_length);
    out.write(reinterpret_cast<const char*>(&mod.value), sizeof(mod.value));
};

/// Reads a Modifier object from a stream.
void deserialize_modifier(Gmeng::modifier& mod, std::istream& in) {
    size_t name_length;
    in.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
    mod.name.resize(name_length);
    in.read(&mod.name[0], name_length);
    in.read(reinterpret_cast<char*>(&mod.value), sizeof(mod.value));
};


/// Writes a modifier list object to a stream.
void serialize_modifier_list(const Gmeng::ModifierList& list, std::ostream& out) {
    size_t list_size = list.values.size();
    out.write(reinterpret_cast<const char*>(&list_size), sizeof(list_size));

    for (const Gmeng::modifier& mod : list.values) {
        serialize_modifier(mod, out);
    }
};

/// Reads a modifier list object from a stream.
void deserialize_modifier_list(Gmeng::ModifierList& list, std::istream& in) {
    size_t list_size;
    in.read(reinterpret_cast<char*>(&list_size), sizeof(list_size));

    /// Since 13.4.0: instead of resizing and deleting all previously
    /// set modifiers, we use the default set_value methods in ModifierList instead.

    //list.values.resize(list_size);
    for (int i = 0; i < list_size; i++) {
        Gmeng::modifier mod;
        deserialize_modifier(mod, in);
        /// if the value isn't -1, there is a value at that modifier key.
        if ( list.get_value( mod.name ) != -1 ) list.set_value( mod.name, mod.value );
        else list._add_value( mod );
    }
};

