#pragma once
#include <algorithm>
#include <string>
#include <vector>
namespace Gmeng {
    /// Modifier : a named in-game variable that can
    /// be modified by the developer console. Used
    /// for cases where in-game changes may be requested
    /// by the player during runtime. Serialized into
    /// binary with `serialize_modifier`.
    struct modifier {
	  public:
        /// Name of the modifier. Accessable in
        /// `Gmeng::ModifierList::modifiers.at( name )`.
		std::string name = "v_unallocated_modifier";
        /// Value of the modifier. Only integer values
        /// are accepted, for multiple reasons, main
        /// one being the fact that most of the time,
        /// modifiers are either enums (glorified integers)
        /// or integer values by nature anyways.
        int value = -1;
	};

    /// ModifierList : Container for modifiers
    /// concerning any structure. Used in multiple
    /// internal classes like `Gmeng::Level` and `Gmeng::Camera`.
	struct ModifierList {
	  public:
        /// List of modifiers within the list.
		std::vector<modifier> values = {};

        /// Sorts the vector for binary-search operations,
        /// this makes access to values much faster.
        void sort() {
            std::sort(values.begin(), values.end(), [](const auto& a, const auto& b) {
                return a.name < b.name;
            });
        };

        modifier& operator[](std::string_view key) {
            auto it = std::lower_bound(values.begin(), values.end(), key,
                [](const auto& val, std::string_view k) {
                    return val.name < k;
                });

            if ( it != values.end() && it->name == key ) {
                return *it;
            } else {
                this->add_value((std::string)key, 0);
                return (*this)[key];
            }
        };

        /// Erases a modifier with the given key. If a modifier
        /// with the given key is not found, no item is erased.
        /// If a modifier is found, the array is sorted after
        /// deleting the modifier.
        ///
        /// This method will only delete the first instance of
        /// a modifier with a given key. It needs to be reexecuted
        /// n times if there are n amount of instances of the modifier.
        void erase(std::string_view key) {
            auto it = std::lower_bound(values.begin(), values.end(), key,
                [](const auto& val, std::string_view k) {
                    return val.name < k;
                });

            if ( it != values.end() && it->name == key )
                this->values.erase(it), this->sort();
        };

        bool has_value(std::string_view key) const {
            return ( this->get_value(key) != -1 );
        };

        /// Returns the value of a modifier with the key
        /// provided. The function will return an unknown
        /// instance of the value, so if there are multiple
        /// stacked within the list any one may be returned.
        /// If no instances of a modifier with the given key
        /// is found, `-1` is returned.
        int get_value(std::string_view key) const {

            auto it = std::lower_bound(values.begin(), values.end(), key,
                [](const auto& val, std::string_view k) {
                    return val.name < k;
                });

            if (it != values.end() && it->name == key) {
                return it->value;
            }

            return -1;
        };

        /// Sets the value of a modifier with the key
        /// provided. The function will only set the first
        /// instance of the value, so if there are multiple
        /// stacked within the list only the first one will
        /// be modified. If no instances of a modifier with
        /// the given key is found, nothing is modified.
        ///
        /// This function does NOT create a new entry if
        /// a modifier with the key provided exists.
        void set_value(std::string_view key, int value) {
            /// Binary search to find where the key should be
            auto it = std::lower_bound(values.begin(), values.end(), key,
                [](const auto& val, std::string_view k) {
                    return val.name < k;
                });

            /// If the key is found, set it.
            if (it != values.end() && it->name == key) {
                it->value = value;
            };
        };

        /// Adds a modifier to the list. If there is already
        /// a modifier with the same key, this function does
        /// NOT modify the value. In every case, this function
        /// will add the new modifier to the end of the list.
        /// Even if there is already a modifier with the same key.
        void _add_value(modifier mod) {
            this->values.push_back( mod );
            this->sort();
        };
        /// Adds a modifier to the list. If there is already
        /// a modifier with the same key, this function does
        /// NOT modify the value. In every case, this function
        /// will add the new modifier to the end of the list.
        /// Even if there is already a modifier with the same key.
        void add_value(std::string key, int value) {
            modifier mod { key, value };
            this->_add_value( mod );
            this->sort();
        };

	};
};
