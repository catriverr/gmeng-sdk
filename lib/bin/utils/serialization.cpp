#pragma once
#include "../gmeng.h"
#include <ios>
#include <ostream>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

///// @since 10.2.0

    ///
  ///
//     THIS UTILITY IS FOR SAVING AND RELOADING LEVEL DATA.
//         none of these methods are used by themselves.
//    instead, use `write_level_data()` and `read_level_data()`.
 ///
 ///

/// serialization for drawpoints
void serialize_drawpoint(const Gmeng::Renderer::drawpoint& dp, std::ostream& out) {
    out.write(reinterpret_cast<const char*>(&dp.x), sizeof(dp.x));
    out.write(reinterpret_cast<const char*>(&dp.y), sizeof(dp.y));
};

void deserialize_drawpoint(Gmeng::Renderer::drawpoint& dp, std::istream& in) {
    in.read(reinterpret_cast<char*>(&dp.x), sizeof(dp.x));
    in.read(reinterpret_cast<char*>(&dp.y), sizeof(dp.y));
};


void serialize_viewpoint(const Gmeng::Renderer::viewpoint& vp, std::ostream& out) {
    serialize_drawpoint(vp.start, out);
    serialize_drawpoint(vp.end, out);
};

void deserialize_viewpoint(Gmeng::Renderer::viewpoint& vp, std::istream& in) {
    deserialize_drawpoint(vp.start, in);
    deserialize_drawpoint(vp.end, in);
};


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


/// Writes a Model object to a stream.
void serialize_model(const Gmeng::Renderer::Model& model, std::ostream& out) {
    out.write(reinterpret_cast<const char*>(&model.width), sizeof(model.width));
    out.write(reinterpret_cast<const char*>(&model.height), sizeof(model.height));
    out.write(reinterpret_cast<const char*>(&model.size), sizeof(model.size));
    out.write(reinterpret_cast<const char*>(&model.position), sizeof(model.position));

    size_t name_length = model.name.size();
    out.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
    out.write(model.name.c_str(), name_length);

    serialize_texture(model.texture, out);

    out.write(reinterpret_cast<const char*>(&model.id), sizeof(model.id));
};

/// Reads a Model object from a stream.
void deserialize_model(Gmeng::Renderer::Model& model, std::istream& in) {
    in.read(reinterpret_cast<char*>(&model.width), sizeof(model.width));
    in.read(reinterpret_cast<char*>(&model.height), sizeof(model.height));
    in.read(reinterpret_cast<char*>(&model.size), sizeof(model.size));
    in.read(reinterpret_cast<char*>(&model.position), sizeof(model.position));

    size_t name_length;
    in.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
    model.name.resize(name_length);
    in.read(&model.name[0], name_length);

    deserialize_texture(model.texture, in);

    in.read(reinterpret_cast<char*>(&model.id), sizeof(model.id));
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

    list.values.resize(list_size);
    for (Gmeng::modifier& mod : list.values) {
        deserialize_modifier(mod, in);
    }
};


/// Writes a Chunk object to a stream.
void serialize_chunk(const Gmeng::chunk& ch, std::ostream& out) {
    serialize_viewpoint(ch.vp, out);

    size_t model_count = ch.models.size();
    out.write(reinterpret_cast<const char*>(&model_count), sizeof(model_count));

    for (const auto& model : ch.models) {
        serialize_model(model, out);
    }
};

/// Reads a Chunk object from a stream.
void deserialize_chunk(Gmeng::chunk& ch, std::istream& in) {
    deserialize_viewpoint(ch.vp, in);

    size_t model_count;
    in.read(reinterpret_cast<char*>(&model_count), sizeof(model_count));

    ch.models.resize(model_count);
    for (auto& model : ch.models) {
        deserialize_model(model, in);
    }
};


void serialize_level_base(const Gmeng::Renderer::LevelBase& level, std::ostream& out) {
    serialize_texture(level.lvl_template, out);
    out.write(reinterpret_cast<const char*>(&level.width), sizeof(level.width));
    out.write(reinterpret_cast<const char*>(&level.height), sizeof(level.height));
};

void deserialize_level_base(Gmeng::Renderer::LevelBase& level, std::istream& in) {
    deserialize_texture(level.lvl_template, in);
    in.read(reinterpret_cast<char*>(&level.width), sizeof(level.width));
    in.read(reinterpret_cast<char*>(&level.height), sizeof(level.height));
};


void serialize_level(const Gmeng::Level& level, std::ostream& out) {
    serialize_level_base(level.base, out);

    size_t chunk_count = level.chunks.size();
    out.write(reinterpret_cast<const char*>(&chunk_count), sizeof(chunk_count));

    for (const auto& chunk : level.chunks) {
        serialize_chunk(chunk, out);
    };

    size_t desc_length = level.desc.size();
    out.write(reinterpret_cast<const char*>(&desc_length), sizeof(desc_length));
    out.write(level.desc.c_str(), desc_length);

    size_t name_length = level.name.size();
    out.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
    out.write(level.name.c_str(), name_length);
};

void deserialize_level(Gmeng::Level& level, std::istream& in) {
    deserialize_level_base(level.base, in);

    size_t chunk_count;
    in.read(reinterpret_cast<char*>(&chunk_count), sizeof(chunk_count));

    level.chunks.resize(chunk_count);
    for (auto& chunk : level.chunks) {
        deserialize_chunk(chunk, in);
    };

    size_t desc_length;
    in.read(reinterpret_cast<char*>(&desc_length), sizeof(desc_length));
    level.desc.resize(desc_length);
    in.read(&level.desc[0], desc_length);

    size_t name_length;
    in.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
    level.name.resize(name_length);
    in.read(&level.name[0], name_length);
};



/// Writes out data of a Level object to a file, can be reloaded with read_level_data().
void write_level_data(std::string filename, Gmeng::Level& level) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        gm_log("ERROR: could not open file " + filename + " to write-out binary level data.");
        out.close();
        return;
    };

    serialize_level(level, out);
    serialize_modifier_list(level.display.camera.modifiers, out);

    out.close();
};

/// Reads data of a Level object from a file.]
void read_level_data(std::string filename, Gmeng::Level& level) {
    std::ifstream inf(filename, std::ios::binary);
    if (!inf.is_open()) {
        gm_log("ERROR: could not open file " + filename + " to read binary level data.");
        inf.close();
        return;
    };

    deserialize_level(level, inf);
    deserialize_modifier_list(level.display.camera.modifiers, inf);

    inf.close();
};
#define GMENG_SERIALIZATION_INIT 1