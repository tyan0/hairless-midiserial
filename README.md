# What is this?

This is a fork from [Hairless MIDI<->Serial Bridge](http://projectgus.github.io/hairless-midiserial/). While the purpose of the original Hairless MIDI<->Serial Bridge is to connect serial devices like Arduinos, this fork aims to connect MIDI sound module such as Roland Sound Canvas seriese and YAMAHA MU seriese via serial port.

Roland Sound Canvas seriese and YAMAHA MU seriese support controlling multi-port (16 parts per port) via single serial port connection. So this fork adds support for the multi-port.

Hairless MIDI<->Serial Bridge assumes to be used with virtual MIDI ports.
In Windows, you can use [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html) to create virtual MIDI ports.
In OS X, use IAC Driver. For Linux, MIDI Through port, such as `14:0 Midi Through` can be used.

The flow of the MIDI signal is as follows.

[MIDI Application]->[loopMIDI]->[Hairless MIDI<->Serial Bridge]->(Serial port)->[MIDI Sound Module]

# Building Hairless Bridge from source

Hairless uses git submodules for library dependencies, so you should use `git clone --recursive URL` when cloning from Github. Alternatively, you can run `git submodule update --init` in order to fetch the submodules to an already-cloned directory.

This fork of Hairless Midiserial Bridge can be built with Qt4 or Qt5.

The Qt package should contain all dependencies, the graphical IDE "Qt Creator" or the program "qmake" can be used to compile the project hairless-midiserial.pro.

On Windows I recommend building with the [MingGW compiler](http://www.mingw.org/). I use MinGW compiler under [MSYS2](https://msys2.github.io/).

To build Hailess MIDI<->Serial Bridge, follow the steps:
* qmake
* make all

# Libraries

* [qextserialport](https://code.google.com/p/qextserialport/) is hosted on Github and is linked into the source tree as a git submodule.

* [The RtMidi library](https://github.com/thestk/rtmidi) is hosted on Github and is linked into the source tree as a git submodule.

Both libraries are small so they compiled as source files directly into Hairless Bridge, not linked as libraries.

# Release builds

The official releases are static linked, so they've actually been built against Qt versions that were compiled from source, configured with `-static` for static linking.
