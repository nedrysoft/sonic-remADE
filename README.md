# sonic remADE

An (incomplete) implementation of the Sonic 1 megadrive engine using C++ & SDL.

## Why?!

For 30+ years since I first saw it running, it still blows me away with just how good it actually looks and I really wanted to understand how it worked and why it looked so good.

## How?!

The excellent [Sonic Physics Guide](https://info.sonicretro.org/Sonic_Physics_Guide) covers a lot of how the Sonic 1 engine works, a lot of information was incredibly useful for re-implementing the engine.

This C++ engine takes the raw data files that sonic used (often compressed) and converts these to easy to handle files which can be accessed and modified easily, along with the C++ implementation, these data files which are JSON and PNG files should make it a lot easier to understand how you would implement the engine.

## Requirements

To build this, you will need....

- A working C++17 compiler (I use macOS and therefore clang)
- CMake
- The SDL library
- The SDL image extension library
- The SDL mixer extension library
- The SDL font extension library
- The ImageMagick library
- The Magick++ extension library
- nlohmann-json C++ library

### macOS

You need a ImageMagick library installed that is 8 bit without hdri, the default imagemagick from brew will not work.  I have included a modified formula to use but I am not supplying instructions on how to use it as it will potentially break anything using the default brew version.

### Windows

Windows

### Tools

I wrote a tool that became a bit of a behemoth, when handed the sonic dissassembly and other files extracted from the ROM (decompressed) and then produces much friendlier files for you to tinker with, for example, it extracts all the map data, generates a PNG tileset from the raw images, produces a JSON file for maps, collision data, animations and so on, providing this engine with PNG files and JSON data that mean that it's much easier to tinker and play with the behaviour.

The tool was hacked and hacked and hacked, it took source files, parsed out stuff, I had to create a lot of manual rules for naming stuff in the resulting JSON so that the data was "nice to consume".  It's not pretty, it has a load of manual conditional compliation functions because my main focus was on getting data out.

I will add this tool as soon as possible.

## What would i have done different?

- Used the original fixed point so that game calculations are the same as the original, this really only becomes any issue when trying to replay the demo where precision I think ends up causing sync issues.
- Not used SDL sprites at all.  I should have emulated the VDP, this would have made many things less complicated especially when it comes to performance since using and updating lots of textures is so slow that using hardware rendering doesn't run acceptably, switching to software rendering resolves the performance.  The worst offender was the parallax background which I did actually re-write to use use a raw pixel buffer and I perform the necessary pixel pushing rather than using sprites.

## Debugging

Under macOS the guard malloc can be used to track memory issues related to buffer overflows, this can be injected into the application by setting an environment variable.

```bash
DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib
```

## Notes!

There are a couple of different disassemblies of Sonic, the original one is complicated and can be difficult to find stuff in.

A second new disassembly is available that is much easier to follow, but it uses macros a lot which while they make the code nice to read, it's more difficult to see raw values and therefore correlate stuff.

Sonics dynamic loading data is found here

```/s1disasm/_maps/Sonic - Dynamic Gfx Script.asm```

Sprite Mappings are found in this file:

```/s1disasm/_maps/Sonic.asm```

```bash
nemcmp -x "Enemy Buzz Bomber.bin" "./decompressed/Enemy Buzz Bomber.bin"
nemcmp -x "Enemy Chopper.bin" "./decompressed/Enemy Chopper.bin"
nemcmp -x "Enemy Crabmeat.bin" "./decompressed/Enemy Crabmeat.bin"
nemcmp -x "Enemy Motobug.bin" "./decompressed/Enemy Motobug.bin"
nemcmp -x "Enemy Newtron.bin" "./decompressed/Enemy Newtron.bin"
nemcmp -x "GHZ Bridge.bin" "./decompressed/GHZ Bridge.bin"
nemcmp -x "GHZ Edge Wall.bin" "./decompressed/GHZ Edge Wall.bin"
nemcmp -x "GHZ Flower Stalk.bin" "./decompressed/GHZ Flower Stalk.bin"
nemcmp -x "GHZ Giant Ball.bin" "./decompressed/GHZ Giant Ball.bin"
nemcmp -x "GHZ Purple Rock.bin" "./decompressed/GHZ Purple Rock.bin"
nemcmp -x "GHZ Spiked Log.bin" "./decompressed/GHZ Spiked Log.bin"
nemcmp -x "GHZ Swinging Platform.bin" "./decompressed/GHZ Swinging Platform.bin"
nemcmp -x "Hidden Bonuses.bin" "./decompressed/Hidden Bonuses.bin"
nemcmp -x "Lamppost.bin" "./decompressed/Lamppost.bin"
nemcmp -x "Monitors.bin" "./decompressed/Monitors.bin"
nemcmp -x "Rings.bin" "./decompressed/Rings.bin"
nemcmp -x "Signpost.bin" "./decompressed/Signpost.bin"
nemcmp -x "Sonic.bin" "./decompressed/Sonic.bin"
nemcmp -x "Spikes.bin" "./decompressed/Spikes.bin"
nemcmp -x "Spring Horizontal.bin" "./decompressed/Spring Horizontal.bin"
nemcmp -x "Spring Vertical.bin" "./decompressed/Spring Vertical.bin"
```

#### Animation Script Control

In order to easily create the sprite animations, the following commands are implemented in the animation script control.

afEnd = 0xFF : return to beginning of animation
afBack = 0xFE :	go back (specified number) bytes
afChange = 0xFD	: specified animation
afRoutine = 0xFC : increment routine counter
afReset = 0xFB : reset animation and 2nd object routine counter
af2ndRoutine : 0xFA : increment 2nd routine counter

### Sprites

http://md.railgun.works/index.php?title=VDP#Sprite_Sizes

## What Else?!

I've tried to make this engine easy to understand and easy to modify, trying to comment as much stuff as possible without writing war and peace.

## Credits

All of this would have been possible without the work of the people who have contributed to the [disassembly project]((https://github.com/sonicretro/s1disasm.git))([s](https://github.com/cvghivebrain/s1disasm.git) and certaintly not without the people who have put together the [Sonics Physics Guide](https://info.sonicretro.org/Sonic_Physics_Guide) which is an amazing piece of work.

Of course, absolutely none of this would be possible without the original author, architect and father of Sonic, who gave the world a game that most of us will remember just how far our jaws dropped when we saw or played sonic for the first time, the great Yuji Naka.

SEGA, because they don't seem overly worried about people providing dissassemblies of the original ROM, kudos to them.

### Links

The sonic retro guide can be found here [here.](https://info.sonicretro.org/Sonic_Physics_Guide)

The original disassembly can be found [here.](https://github.com/sonicretro/s1disasm.git)

The new disassembly can be found [here.](https://github.com/cvghivebrain/s1disasm.git)

An excellent C++ JSON parser. https://github.com/nlohmann/json

# SDL

The SDL from brew seems to be broken under macOS, it was working with 2.0.20 but they updated to a new version and it's broken.

```bash
wget https://github.com/libsdl-org/SDL/releases/download/release-2.0.20/SDL2-2.0.20.tar.gz -o SDL2.tar.gz
wget https://github.com/libsdl-org/SDL_image/archive/refs/tags/release-2.0.5.tar.gz -o SDL2_image.tar.gz
wget https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.0.18/SDL2_ttf-2.0.18.tar.gz -o SDL2_ttf.tar.gz
wget https://github.com/libsdl-org/SDL_mixer/archive/refs/tags/release-2.0.4.tar.gz -o SDL2_mixer.tar.gz

tar -xvf SDL2.tar.gz
tar -xvf SDL2_image.tar.gz
tar -xvf SDL2_mixer.tar.gz
tar -xvf SDL2_ttf.tar.gz

(cd SDL2 &&  \
./autogen.sh && \
./configure \
--prefix=/usr/local \
--enable-hidapi \
--without-x && \
make install)

(cd SDL2_image && \
./autogen.sh && \
./configure \
--disable-dependency-tracking \
--prefix=/usr/local \
--disable-imageio \
--disable-jpg-shared \
--disable-png-shared \
--disable-tif-shared \
--disable-webp-shared \
CFLAGS="-I/usr/local/include/SDL2" && \
make install)

(cd SDL2_mixer && \
./autogen.sh && \
./configure \
--prefix=/usr/local/sdl2_mixer \
--disable-dependency-tracking \
--enable-music-flac \
--disable-music-flac-shared \
--disable-music-midi-fluidsynth \
--disable-music-midi-fluidsynth-shared \
--disable-music-mod-mikmod-shared \
--disable-music-mod-modplug-shared \
--disable-music-mp3-mpg123-shared \
--disable-music-ogg-shared \
--enable-music-mod-mikmod \
--enable-music-mod-modplug \
--enable-music-ogg \
--enable-music-mp3-mpg123 \
CFLAGS="-I/usr/local/include/SDL2" && \
make install)

(cd SDL2_ttf \
./autogen.sh \
./configure \
--disable-freetype-builtin \
--prefix=/usr/local \
CFLAGS="-I/usr/local/include/SDL2" && \
make install)
```