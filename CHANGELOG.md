<div align="center">
    <img src="https://allahcorp.com/static/assets/gmeng-more-hd.png##" style="max-width: 900px;">
    <h2>Gmeng Changelog</h2>
</div>

This document was created at `25/09/2024`.
Latest entry is dated `25/10/2024`.

**25/10/2024 (release)** - Gmeng version 10.0.0 Released.
> Gmeng now has a Game Event Loop System, which allows users to implement `update, fixedUpdate, init, deinit, reset, reload, keypress, mouse click, etc..` event handlers
> to their games. refer to the `tests/event_loop.cpp` test file for a prototype game using this functionality.
> the `__GMENG_ALLOW_LOG__` definition has been removed, and replaced with `__GMENG_DISABLE_LOG__`. this is due to logging being disabled
> by default which is not the approach I want to take when moving to the user-scaled testing era of the engine.
> A new namespace, `Gmeng::TerminalUtil` has been implemented for mouse tracking, raw mode stdin settings and non-blocking input mode controllers.
> Gmeng also now has a general-purpose Util library in the namespace `Gmeng::Util`, which currently has limited methods but I will add more utility functions as I think of them/find requirements for them.

**12/10/2024 (module)** - Gmeng now includes `catriverr/noble` as a submodule for scripting.
> `catriverr/noble` is the index for the Noble scripting language, which I am working on importing to Gmeng for
> scripting in game instances without rebuilding compiled game executables. This is particularly essential
> for minor changes to hooks of game events, imagine `button press` or `player move` or `entity move`... etc.
> Scripting will not be extensive enough, however, to allow for programming entire games within them, since that's dumb & NOT good practice.

**28/09/2024 (info)** - repository cleanup
> CODE_OF_CONDUCT and CONTRIBUTING have been moved to the `.github` directory
> so the repository is less packed.<br> The `.clangd` file has also been removed as it previously remained unused.

**25/09/2024 (info)** - swapped to `Zlib License`
> Gmeng now uses the Zlib License as it better fits its use cases.

**25/09/2024 (info)** - added `CHANGELOG.md`
> I will now be logging changes to the engine to this file since we have a stable version of Gmeng (currently version 9.1.0)
