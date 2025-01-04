#pragma once
#include <cstddef>
#include <cstring>
#include <ncurses.h>
#include <string>
#include "../gmeng.h"
#include "../src/audio.cpp"
#include "../cli/index.h"
#include <ncursesw/ncurses.h>

using namespace Gmeng;

#include <map>
#include <vector>

// Define the struct for keys
struct INSTRUMENT_KEY {
    enum KEY { C, D, E, F, G, A, B, C_SHARP, D_SHARP, F_SHARP, G_SHARP, A_SHARP, UNKNOWN };
    KEY key;
    unsigned int octave;
    enum STATE { PRESSED, RELEASED } state;



};
    static INSTRUMENT_KEY UNKNOWN_KEY = {
        INSTRUMENT_KEY::UNKNOWN, 0
    };


static int numKeys = 31;  // Adjust for the number of keys (e.g., 2 octaves starting from A4)

// Store pressed keys
vector<INSTRUMENT_KEY> pressedKeys;

// Function to handle key press or release
void handleKeyEvent(const INSTRUMENT_KEY& keyEvent) {
    const char* keyNames[] = {"C", "D", "E", "F", "G", "A", "B", "C#", "D#", "F#", "G#", "A#"};
    mvprintw(LINES - 1, 0, "Key: %s%d State: %s          ",
             keyNames[keyEvent.key], keyEvent.octave,
             keyEvent.state == INSTRUMENT_KEY::PRESSED ? "PRESSED" : "RELEASED");
    refresh();

    const double keyFrequencies[] = {
        MUSICAL_NOTE_A,
        MUSICAL_NOTE_B,
        MUSICAL_NOTE_C,
        MUSICAL_NOTE_D,
        MUSICAL_NOTE_E,
        MUSICAL_NOTE_F,
        MUSICAL_NOTE_G,
        MUSICAL_NOTE_As,
        MUSICAL_NOTE_Cs,
        MUSICAL_NOTE_Ds,
        MUSICAL_NOTE_Fs,
        MUSICAL_NOTE_Gs,
        MUSICAL_NOTE_A
    };

    if (keyEvent.state == INSTRUMENT_KEY::RELEASED) Gmeng::Audio::play_frequency(0, 0);
    else if (keyEvent.state == INSTRUMENT_KEY::PRESSED) Gmeng::Audio::play_frequency(
            FROM_OCTAVE(keyFrequencies[keyEvent.key], keyEvent.octave), 100
    );
};



// Update the black key mapping logic
INSTRUMENT_KEY mapMouseToKey(int x, int y) {
    INSTRUMENT_KEY keyEvent;

    int whiteKeyWidth = 4, whiteKeyHeight = 5;
    int blackKeyWidth = 3, blackKeyHeight = 3;
    int startX = 1, startY = 1;

    if (y < startY || y > startY + whiteKeyHeight) return UNKNOWN_KEY;

    int keyIndex = (x - startX) / whiteKeyWidth;
    bool isBlackKey = false;

    // Check for black key
    if (y >= startY-1 && y < startY + blackKeyHeight) {
        int blackKeyOffset = (x - startX) % whiteKeyWidth;
        if (blackKeyOffset > 0 && blackKeyOffset < blackKeyWidth) {
            if (keyIndex % 7 != 1 && keyIndex % 7 != 4) {  // Skip E# and B#
                isBlackKey = true;
            }
        }
        mvaddstr(LINES-1, COLS-16, "BLACK KEY RN    ");
    } else mvaddstr(LINES-1, COLS-16, "NOT BLACK KEY RN");

    // White and Black key mappings
    static INSTRUMENT_KEY::KEY whiteKeys[] = {INSTRUMENT_KEY::A, INSTRUMENT_KEY::B, INSTRUMENT_KEY::C,
                                                    INSTRUMENT_KEY::D, INSTRUMENT_KEY::E,INSTRUMENT_KEY::F, INSTRUMENT_KEY::G };

    static const INSTRUMENT_KEY::KEY blackKeys[] = {INSTRUMENT_KEY::A_SHARP, INSTRUMENT_KEY::B,INSTRUMENT_KEY::C_SHARP,
                                                    INSTRUMENT_KEY::D_SHARP, INSTRUMENT_KEY::E,INSTRUMENT_KEY::F_SHARP, INSTRUMENT_KEY::G_SHARP};


    static const std::map<INSTRUMENT_KEY::KEY, INSTRUMENT_KEY::KEY> SHARP_MAP = {
        {
            INSTRUMENT_KEY::A, INSTRUMENT_KEY::A_SHARP
        },
        {
            INSTRUMENT_KEY::B, INSTRUMENT_KEY::B
        },
        {
            INSTRUMENT_KEY::C, INSTRUMENT_KEY::C_SHARP
        },
        {
            INSTRUMENT_KEY::D, INSTRUMENT_KEY::D_SHARP
        },
        {
            INSTRUMENT_KEY::E, INSTRUMENT_KEY::E
        },
        {
            INSTRUMENT_KEY::F, INSTRUMENT_KEY::F_SHARP
        },
        {
            INSTRUMENT_KEY::G, INSTRUMENT_KEY::G_SHARP
        }
    };


    // Calculate the key index for black and white keys
    if (isBlackKey) {
        int adjustedIndex = (keyIndex % 7) % 5;  // Map to black key range
        keyEvent.key = SHARP_MAP.at( whiteKeys[keyIndex %7] );
        keyEvent.octave = keyIndex/7 + 4;  // Adjust octave correctly
    } else {
        keyEvent.key = whiteKeys[keyIndex % 7];
        keyEvent.octave = keyIndex/7 + 4;  // Adjust octave correctly
    }

    mvaddstr(LINES-1, COLS-3,  (std::to_string(keyIndex) + "  ").c_str());

    keyEvent.state = INSTRUMENT_KEY::PRESSED;
    return keyEvent;
}

