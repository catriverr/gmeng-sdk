#pragma once

#include <cassert>
#include <cstddef>
#include <vector>
#include <array>

#include "../src/objects.cpp"

namespace {
    using namespace Gmeng;
}

template<typename T>
struct subarray_2d {
  private:
    T* main_array = nullptr;
  public:
    Renderer::viewpoint main_size;
    Renderer::viewpoint view;

    size_t view_size;
    size_t stride;
    size_t view_stride;

    subarray_2d(T* arr, const Renderer::viewpoint& _main_size, const Renderer::viewpoint& _box)
        : main_array(arr), main_size(_main_size), view(_box) {
            this->stride = ( main_size.end.x - main_size.start.x );
            this->view_stride = ( view.end.x - view.start.x );
            this->view_size = ( this->view_stride * ( view.end.y - view.start.y ) );
        };
    subarray_2d() = default;

    inline constexpr bool exists() { return main_array != nullptr; };

    inline constexpr T& operator[](size_t index) {
        return this->main_array[(*this)(index)];
    };

    inline constexpr const Gmeng::Renderer::drawpoint point(int index) const {
        return {
            view.start.x + index % (int)view_stride,
            view.start.y + index / (int)view_stride
        };
    };

    inline constexpr size_t operator()(size_t index) {
        int _x = index % view_stride;
        int _y = index / view_stride;
        int main_array_index = ( (view.start.y + _y) * stride ) + view.start.x + _x;

        if ( main_array_index < 0 ) return ( view.end.y * stride ) - main_array_index;

        return main_array_index; 
    };

    inline constexpr T& at( size_t index ) { return (*this)[index]; };
};
