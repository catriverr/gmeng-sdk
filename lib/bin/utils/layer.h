#pragma once

#include "../types/entity.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

static void gmeng_sort_entities_by_z_index(std::vector< std::shared_ptr<Gmeng::EntityBase>>& entities) {
    std::sort(entities.begin(), entities.end(),
        [](const std::shared_ptr<Gmeng::EntityBase>& a,
            const std::shared_ptr<Gmeng::EntityBase>& b) {

            int a_zindex = a->modifiers.get_value("z_index");
            int b_zindex = b->modifiers.get_value("z_index");

            if ( a_zindex == b_zindex ) return false;
            else return a_zindex < b_zindex;
        });
};

/// Moves an item in a vector N times. Time complexity is O(n) where n = vec.size()
/// Should only be used for z-index sorts. Gmeng 14.0.0.
template<typename T>
static void gmeng_move_item_in_vector(std::vector<T>& vec, size_t item_index, int n = 1) {
    std::vector<T> n_vec;

    assert(item_index < vec.size() && "item index is larger than the vector size");
    assert(item_index - n > 0 && "moving item to index smaller than 0 is not permitted");
    assert(item_index - n < vec.size() && "moving item to index larger than vector size is not permitted");

    for (size_t i = 0; i < vec.size(); i++) {
        if ( i == item_index ) continue;
        if ( i == item_index - n ) n_vec.push_back( vec[item_index] );
        n_vec.push_back( vec[item_index] );
    };
    vec = n_vec;
};