// Update the drawPiano function to reflect key placements correctly
void drawPiano() {
    int whiteKeyWidth = 4, whiteKeyHeight = 5;
    int blackKeyWidth = 3, blackKeyHeight = 3;
    int startX = 1, startY = 1;
    int currentOctave = 4;

    // Define the sequence of keys
    static const INSTRUMENT_KEY::KEY whiteKeys[] = {
        INSTRUMENT_KEY::A, INSTRUMENT_KEY::B, INSTRUMENT_KEY::C, INSTRUMENT_KEY::D,
        INSTRUMENT_KEY::E, INSTRUMENT_KEY::F, INSTRUMENT_KEY::G};
    static const INSTRUMENT_KEY::KEY blackKeys[] = {
        INSTRUMENT_KEY::A_SHARP, INSTRUMENT_KEY::C_SHARP, INSTRUMENT_KEY::D_SHARP,
        INSTRUMENT_KEY::F_SHARP, INSTRUMENT_KEY::G_SHARP};

    // Draw white keys
    for (int i = 0; i < numKeys; ++i) {
        int x = startX + i * whiteKeyWidth;
        INSTRUMENT_KEY::KEY keyType = whiteKeys[i % 7];
        unsigned int octave = currentOctave + (i / 7);

        // Check if the key is pressed
        bool isPressed = false;
        for (const auto& key : pressedKeys) {
            if (key.key == keyType && key.octave == octave && key.state == INSTRUMENT_KEY::PRESSED) {
                isPressed = true;
                break;
            }
        }

        int color = isPressed ? 3 : 1;  // Green if pressed, white otherwise
        for (int y = startY; y < startY + whiteKeyHeight; ++y) {
            mvaddch(y, x, ' ' | COLOR_PAIR(color));
            mvaddch(y, x + 1, ' ' | COLOR_PAIR(color));
            mvaddch(y, x + 2, ' ' | COLOR_PAIR(color));
        }

        for (int zz = startY; zz < startY + whiteKeyHeight; ++zz) {
            mvaddch(zz, x + 3, ACS_VLINE | COLOR_PAIR(1));
        }
    }

    // Draw black keys
    for (int i = 0; i < numKeys; ++i) {
        // Skip black keys where necessary
        if (whiteKeys[i % 7] == INSTRUMENT_KEY::B || whiteKeys[i % 7] == INSTRUMENT_KEY::E) {
            continue;
        }

        int x = startX + i * whiteKeyWidth + 2;
        INSTRUMENT_KEY::KEY keyType = blackKeys[i % 5];
        unsigned int octave = currentOctave + (i / 7);

        // Check if the key is pressed
        bool isPressed = false;
        for (const auto& key : pressedKeys) {
            if (key.key == keyType && key.octave == octave && key.state == INSTRUMENT_KEY::PRESSED) {
                isPressed = true;
                break;
            }
        }

        int color = isPressed ? 3 : 2;  // Green if pressed, black otherwise
        for (int y = startY; y < startY + blackKeyHeight; ++y) {
            mvaddch(y, x, ' ' | COLOR_PAIR(color));
            mvaddch(y, x + 1, ' ' | COLOR_PAIR(color));
        }
    }

    refresh();
}




