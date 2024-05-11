<img width="524" alt="shortline_banner" src="https://github.com/konovalov-aleks/reSL/assets/11459433/1d632634-559e-48ea-9234-471e14b8f2ab">

# reSL
The reverse engineered DOS game [ShortLine v1.1. (DOKA 1992)](https://www.youtube.com/watch?v=nfpi1QWPuz8)

The original game can be downloaded from the link below (version 1.1): 
https://www.old-games.ru/game/download/1232.html

## Current status

<img width="640" alt="reSL screenshot 2024-05-05 01 29 37" src="https://github.com/konovalov-aleks/reSL/assets/11459433/a429742a-ac7b-498b-833b-bdf6309b69cf">

### Progress:
1. Disassembled most of the binary code.
2. Reimplemented in C++ some functions:
   * loading the state from original game save files.
   * drawing the world (all objects on the game field).
   * drawing the game field header.
   * loading the original game records file and drawing records screen.
   * (WIP) train movement logic

## Idea and goals of the project:

This project does not pursue any profit and will not be used for commercial purposes.
The goal of the project is to gain fun and experience, as well as to port my favorite childhood game to modern mobile platforms, adapt it to touch controls.

## Why did I take v1.1, not latest v2.0?

I initially started reverse engeneering the second version. But I soon realized that:
* this game is different a bit from the one I played in childhood.
* moreover it works much worse - there are no animations, the screen blinks when switching menus.
* v2.0 is written on Pascal using BGI library. But v1.1 is written on TurboC using assembler parts for working with graphics and to implement some tricky mechanics (ShortLine even uses coroutines written on assembler!).

So, v1.1 works better and it's much more interesting to decompile.

## Reverse engineering process

md5 hash of the file I decompiled (see the link to download above):
> d3516ca38a6e17a7326141794a041212  SL.EXE

I use Ghidra as a main tool:
https://github.com/NationalSecurityAgency/ghidra

At the first step I want to restore the code as close to original program as possible. At this stage it's useful to use a code structure that is close to the original. Thus, I even currently use an ineffective "emulator" of the VGA adapter instead of normal graphics functions.

Every function/global variable has a comment like `/* 18fa:08d6 */`. This is an address of the function/variable in the original binary. It helps a lot not to be confused while working and also might be interesting for newbies who want to compare the restored code with how it looks in the decompiler.

<img width="1714" alt="Снимок экрана 2024-05-04 в 01 45 21" src="https://github.com/konovalov-aleks/reSL/assets/11459433/8573c2d1-270a-4dd9-b9cf-f0b8bc43b311">


After all the code has been rewritten, in the second stage, I will optimize the code for modern platforms and adapt the game to a touch interface.

## Legal notes:

The original game was created by **Doka** in 1992.

So, it's very old and now it's not clear how to contact the authors to get permission to reverse engineer their product. I hope they don't mind me giving the game a second life and adapting it to modern platforms.

**I don't plan to make any profit, this project is just for fun. The rights to all algorithms in the restored code belong to the authors of the original game (Andrei Snegov, DOKA).**
