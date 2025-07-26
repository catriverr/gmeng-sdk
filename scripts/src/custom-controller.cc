#include "../../include/mp-pico-cpp/pico.cc"
#include "../../lib/bin/easy.h"
#include <chrono>
#include <cstdint>

/// GMENG INTERNAL SCRIPT FOR CUSTOM CONTROLLERS
/// gmeng 12.0.0
///
/// This utility provides the ability to communicate with
/// custom hardware with a Raspberry PICO's PWM,
/// Analog-to-Digital Converters and GPIO ports.
///
/// The functionality of this utility is dependant on
/// the mp-pico-cpp library, built by catriverr.
/// For more information on this library, visit
/// https://www.github.com/catriverr/mp-pico-cpp


int uint16_to_percentage(uint16_t val) {
    return (int) (val / 65535) * 100;
};

int custom_controller_handler( Gmeng::EventLoop* ev ) {
        /// initialize communications through usbmodem tty
        if ( pico_util::auto_init() == false ) {
            GAME_LOG("cannot establish connection to a PICO microcontroller.");
        } else {
            GAME_LOG("connected to Raspberry PICO microcontroller.");
            auto led_checker = time_rn;
            auto move_checker = time_rn;
            bool ledstate = false;

            Gmeng::EventInfo moveevent =  {
               .EVENT = Gmeng::KEYPRESS,
               .KEYPRESS_CODE = 'd',
               .prevent_default = false,
               .MOUSE_X_POS = -1,
               .MOUSE_Y_POS = -1,
               .alternative = false
            };

            while (ev->level->display.camera.modifiers.get_value("allow_controllers") != 0) {
                if (time_rn - led_checker > std::chrono::milliseconds(125)) { //// blinking led
                    led_checker = time_rn;
                    ledstate = !ledstate;
                    pico_util::GPIO_write(0, ledstate ? 1 : 0);
                };

                pico_util::PWM_set(15, 0, 1000); /// reset the gpio port to 0.
                pico_util::PWM_set(14, 0, 1000); /// reset the gpio port to 0.

                uint16_t fader_pot_value = pico_util::ADC(0);
                bool toggle_state = pico_util::GPIO_read(14) == pico_util::GPIOState::HIGH;
                bool nuke_state = pico_util::GPIO_read(15) == pico_util::GPIOState::HIGH;
                auto dstate =  nuke_state ? 2 : 0;

                if ( ev->level->display.camera.modifiers.get_value("debug_render") != dstate) {
                    ev->level->display.camera.set_modifier("debug_render", dstate);
                    ev->call_event(Gmeng::Event::FIXED_UPDATE, Gmeng::NO_EVENT_INFO);
                };


                int millisecond_delay = 100 - uint16_to_percentage(fader_pot_value);
                if (toggle_state) { /// switch is on
                    /// move player a little
                    if (time_rn - move_checker > std::chrono::milliseconds(millisecond_delay)) {
                        move_checker = time_rn;

                        ev->call_event( Gmeng::Event::KEYPRESS, moveevent );
                        ev->call_event(Gmeng::Event::FIXED_UPDATE, Gmeng::NO_EVENT_INFO);
                    };

                };

            };
        };
        return 0;
};


extern "C" int gmeng_script_command( Gmeng::EventLoop* ev ) {
    return custom_controller_handler( ev );
};
