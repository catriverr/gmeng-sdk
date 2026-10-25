#pragma once

#include "global.h"
#include "setup_class.h"
#include <map>

namespace Gmeng {
    /// Lua Script Routine. Can be ran periodically with Gmeng's built-in sol2
    /// lua support for various stuff like Entities, Models, and much more.
    struct Routine {
      public:
        /// ID of the routine.
        int id = g_mkid();

        /// priority of the subroutine. Higher priority routines
        /// are executed first, and lower priority ones last.
        int priority = 0;

        /// whether the routine has been initialized.
        bool init = false;
        /// name of the routine.
        std::string title;

        /// function to execute when running the subroutine.
        std::string entry_point;
        /// source code for the routine.
        std::string lua_source;

        /// value to determine how often the routine will run.
        /// Default is 1.0f, which is equal to 1 second. Value of
        /// 0.0f can be set, in which case the routine will be
        /// executed on every update() call.
        float period = 1.0f;
        /// progress of the subroutine, a counter updated with
        /// the default FIXED_UPDATE event handler which increments
        /// this value by 1.0f / display.FIXED_UPDATE_FPS every frame.
        /// progress / period returns how close to execution a subroutine
        /// is, in a 0.0 -> 1.0f range where 1.0f is the t = execute.
        float progress = 0.0f;
        /// whether the subroutine is an executable or a function.
        /// if set to false, the function becomes accessible from
        /// other subroutines. If for example a subroutine includes
        /// a utility function like `check_entity_status`, it can
        /// be executed by another subroutine instead.
        ///
        /// Note that non-executable functions must follow specific
        /// declaration order. This means that a subroutine referencing
        /// such a subroutine must come AFTER the non-executable subroutine
        /// being declared. To make sure of this order being followed,
        /// use the § `priority` parameter - higher priority means
        /// executing first, lower means last. Gmeng auto-sorts
        /// ONLY when using the default subroutine editor in the
        /// level manager as part of the builtin development console.
        /// sort by priority using `std::sort` on entity->routines to
        /// achieve the same functionality as the engine's implementation.
        bool executable = true;

        GMENG_INIT_TYPE( id, title, entry_point,
                         lua_source, period, priority,
                         progress, init, executable );
    };

    using RoutineList = std::vector<Routine>;
};

#ifndef GMENG_NO_SOL

#include "../utils/scripts.h"
static int __gmeng_define_lua_routine_method = ([]() -> int {
    using namespace Gmeng;
    using namespace Scripts;

    gmeng_lua_namespace.new_usertype<Routine>("Routine",
        "id", &Routine::id,
        "lua_source", &Routine::lua_source,
        "period", &Routine::period,
        "title", &Routine::title
    );

    return 0;
})();
#endif
