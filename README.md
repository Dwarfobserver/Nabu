
# Nabu

Another game engine, for 2D roguelike games.
It will eventually be strongly coupled with the game.
I aim for these properties :
 - Custom OpenGL framework, with 2D shaders
 - Authoritative server architecture
 - Support for procedural generation
 - Level / room editor

### Building

It only supports msvc for now. It builds with CMake and use
the python script 'full_build.py'. The build configuration is
written in 'config.json'.

### What is done

Working CMake setup for common, client and server parts with
it's tests.
