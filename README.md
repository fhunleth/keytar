# keytar

This project handles events from a Rockband 3 Keytar via the USB interface (not the MIDI one).

## Building

### OSX

```sh
brew install hidapi
make
```

### Windows

Install MinGW

```sh
git submodule update --init --recursive
mingw32-make -f Makefile.mingw
```
