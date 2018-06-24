
# Nabu

Another game engine, for 2D roguelike games.
It will eventually be strongly coupled with the game.
I aim for these properties :
 - Custom OpenGL framework, with 2D shaders
 - Authoritative server architecture
 - Support for procedural generation
 - Level / room editor

### Building

It only supports msvc for now. Commands to build and test :

```

md build
cd build
cmake .. [-DMERGE_CLIENT_SERVER=ON] (off by default)
cmake --build . --config [Debug, Release] -- /v:m
ctest -V -C [Debug, Release]

```
