<div align="center">
    <img src="https://allahcorp.com/static/assets/gmeng-more-hd.png##" style="max-width: 900px;">
    <h2>Gmeng Changelog</h2>
</div>

This document was created at `25/09/2024`.
Latest entry is dated `28/04/2025`.


**28/04/2025 (info)** Gmeng Now runs on `Linux` continously & `make setup` command
> The engine now is able to run on all Linux distributions. To make the
> installation & setup process easier, there is now a new command for setting
> up the environment required by gmeng to function. Run `sudo make setup`
> for the system to automatically detect your OS and install the required
> dependencies accordingly, as well as initialize the git submodules.


**06/04/2025 (release)** Gmeng 11.0.0: Entity handlers & Debug Rendering.
> The engine now has fully-functional Entities. Save files have also
> received the ability to save & load entities. Entities can be
> derived from the `Gmeng::Entity` class. They have to be registered
> to the EntityFactory with `REGISTER_ENTITY_TYPE( class_name )` (due to reflection purposes).
> Each entity handles its serialization by itself. By default, the basic variables within the
> `EntityBase` class will be saved. If derived entity classes require to save different info,
> they can add a new `serialize()` and `deserialize()` method to their classes with an `override` modifier
> to override the default serialization, and save changes. Keep in mind that this means you will have
> to save the default variables as well, not just extras.


**04/04/2025 (release)** Gmeng 10.4.0: Scripting & Client-side networking.
> The engine has had an overhaul regarding code cleanliness and readibility,
> and many of the functions now have comments about what they do.
> This helps both in documentation and for contributions to the engine.
> The engine now fully utilizes [`NOBLE Dynamic Shared Library Scripts`](https://github.com/catriverr/noble) and can run them
> by itself, and via commandline with the new `runscript` command.
> On another note, Client-Sided networking is now enabled by default, and users can
> connect to an EventLoop instance with a gmeng networking-style stream.
> Users can make use of the new `gmeng stream` subcommand in the CLI. EventLoop servers
> reside in the port space of `7388-7488`. Servers can be password-protected by setting the
> modifier `server_passkey` modifier in the EventLoop modifier list.



**31/12/2024 (release)** Gmeng 10.3.0: Gmeng now has an Audio Engine.
> The engine is able to play desired frequencies and chords of frequencies.
> The engine also has an example usage file in `tests/audio_test.cpp`. Check it out for examples.
> This version also comes with server stream capabilities which later on will be able to communicate with clients
> for Gmeng Multiplayer. The makefile also includes options to build specific files instead of defining a target file, usable with `make compile-file filename=<file.cpp>`.
> Check the readme for more information about this parameter.


**8/11/2024 (release)** Gmeng version 10.2.0 Released.
> Gmeng now allows writing & reading Level files fully.
> Level files are no longer stored as string-based parsable files, instead they are
> binaries. To import serialization & level file reloading capabilities, import the serialization header.
> ```c
> #include <gmeng/utils/serialization.cpp>
>
> int main(int argc, char** argv) {
>    Gmeng::Level level;
>    read_level_data("./level.glvl", level);
>    // logic..
>    write_level_data("./level.glvl", level);
> };
> ````
> You may also inspect, and although limitedly edit, .GLVL level files with the new CLI utility.
> To use this, compile the default target with `make all` or `make` and run the command with `./gmeng glvl <filename>`

**2/11/2024 (release)** Gmeng version 10.1.0 Released.
> Gmeng now has a Game State system which allows for **PARTIAL** reloading of level data.
> This format is in binary form, and written to files with the `writeout_properties` and `read_properties` methods.
> This version also introduces `gmeng preview` and `gmeng gamestate` subcommands for the CLI, allowing for previewing texture files and modifying
> the gamestate files, which needs to be done with proprietary utilities since it's in binary form. Use `gmeng gamestate <filename>` to modify one.

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
