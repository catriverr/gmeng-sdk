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

#ifndef GMENG_NO_SOL

#include "../utils/scripts.h"
static int __gmeng_define_lua_profiler_method = ([]() -> int {
    using namespace Gmeng;
    using namespace Scripts;

    gmeng_lua_namespace.new_usertype<FrameProfiler>("FrameProfiler",
        "duration", &FrameProfiler::duration,
        "start_profile", &FrameProfiler::start_profile,
        "end_profile", &FrameProfiler::end_profile
    );
    return 0;
})();
#endif


#undef ZERO_MS
#undef ZERO_TP
