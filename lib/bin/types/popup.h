#pragma once
#include "../utils/global.h"
#include "../utils/log.h"
#include "../src/objects.cpp"

#include "modifier.h"
#include "texture.h"
#include "event.h"
#include <map>
#include <string>
#include <tuple>


inline void str_append(char* s, char c) {
    int len = strlen(s);
    s[len] = c;
    s[len+1] = '\0';
};

namespace Gmeng {
    struct popup_flags {
        bool focused = false;
        bool fullscreen = false;

        bool receive_input_event = false;
        bool receive_mouse_event = false;

        bool receive_events_when_unfocused = false;
    };

    struct Popup {
      public:
        int id = g_mkid();
        char* label = "unknown";
        Objects::coord position = { 0, 0 };
        popup_flags flags;
        ModifierList modifiers;

        Blob blob;

        virtual void focus(bool state) {
            this->flags.focused = state;
        };

        virtual void event_handler( Event ) = 0;
        virtual const Blob& draw( Renderer_Type ) { return this->blob; };
    };

    using std::map, std::string, std::tuple;
    struct Modal : public Popup {
        union modal_item {
            /// for content_type: INT_INPUT
            int* integer_input_buffer;
            /// for content_type: FLT_INPUT
            float* float_input_buffer;
            /// for content_type: STR_INPUT
            char** string_input_buffer;
            /// for content_type: TEXT
            char* text_content;
        };

        enum content_type {
            /// int*
            INT_INPUT,
            /// float*
            FLT_INPUT,
            /// char**
            STR_INPUT,
            /// char*
            TEXT,
            /// horizontal splitter
            SPLITTER,
        };

      private:
        map<unsigned int, tuple<content_type, std::string_view, modal_item*>> items;
        int buffer_count = 0;

      public:

        int max_input_length = 50;

        inline void format_blob() {
            if ( !this->modifiers.has_value("outline") )
                this->modifiers.add_value("outline", 1);
            int outline = this->modifiers.get_value("outline");

            int width_needed  = 0;
            int height_needed = 0;

            if ( outline > 0 ) height_needed += 4;
            for ( auto item : this->items ) {
                union modal_item* content = std::get<2>( item.second );

                switch ( (content_type)item.first ) {
                    case STR_INPUT: {
                        int len = strlen(*content->string_input_buffer);
                        if ( len > width_needed )
                            width_needed = len + 2 + ( outline > 0 ? 4 : 0 );
                    };
                    case INT_INPUT:
                    case FLT_INPUT:
                        std::string_view label = std::get<1>( item.second );

                        if ( width_needed < label.length() )
                            width_needed = label.length() + ( outline > 0 ? 4 : 0 );
                        height_needed++;
                        break;
                };
            };
        };

        inline int add_content(enum content_type item_type, std::string_view label, union modal_item* item) {
            this->items[buffer_count] = { item_type, label, item };

            this->format_blob();

            return buffer_count++;
        };

    };
};
