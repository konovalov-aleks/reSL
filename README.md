[<img width="516" alt="banner" src="https://github.com/konovalov-aleks/reSL/assets/11459433/eea5c05c-8175-4417-9fed-990ca22190e5">](https://konovalov-aleks.github.io/)

# reSL

The reverse engineered DOS game ShortLine v1.1 (DOKA 1992)

<img width="640" alt="reSL" src="https://github.com/user-attachments/assets/774774a4-c3c7-41d5-a76e-4183ece09393">

**You can play the game here, right in your browser: https://konovalov-aleks.github.io/**

Visit the [releases page](https://github.com/konovalov-aleks/reSL/releases/) to download the game.

## Current status

The code has been fully restored and adapted to work on touch-controlled devices ✅

The *main* branch contains an improved version of the game.

You can see the version closest to the original game in the ["original" branch](https://github.com/konovalov-aleks/reSL/tree/original). This can be useful if you want to study reverse engineering. Each function has a comment pointing to the function offset in the original binary - this will allow you to compare functions in the decompiler and in the reconstructed code.

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

The aim of this project is to recreate the game as close as possible to the original ShortLine, so that it would be possible to play on modern devices.
Therefore, the existing differences between reSL and the original game are due to one of following reasons:
* the original game was written for DOS, but reSL should be cross-platform.
* the original game has bugs and it's weird not to fix them.
* UI improvements to make it possible to play using touch controls.

### Improvements

1. Ability use mouse/touch in all menus (main menu, pause menu, records, archive, etc.).
2. Ability to control the game without a mouse (long touch instead of right mouse button click, swipe instead of simultaneous pressing of both mouse buttons).
3. Ability to use `<space>` button instead of simultaneous pressing of both mouse buttons on PC.
4. Grid lines in construction mode to simplify the positioning especially on touch devices.
5. Animation when closing the "Records" screen (the same animation as other screens have).
6. Better error handling (the original game could crash or reach an unpredictable state when trying to read a broken save file). ([issue #17](https://github.com/konovalov-aleks/reSL/issues/17))

### Fixed bugs of the original game

1. If the last game session was stopped when trains are waiting in the first entrance (the yellow dispatcher is showing the flag), then when starting a new game the program will freeze.
2. The loading screen is momentarily drawn in incorrect colors.
3. The interface was corrupted if the user deleted the last file in the "Archive" menu. ([issue #7](https://github.com/konovalov-aleks/reSL/issues/7))
4. The yellow entrance never spawns if the user removes the last archive file and runs a new game. ([issue #15](https://github.com/konovalov-aleks/reSL/issues/15))
5. The record screen was not drawn if there was no RECORDS.TBL file ([issue #37](https://github.com/konovalov-aleks/reSL/issues/37))

## Reverse engineering process

md5 hash of the file I decompiled:
> d3516ca38a6e17a7326141794a041212  SL.EXE

The original game can be downloaded from the link below (version 1.1): 
https://www.old-games.ru/game/download/1232.html

I used Ghidra as a main tool:
https://github.com/NationalSecurityAgency/ghidra

At the first stage, I had an aim to restore the code as close to the original program as possible. You can see that version in the ["original" branch](https://github.com/konovalov-aleks/reSL/tree/original).

Every function/global variable has a comment like `/* 18fa:08d6 */`. This is an address of the function/variable in the original binary. It helped a lot not to be confused while working and also might be interesting for newbies who want to compare the restored code with how it looks in the decompiler.

<img width="1714" alt="Снимок экрана 2024-05-04 в 01 45 21" src="https://github.com/konovalov-aleks/reSL/assets/11459433/8573c2d1-270a-4dd9-b9cf-f0b8bc43b311">

In the second stage, I adapted the game for touch controls, optimized the code and fixed some inherited issues.

## Building reSL

reSL only uses the standard C and C++ libraries, and the cross-platform SDL2 library. Thus, it can be easily compiled for many platforms without any difficulties. This chapter describes the build process for the most popular platforms. But the list of supported platforms is not limited to this set.

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

### Android

1. Install [Android Studio](https://developer.android.com/studio)
2. Open the project file "./android-project" in Android Studio
3. Choose your device or emulator, click `Run 'app'` button
<img width="419" alt="Снимок экрана 2025-01-03 в 14 11 09" src="https://github.com/user-attachments/assets/9935ee4e-f35e-4c65-ab8a-7f51eaf6b79b" />


### iOS

*The build is only possible on Mac computers!*
1. Install Xcode
2. Install [cmake](https://cmake.org/download/)
3. Open `Terminal` in the directory where you downloaded the source code
4. Run the following commands:
```
mkdir build_ios && cd build_ios
cmake .. -DCMAKE_TOOLCHAIN_FILE=../ios/cmake/ios.toolchain.cmake -G Xcode -DSDL2IMAGE_BACKEND_IMAGEIO=OFF -DPLATFORM=OS64COMBINED
```
5. Open the project `build_ios` in Xcode
6. Select the `resl` target, choose your device or emulator and click `Run` button
<img width="758" alt="Снимок экрана 2025-01-03 в 14 09 31" src="https://github.com/user-attachments/assets/06ec697d-6bee-42ad-9156-18b152794a22" />


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
