#pragma once
#include <chrono>

#define ZERO_MS std::chrono::milliseconds(0)
#define ZERO_TP std::chrono::time_point<std::chrono::high_resolution_clock>( ZERO_MS )

namespace Gmeng {
    class FrameProfiler {
      private:
        using clock = std::chrono::high_resolution_clock;
        using timepoint = std::chrono::time_point<clock>;

        timepoint start_time;

      public:
        std::chrono::duration<float> duration = ZERO_MS;
        /// Called before frame creation
        void start_profile() {
            start_time = clock::now();
        };

        /// Called after frame creation
        float end_profile() {
            timepoint endTime = clock::now();
            duration = endTime - start_time;
            return duration.count();
        };
    };
}

#undef ZERO_MS
#undef ZERO_TP
