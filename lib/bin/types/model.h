#pragma once

#include "../utils/functree.h"

#include "../src/objects.cpp"
#include "../src/textures.cpp"

#include "texture.h"


namespace Gmeng::Renderer {
        /// Gmeng class for Models. Models are any standalone object
        /// within a render instance that isn't treated as an entity.
        /// Models are part of a 'chunk' which make up a Level instance.
        class Model {
            private:
              inline Objects::coord get_pointXY(int pos) {
                  __functree_call__(Gmeng::Renderer::Model::get_pointXY);
                  int __p1_x = (pos % this->width);
                  int __p1_y = (pos / this->width);
                  return Objects::coord {
                        .x=__p1_x,
                        .y=__p1_y
                  };
              };
            public:
              texture highlighted_texture = { Blob(0, 0, {}), false, "#$!_NONE" };
              /// Whether or not the model is highlighted. Gmeng
              /// internally draws a box around the model if this
              /// parameter is set to true. Useful for debugging.
              bool highlighted = false;
              /// Activity status of the model. If set to false,
              /// the model is treated as inactive and it will
              /// not be drawn in the renderscale. Any effects
              /// the model might have on collision and lighting
              /// will also not be taken into account during calculation.
              bool active = true;
              /// Width of the model. This should always be equal to
              /// `Model::texture.width`, as the texture is what's
              /// taken into account when drawing a Model.
              std::size_t width;
              /// Height of the model. This should always be equal to
              /// `Model::texture.height`, as the texture is what's
              /// taken into account when drawing a Model.
              std::size_t height;
              /// Size of the model. This should always be equal to
              /// `Model::width * Model::height`. Internally managed.
              std::size_t size;
              /// Position of the model within its parent `chunk`
              /// container. This is almost always equal to the
              /// model's position within the Level, but with
              /// levels that have multiple chunks, it's not guaranteed.
              drawpoint position;
              /// Name of the model. Useful for identifying specific
              /// models that may be using the same texture as another
              /// model within the same level.
              std::string name;
              /// Texture of the model. The texture controls the
              /// model's `width, height, size` values internally.
              /// This is the image that is drawn as a Model when
              /// `get_renderscale` accesses this model's parent chunk.
              Gmeng::texture texture;
              /// ID of the model.
              int id;

              GMENG_INIT_TYPE( highlighted, active, width, height,
                               size, position, name, texture, id );
              /// Resets the texture into the default null-layer with the same height
              /// as the previously attached texture.
              ///
              /// @since 4.1-glvl
              inline void reset_texture() {
                  __functree_call__(Gmeng::Renderer::Model::reset_texture);
                  this->texture = generate_empty_texture(this->width, this->height);
              };

              const Gmeng::texture& get_sprite(bool force_highlight = false) {
                  if ( !this->highlighted && !force_highlight ) return this->texture;
                  else {
                      if ( this->highlighted_texture.name == "#$!_NONE" )
                          this->highlighted_texture = gmeng_create_highlighted_texture(this->texture);
                      return this->highlighted_texture;
                  };
              };

              /// Resizes the Model (and its texture)
              inline void resize( const std::size_t& new_width, const std::size_t& new_height ) {
                  this->texture.resize( new_width, new_height );
                  this->width = new_width; this->height = new_height;
              };

              /// Resizes the Model (and its texture)
              inline void resize( const float& scale ) {
                  this->texture.resize(width*scale, height*scale);
                  this->width = width*scale; this->height = height*scale;
              };

              /// Attaches a given texture to the model.
              ///
              /// @since 4.1-glvl
              inline void attach_texture(const Gmeng::texture& __t) {
                  __functree_call__(Gmeng::Renderer::Model::attach_texture);
                  this->texture = __t; this->width = __t.width; this->height = __t.height;
              };

              /// Loads a texture from a file.
              ///
              /// @since 4.1-glvl
              inline void load_texture(std::string __tf) {
                  __functree_call__(Gmeng::Renderer::Model::load_texture);
                  this->texture = Gmeng::LoadTexture(__tf);
                  this->width = this->texture.width; this->height = this->texture.height;
              };

              /// sets the border of the model.
              /// Models do not have borders by default.
              /// this method can be used for debug_render rendering modes.
              ///
              /// @since 10.4.0-d
              inline void set_border(color_t border_color) {
                  // no functree call here
                  for (int x, y = 0; (y*width)+x < width*height; x++) {
                      if (x != 0 && x % width == 0) y++, x = 0;
                      if ( y == 0        || x == 0      ||
                              y == height-1 || x == width-1   ) this->texture.units.at(x) = {
                          .color = border_color, .transparent = false, .special = false
                      };
                  };
              };
        };

        /// Returns a model's viewpoint. Calculated by taking
        /// a model's position and width,height.
        inline Renderer::viewpoint model_view( Renderer::Model model ) {
            return {
                model.position,
                { model.position.x + (int)model.width,
                       model.position.y + (int)model.height }
            };
        };

        inline Model generate_empty_model(int width, int height) {
            __functree_call__(Gmeng::Renderer::generate_empty_model);
            int id = g_mkid();
            return Model {
                .width=static_cast<std::size_t>(width), .height=static_cast<std::size_t>(height), .size=static_cast<std::size_t>(width*height),
                .position = { 0, 0 }, .name = "unknown_" + v_str(id),
                .texture = generate_empty_texture(width, height), .id = id
            };
        };
};
