[<img width="516" alt="banner" src="https://github.com/konovalov-aleks/reSL/assets/11459433/eea5c05c-8175-4417-9fed-990ca22190e5">](https://konovalov-aleks.github.io/)

# reSL

The reverse engineered DOS game ShortLine v1.1. (DOKA 1992)

<img width="640" alt="reSL" src="https://github.com/user-attachments/assets/774774a4-c3c7-41d5-a76e-4183ece09393">

**You can play the game here, right in your browser: https://konovalov-aleks.github.io/**

## Current status

The code has been fully restored. The game is now being adapted to work on touch-controlled devices.

The *main* branch contains an improved version of the game. You can see the version closest to the original game in the ["original" branch](https://github.com/konovalov-aleks/reSL/tree/original).

## Idea and goals of the project:

This project does not pursue any profit and will not be used for commercial purposes.
The goal of the project is to gain fun and experience, as well as to port my favorite childhood game to modern mobile platforms, adapt it to touch controls.

## Why did I take v1.1, not latest v2.0?

I initially started reverse engineering the second version. But I soon realized that:
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

md5 hash of the file I decompiled:
> d3516ca38a6e17a7326141794a041212  SL.EXE

The original game can be downloaded from the link below (version 1.1): 
https://www.old-games.ru/game/download/1232.html

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

### MacOS / Linux

Open the Terminal application and execute the following instructions:
```
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
2. Open the "X64 Native Tools Comand Prompt for VS 2022" terminal:
```
# prepare the build folder
mkdir -p build && cd build
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

### Interesting fact

In the [last chapter of the game manual "Inside SHORTLINE"](https://github.com/konovalov-aleks/reSL/blob/aff51dc2464df675437e3c788970195f1f74e7df/resources/RULES.TXT#L223C1-L230C35), the authors planned to describe the game mechanics in detail. But the section was never finished:

>  13. 'Inside SHORTLINE'
>
>This manual is to be supplied. It'll contain the detailed
>description of the model geometry and topology, math formulas and
>constant using in simulation, provides the better understanding
>of different aspects of the game and give strategy tips.

It seems that this project has realized the idea of ​​the authors of the original game. After all, open source code is the best description of algorithms, formulas and constants.
