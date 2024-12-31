#pragma once
#include <chrono>
#include <cmath>
#include <cstdint>
#include <vector>
#include <thread>
#include <functional>
#include <iostream>

constexpr double PI = 3.14159265358979323846;

#if __APPLE__
#include "AudioToolbox/AudioToolbox.h"
#include <CoreAudio/CoreAudio.h>


namespace Gmeng::Audio {
    static const double kSampleRate = 44100.0;
    static double phase = 0.0;  // Phase for sine wave
    static double volume = 0.5f;
    static AudioUnit audio_output = nullptr;

// Audio rendering callback
OSStatus RenderAudio(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp,
                     UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) {

    // Check for valid audio data buffer
    if (ioData == nullptr) {
        return -1;  // Return error if audio buffer is invalid
    }

    const double frequency = *reinterpret_cast<double*>(inRefCon);  // Frequency passed in from the caller
    const double phaseIncrement = 2.0 * M_PI * frequency / kSampleRate;

    // Loop through each frame and fill the buffer with audio data
    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        // Generate the sine wave sample
        float sample = static_cast<float>(sin(phase)) * volume;
        phase += phaseIncrement;

        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI; // Reset phase after a full cycle
        }

        // Write to both left and right channels (stereo output)
        ((float*)ioData->mBuffers[0].mData)[i] = sample;       // Left channel
        ((float*)ioData->mBuffers[1].mData)[i] = sample;       // Right channel
    }

    return noErr;
};

void init_audio_output() {
    AudioUnit audioUnit = nullptr;
    OSStatus result;

    // Set up audio component description
    AudioComponentDescription description;
    description.componentType = kAudioUnitType_Output;
    description.componentSubType = kAudioUnitSubType_DefaultOutput;
    description.componentManufacturer = kAudioUnitManufacturer_Apple;
    description.componentFlags = 0;
    description.componentFlagsMask = 0;

    // Find the audio component
    AudioComponent component = AudioComponentFindNext(nullptr, &description);
    if (component == nullptr) {
        std::cerr << "Error: Unable to find the default audio output component." << std::endl;
        return;
    }

    // Get the AudioUnit from the component
    result = AudioComponentInstanceNew(component, &audioUnit);
    if (result != noErr) {
        std::cerr << "Error: Unable to create AudioUnit instance, result = " << result << std::endl;
        return;
    }

    // Set up audio format
    AudioStreamBasicDescription streamFormat;
    streamFormat.mSampleRate = kSampleRate;
    streamFormat.mFormatID = kAudioFormatLinearPCM;
    streamFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    streamFormat.mChannelsPerFrame = 2;  // Stereo output
    streamFormat.mBitsPerChannel = 32;
    streamFormat.mBytesPerFrame = 4 * 2;
    streamFormat.mBytesPerPacket = 4 * 2;
    streamFormat.mFramesPerPacket = 1;

    result = AudioUnitSetProperty(audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0,
                                  &streamFormat, sizeof(streamFormat));
    if (result != noErr) {
        std::cerr << "Error: Unable to set stream format, result = " << result << std::endl;
        return;
    };

    result = AudioUnitInitialize(audioUnit);
    if (result != noErr) {
        std::cerr << "Error: Unable to initialize AudioUnit, result = " << result << std::endl;
        return;
    }


    audio_output = audioUnit;
};

// Function to play a frequency
void play_frequency(double frequency, unsigned int duration) {
    OSStatus result;
    if (audio_output == nullptr) init_audio_output();
    // Initialize the AudioUnit

    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = RenderAudio;
    callbackStruct.inputProcRefCon = reinterpret_cast<void*>(&frequency);  // Pass the frequency to the callback

    result = AudioUnitSetProperty(audio_output, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0,
                                  &callbackStruct, sizeof(callbackStruct));
    if (result != noErr) {
        std::cerr << "Error: Unable to set render callback, result = " << result << std::endl;
        return;
    }




    // Start audio playback
    result = AudioOutputUnitStart(audio_output);
    if (result != noErr) {
        std::cerr << "Error: Unable to start audio output, result = " << result << std::endl;
        return;
    }

    // Run for some time (duration of the note)
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    if (frequency != 0) play_frequency(0, 0);
};

void delete_audio_output() {
    AudioOutputUnitStop(audio_output);
    AudioUnitUninitialize(audio_output);
    AudioComponentInstanceDispose(audio_output);
};







