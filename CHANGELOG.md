<div align="center">
    <img src="https://allahcorp.com/static/assets/gmeng-more-hd.png##" style="max-width: 900px;">
    <h2>Gmeng Changelog</h2>
</div>

This document was created at `25/09/2024`.
Latest entry is dated `12/10/2024`.

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