void drawOther() {
    attron(A_BOLD);
    mvhline(0, 0, ACS_HLINE, COLS);
    mvhline(6, 0, ACS_HLINE, COLS);

    /// GMENG INFO SLATE
    mvaddch(10, 0, ACS_ULCORNER);
    mvhline(10, 1, ACS_HLINE, 39);
    mvaddch(10, 40, ACS_URCORNER);

    for (int YF = 0; YF < 10; YF++) {
        mvaddch(11+YF, 0, ACS_VLINE);
        mvaddch(11+YF, 40, ACS_VLINE);
    };
    mvaddch(21, 0,  ACS_LLCORNER);
    mvhline(21, 1,  ACS_HLINE, 39);
    mvaddch(21, 40, ACS_LRCORNER);

    // "GMENG BASIC BEAT-MAKER" // length 22 // 40-22=18/2=9

    mvaddstr(11, 9, "GMENG BASIC BEAT-MAKER");
};




int AEC__runner() {
    // Initialize ncurses
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
    start_color();
    use_default_colors();

    curs_set(0);

    // Initialize colors
    init_pair(1, COLOR_BLACK, 15);   // White key
    init_pair(2, COLOR_WHITE, COLOR_BLACK);  // Black key
    init_pair(3, COLOR_BLACK, COLOR_BLUE);  // Pressed key
    init_pair(4, 15, COLOR_RED);
    init_pair(5, COLOR_YELLOW, COLOR_RED);

    bkgd(COLOR_PAIR(4));
    crmode();

    // Event loop
    MEVENT event;
    while (true) {
        drawPiano();
        drawOther();

        int ch = getch();
        if (ch == 'q') break;
        if (ch == 'c' || ch == 'r') pressedKeys.clear();
        if (ch == 'a') {
            vector<double> frq = {
                FROM_OCTAVE(MUSICAL_NOTE_C, 4),
                FROM_OCTAVE(MUSICAL_NOTE_E, 4),
                FROM_OCTAVE(MUSICAL_NOTE_A, 5)
            };
            Gmeng::Audio::play_chord(frq, 1000);
        };

        if (ch == KEY_MOUSE && getmouse(&event) == OK) {
            if (event.bstate & BUTTON1_CLICKED) {
                INSTRUMENT_KEY keyEvent = mapMouseToKey(event.x, event.y);
                if (keyEvent.key == INSTRUMENT_KEY::UNKNOWN) continue;
                keyEvent.state = INSTRUMENT_KEY::PRESSED;

                int xd = 0;
                bool found = false;
                for (const auto& ke : pressedKeys) {
                    if (ke.key == keyEvent.key && ke.octave == keyEvent.octave) { found = true; pressedKeys.erase(pressedKeys.begin() + xd); break; };
                    xd++;
                };

                if (!found) pressedKeys.push_back(keyEvent);
                if (found) keyEvent.state = INSTRUMENT_KEY::RELEASED;
                handleKeyEvent(keyEvent);
            }
        }
    }
    Gmeng::Audio::delete_audio_output();
    // Cleanup
    endwin();
    return 0;
}







class audio_editor_command_t : public Gmeng_Commandline::Subcommand {
  public:
    audio_editor_command_t(string name, string desc): Subcommand(name, desc) {};
    void run(vector<std::string> args) {
        if (args.size() < 1) {
            // file load system
        };
        AEC__runner();
    };
};

static audio_editor_command_t AEC("songmaker", "Basic songmaker utility for Gmeng's Audio Engine");

static Gmeng_Commandline::InterfaceRegistrar register_AEC(
    std::make_unique<audio_editor_command_t>( AEC )
);
