#pragma once

#include "../utils/setup_class.h"
#include "color32.h"
#include "unit.h"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

namespace Gmeng {
    struct BaseImageMethods {
        virtual void resize( const std::size_t& width, const std::size_t& height ) = 0;
        virtual void resize( float scale ) = 0;
        virtual void recolor( uint32_t color ) = 0;
        virtual void recolor( uint32_t color, uint32_t to ) = 0;
        virtual uint8_t* rgb_array() = 0;

        bool operator==(const BaseImageMethods&) const = default;

        virtual ~BaseImageMethods() = default;
    };

    /// Blob : Render blob that contains a vector of units
    struct Blob : public BaseImageMethods {
      public:
        /// Width of the render blob.
        std::size_t width = 0;
        /// Height of the render blob.
        std::size_t height = 0;
        /// Unit vector of the render blob. The vector
        /// is treated as a 2D entity with a size of
        /// `VECTOR( WIDTH * HEIGHT )`, every time
        /// `vector.at(n) % WIDTH` is `0 (divisable)`,
        /// the next `Y++` value within the `HEIGHT`
        /// of the blob is present.
        std::vector<Gmeng::Unit> units;

        Blob(std::size_t w, std::size_t h, std::vector<Gmeng::Unit> u = {})
            : width(w), height(h), units(std::move(u)) {};
        Blob() = default;

        /// Recolors the Blob to the specified color.
        inline void recolor( uint32_t color ) override {
            for ( auto& un : this->units )
                un.color = color;
        };
        /// Recolors units within the Blob that are colored
        /// `color` with the color specified with `to`.
        inline void recolor( uint32_t color, uint32_t to ) override {
            for ( auto& un : this->units ) {
                if ( un.color == color ) un.color = to;
            };
        };

        /// Resizes the Blob and the vector of Units.
        virtual void resize( const std::size_t& new_width, const std::size_t& new_height ) override {
            this->_resize( new_width, new_height );
        };

        /// Resizes the Blob and the vector of Units.
        virtual void resize( float scale ) override {
            /// actually this is not the fastest way to do this
            /// since if we have the scale the _resize() method
            /// becomes a bit redundant but whatever
            this->resize( width*scale, height*scale );
        };

        virtual uint8_t* rgb_array() override {
            uint8_t* arr = new uint8_t[ this->width * this->height * 4 ];

            for ( int y = 0; y < this->height; y++ )
                for ( int x = 0; x < this->width; x++ ) {
                    const auto& unit = this->units[y * this->width + x];
                    const uint32_t& clr = GMENG_COL32( unit.color );
                    arr[ ( y * this->width ) + x + 0 ] = ( clr >> 16 ) & 0xFF;
                    arr[ ( y * this->width ) + x + 1 ] = ( clr >>  8 ) & 0xFF;
                    arr[ ( y * this->width ) + x + 0 ] = ( clr       ) & 0xFF;
                    arr[ ( y * this->width ) + x + 0 ] = 255;
                };

            return arr;
        };

        bool operator==(const Blob&) const = default;

      protected:
        void _resize( const std::size_t& new_width, const std::size_t& new_height ) {
            if ( new_width == width && new_height == height ) return;
            std::vector<Gmeng::Unit> new_img;
            new_img.resize( new_width*new_height );

            /// We calculate the scale of the resize operation.
            /// This returns something like 0.5 * current_size,
            /// which we can just multiply coordinates with to get
            /// the unit that coordinate would point to in the source image.
            float scale_x = static_cast<float>(width) / new_width;
            float scale_y = static_cast<float>(height) / new_height;

            for ( std::size_t y = 0; y < new_height; ++y ) {
                for ( std::size_t x = 0; x < new_width; ++x ) {
                    std::size_t src_x = (int)( x * scale_x );
                    std::size_t src_y = (int)( y * scale_y );

                    /// Clamp the values
                    if ( src_x >=  width ) src_x = width-1;
                    if ( src_y >= height ) src_y = height-1;

                    new_img[y * new_width + x] = this->units[ src_y * width + src_x ];
                };
            };
            this->units = std::move( new_img );
            /// set the new width & height values
            width = new_width; height = new_height;
        };
    };

    /// Texture : Gmeng Implementation for textures.
    /// Works as an image, as the terminal does not support
    /// importing image files to draw, this is a custom implementation.
    struct texture : public Blob {
      public:
        /// Collision property. If set to true,
        /// the texture (weirdly having this parameter)
        /// will not allow for phasing (multiple-item collision).
        bool collidable = false;
        /// Name of the texture. Used by the VisualCache
        /// Graphics Manager to sort and quickly access
        /// items within the texture dictionary.
        std::string name;

        using Blob::resize;

        texture() = default;
        texture( const Blob& b, bool _collidable, std::string name ) :
            collidable( _collidable ), name( name ) {
                this->width  = b.width;
                this->height = b.height;
                this->units  = (b.units);

            };

        void resize(const std::size_t &new_width, const std::size_t &new_height) override {
            this->_resize( new_width, new_height );
            width = new_width; height = new_height;
        };

        void resize( float scale ) override {
            this->resize( width*scale, height*scale );
        };

        GMENG_INIT_TYPE( width, height, units,
                         collidable, name );
    };
};

#include "../utils/functree.h"

namespace Gmeng::Renderer {
        /// Generates an empty texture with a pink-black layer
        /// (the default null layer), can be placed into a Model
        /// to become a textureless object.
        inline Gmeng::texture generate_empty_texture(int width, int height) {
            __functree_call__(Gmeng::Renderer::generate_empty_texture);
            Gmeng::texture __t; __t.width = width; __t.height = height; __t.collidable = false;
            int color_value = 0;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++)
                    __t.units.push_back(Gmeng::Unit {
                        .color = j % 2 == color_value ? Gmeng::color_t::PINK : Gmeng::color_t::BLACK
                    });
                if (color_value == 0) color_value++;
                else color_value = 0;
            };
            return __t;
        };
};
