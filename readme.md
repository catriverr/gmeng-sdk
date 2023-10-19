# Gmeng Development Kit

Gmeng SDK is a development kit for Gmeng - a 2D game engine for the terminal - that includes the required utilities for developing a game in Gmeng.
following 2.0, the original gmeng repository is no longer up-to-date with the SDK version. please use this version instead of the legacy version.

## Installation 

installation is easy. Clone the repository with:
```sh
$ git clone https://github.com/catriverr/gmeng-sdk
```
However, there are a few requirements.
```
- Node.js & npm
- ts-node (npm install ts-node -g)
- gcc & g++ (required for building the engine source)
```
There are a few prebuilt executables, but it is recommended to build from source, as it is the most reliable way.

## Launching 

To launch the application, use:
```sh
$ ./sdk/start 
```
this will launch the application, that by default will load the map in the file `test.gm`, so do not change directories or remove the file.
If you wish to launch the map builder, use:
```sh
$ ./sdk/start -b
```
This will launch the map editor. If you wish to edit an already-existant file, just state the name of the file (without the .gm extension) to the editor's file name selection screen.
If you would like to create a new map, continue the editors instructions and it will lead you to the editor.

**Remember that `map size` and `player starting position` are variables that can not be changed after your initial input.**


### Thank you for checking out this repository.