constexpr uint32_t SAMPLE_RATE = 44100;
constexpr double AMPLITUDE = 0.25f;

OSStatus RenderAudioMult(void *inRefCon,
                          AudioUnitRenderActionFlags *ioActionFlags,
                          const AudioTimeStamp *inTimeStamp,
                          UInt32 inBusNumber,
                          UInt32 inNumberFrames,
                          AudioBufferList *ioData) {
    // Get the list of frequencies passed via inRefCon
    std::vector<double>* frequencies = reinterpret_cast<std::vector<double>*>(inRefCon);

    // Prepare the audio data buffers (stereo: two channels)
    AudioBuffer& leftChannel = ioData->mBuffers[0];  // Left channel
    AudioBuffer& rightChannel = ioData->mBuffers[1]; // Right channel

    // Set the buffer size to match the number of frames to be rendered
    leftChannel.mData = malloc(inNumberFrames * sizeof(float));
    rightChannel.mData = malloc(inNumberFrames * sizeof(float));
    leftChannel.mDataByteSize = inNumberFrames * sizeof(float);
    rightChannel.mDataByteSize = inNumberFrames * sizeof(float);

    // Loop through each frame and generate the audio sample for each frequency
    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        float leftSample = 0.1f;
        float rightSample = 0.1f;

        // Mix each frequency into both the left and right channels
        for (double frequency : *frequencies) {
            float sample = static_cast<float>(sin(2.0 * M_PI * frequency * (i / kSampleRate)));
            leftSample += sample;  // Add to left channel
            rightSample += sample; // Add to right channel
        }

        // Scale the amplitude to avoid clipping (you can adjust this value)
        leftSample *= 0.1f; // Adjust amplitude
        rightSample *= 0.1f; // Adjust amplitude

        // Store the samples in the audio buffers
        static_cast<float*>(leftChannel.mData)[i] = leftSample;
        static_cast<float*>(rightChannel.mData)[i] = rightSample;
    }

    return noErr;
};

// Play a chord with multiple frequencies
void play_chord(std::vector<double>& frequencies, unsigned int duration, AudioUnit audioUnit = audio_output) {
    AudioComponentInstance audioUnitInstance = audioUnit;

    // Set up the callback for rendering multiple frequencies
    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = RenderAudioMult;
    callbackStruct.inputProcRefCon = reinterpret_cast<void*>(&frequencies);

    // Set the render callback property for the audio unit
    OSStatus result = AudioUnitSetProperty(audioUnitInstance, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0,
                                           &callbackStruct, sizeof(callbackStruct));
    if (result != noErr) {
        std::cerr << "Error setting render callback: " << result << std::endl;
        return;
    }

    // Start the audio unit to begin rendering the chord
    result = AudioOutputUnitStart(audioUnitInstance);
    if (result != noErr) {
        std::cerr << "Error starting audio unit: " << result << std::endl;
    }

    std::this_thread::sleep_for( std::chrono::milliseconds(duration) );

    play_frequency(0, 0);
};




};


#else
#error "GMENG AUDIO ENGINE: Unsupported Platform (only MacOS, Windows & Linux are supported).\nSEE https://gmeng.org"
#endif




/// MUSICAL NOTE FREQUENCY TABLE
/// FROM A0 TO G#0

#define MUSICAL_NOTE_A      27.50
#define MUSICAL_NOTE_As	    29.14

#define MUSICAL_NOTE_Bb     29.14
#define MUSICAL_NOTE_B	    30.87

#define MUSICAL_NOTE_C	    32.70
#define MUSICAL_NOTE_Cs  	34.65

#define MUSICAL_NOTE_Db     34.65
#define MUSICAL_NOTE_D	    36.71
#define MUSICAL_NOTE_Ds     38.89

#define MUSICAL_NOTE_Eb 	38.89
#define MUSICAL_NOTE_E	    41.20

#define MUSICAL_NOTE_F	    43.65
#define MUSICAL_NOTE_Fs     46.25

#define MUISCAL_NOTE_Gb 	46.25
#define MUSICAL_NOTE_G	    49.00
#define MUSICAL_NOTE_Gs     51.91

#define MUSICAL_NOTE_Ab 	51.91

/// Returns the frequency of a musical note scaled to the desired octave.
#define FROM_OCTAVE(note_freq, octave_number) ((note_freq) * std::pow(2, (octave_number)))

