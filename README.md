# Keytar

This project handles events from a Rockband 3 Keytar via the USB interface (not
the MIDI one). It generates keyboard press and release events so that the Keytar
can control things in other programs such as
[Scratch](https://scratch.mit.edu/).

The following works:

* Keytar keys (C3 to C5) - generate key presses for 'A' to 'Y' (25 keys total).
  5 keys can be pressed simultaneously
* Key velocity - only printed to console
* Pitch bend - only printed to console
* UDLR pad - generates arrow up/down/left/right key presses
* 12AB buttons - generates '1', '2', '3', '4' key presses
* '-' and '+' buttons - generates '-' and '+' key presses

Right now, only Windows generates key presses. OSX and Linux were used to test
out the Keytar USB interface so they may not be that far off.

## Building

### Windows

Install Mingw-w64 (https://mingw-w64.org/doku.php). I used the version that was
installed by Qt (https://www.qt.io/). In a shell window with the paths set for
Mingw:

```sh
git submodule update --init --recursive
mingw32-make -f Makefile.mingw
```

The `keytar.exe` binary is self-contained and can be copied by itself to any
other Windows system.

### OSX

```sh
brew install hidapi
git submodule update --init --recursive
make
```
