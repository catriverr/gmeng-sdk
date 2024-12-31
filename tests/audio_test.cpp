#include <iostream>
#include "../lib/bin/gmeng.h"
#include "../lib/bin/src/audio.cpp"

using namespace Gmeng;
int main(int argc, char** argv) {
    Audio::play_frequency(FROM_OCTAVE(MUSICAL_NOTE_E, 4), 500);
    Audio::play_frequency(FROM_OCTAVE(MUSICAL_NOTE_A, 5), 500);
    Audio::play_frequency(FROM_OCTAVE(MUSICAL_NOTE_D, 5), 500);
    Audio::play_frequency(FROM_OCTAVE(MUSICAL_NOTE_G, 5), 500);

    std::vector<double> CHORD1 = {
            FROM_OCTAVE(MUSICAL_NOTE_F, 4),
            FROM_OCTAVE(MUSICAL_NOTE_Bb, 5),
            FROM_OCTAVE(MUSICAL_NOTE_Cs, 5),
            FROM_OCTAVE(MUSICAL_NOTE_F, 5),
    };

    std::vector<double> CHORD2 = {
            FROM_OCTAVE(MUSICAL_NOTE_E, 4),
            FROM_OCTAVE(MUSICAL_NOTE_Gs, 5),
            FROM_OCTAVE(MUSICAL_NOTE_Cs, 5),
    };

    Audio::play_chord(CHORD1, 1000);
    Audio::play_chord(CHORD2, 1000);

    Audio::delete_audio_output();
    return 0;
};
