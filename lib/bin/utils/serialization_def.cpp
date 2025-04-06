#pragma once
#include "../gmeng.h"


/// Serializes a Unit object, and writes it to a stream.
void serialize_unit(const Gmeng::Unit& unit, std::ostream& stream) {
    stream.write(reinterpret_cast<const char*>(&unit.color), sizeof(unit.color));
    stream.write(reinterpret_cast<const char*>(&unit.collidable), sizeof(unit.collidable));
    stream.write(reinterpret_cast<const char*>(&unit.transparent), sizeof(unit.transparent));
    stream.write(reinterpret_cast<const char*>(&unit.special), sizeof(unit.special));
    stream.write(reinterpret_cast<const char*>(&unit.special_clr), sizeof(unit.special_clr));

    size_t special_c_unit_size = unit.special_c_unit.size();
    stream.write(reinterpret_cast<const char*>(&special_c_unit_size), sizeof(special_c_unit_size));
    stream.write(unit.special_c_unit.c_str(), special_c_unit_size);
}

/// Reads a Unit object from a stream.
void deserialize_unit(Gmeng::Unit& unit, std::istream& stream) {
    stream.read(reinterpret_cast<char*>(&unit.color), sizeof(unit.color));
    stream.read(reinterpret_cast<char*>(&unit.collidable), sizeof(unit.collidable));
    stream.read(reinterpret_cast<char*>(&unit.transparent), sizeof(unit.transparent));
    stream.read(reinterpret_cast<char*>(&unit.special), sizeof(unit.special));
    stream.read(reinterpret_cast<char*>(&unit.special_clr), sizeof(unit.special_clr));


    size_t special_c_unit_size;
    stream.read(reinterpret_cast<char*>(&special_c_unit_size), sizeof(special_c_unit_size));
    unit.special_c_unit.resize(special_c_unit_size);
    stream.read(&unit.special_c_unit[0], special_c_unit_size);
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
