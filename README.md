[<img width="516" alt="banner" src="https://github.com/konovalov-aleks/reSL/assets/11459433/eea5c05c-8175-4417-9fed-990ca22190e5">](https://konovalov-aleks.github.io/)

# reSL
The reverse engineered DOS game [ShortLine v1.1. (DOKA 1992)](https://www.myabandonware.com/game/shortline-railroad-1i6)

**You can try the live demo here: https://konovalov-aleks.github.io/**

The original game can be downloaded from the link below (version 1.1): 
https://www.old-games.ru/game/download/1232.html

## Current status

<img width="640" alt="reSL" src="https://github.com/konovalov-aleks/reSL/assets/11459433/35f3f24b-b513-44ed-a5a8-17c03803bbab">

The code has been fully restored. The game is now being adapted to work on touch-controlled devices.

## Idea and goals of the project:

This project does not pursue any profit and will not be used for commercial purposes.
The goal of the project is to gain fun and experience, as well as to port my favorite childhood game to modern mobile platforms, adapt it to touch controls.

## Why did I take v1.1, not latest v2.0?

I initially started reverse engeneering the second version. But I soon realized that:
* this game is a bit different from the one I played in childhood.
* moreover it works much worse - there are no animations, the screen blinks when switching menus.
* v2.0 is written on Pascal using BGI library. But v1.1 is written on TurboC using assembler parts for working with graphics and to implement some tricky mechanics (ShortLine even uses coroutines written on assembler!).

So, v1.1 works better and it's much more interesting to decompile.

## Differences from the original game

The aim of this project is to recreate the game as close as possible to the original ShortLine.
Therefore, the existing differences between reSL and the original game are due to one of following reasons:
* the original game was written for DOS, but reSL should be cross-platform
* the original game has some obvious bugs and it's weird not to fix them.

### Fixed bugs of the original game

1. If the last game session was stopped when trains are waiting in the first entrance (the yellow dispatcher is showing the flag), then when starting a new game the program will freeze.
2. The loading screen is momentarily drawn in incorrect colors.

## Reverse engineering process

md5 hash of the file I decompiled (see the link to download above):
> d3516ca38a6e17a7326141794a041212  SL.EXE

I use Ghidra as a main tool:
https://github.com/NationalSecurityAgency/ghidra

At the first step I want to restore the code as close to original program as possible. At this stage it's useful to use a code structure that is close to the original. Thus, I even currently use an ineffective "emulator" of the VGA adapter instead of normal graphics functions.

Every function/global variable has a comment like `/* 18fa:08d6 */`. This is an address of the function/variable in the original binary. It helps a lot not to be confused while working and also might be interesting for newbies who want to compare the restored code with how it looks in the decompiler.

<img width="1714" alt="Снимок экрана 2024-05-04 в 01 45 21" src="https://github.com/konovalov-aleks/reSL/assets/11459433/8573c2d1-270a-4dd9-b9cf-f0b8bc43b311">


After all the code has been rewritten, in the second stage, I will optimize the code for modern platforms and adapt the game to a touch interface.

## Building reSL

To build reSL you will need:
* modern C++ compiler with C++20 support (modern clang, GCC, MSVC)
* [cmake](https://cmake.org/download/)
* [SDL2](https://github.com/libsdl-org/SDL/releases)

### MacOS

Open the Terminal application and execute the following instructions:
```
# install dependencies
brew install cmake sdl2
# prepare the build folder
mkdir -p build && cd build
# run cmake
cmake -DCMAKE_BUILD_TYPE=Release ..
# build the project
cmake --build . -j4
# enjoy!
./resl
```

### Linux (Ubuntu)

Open the terminal application and execute the following instructions:
```
# install dependencies
sudo apt install -y cmake libsdl2-dev
# prepare the build folder
mkdir -p build && cd build
# run cmake
cmake -DCMAKE_BUILD_TYPE=Release ..
# build the project
cmake --build . -j4
# enjoy!
./resl
```

### Windows

1. Install [Microsoft Visual Studio](https://visualstudio.microsoft.com/ru/downloads/) 2022 or newer.
2. Download and unpack the [SDL2 development package for VC](https://github.com/libsdl-org/SDL/releases) (the name should be something like "SDL2-devel-2.30.5-VC.zip")
3. Open the "X64 Native Tools Comand Prompt for VS 2022" terminal:
```
# prepare the build folder
mkdir -p build && cd build
# set the path to the folder where you unpacked the SDL2 library (replace the path with yours)
set SDL2_DIR=c:\projects\SDL2-2.30.5
# run cmake
cmake -DCMAKE_BUILD_TYPE=Release -GNinja ..
# build the project
cmake --build . -j4
# enjoy!
resl.exe
```

### WebAssembly (emscripten)

1. install emscripten
https://emscripten.org/docs/getting_started/downloads.html

2. in the terminal:
```
mkdir -p build && cd build
emcmake cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j4
```

## Legal notes:

The original game was created by **Doka** in 1992.

So, it's very old and now it's not clear how to contact the authors to get permission to reverse engineer their product. I hope they don't mind me giving the game a second life and adapting it to modern platforms.

**I don't plan to make any profit, this project is just for fun. The rights to all algorithms in the restored code belong to the authors of the original game (Andrei Snegov, DOKA).**
