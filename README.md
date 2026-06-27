# skosh_midi.h
A cross-platform single-header MIDI 1.0 I/O library in <500 lines.

- Supported platforms: ALSA sequencer (Linux), CoreMIDI (macOS), WinMM (Windows).
- MIDI messages: up to 3 bytes.
- Usage:
```c
  #define SKOSH_MIDI_IMPLEMENTATION
  #include "skosh_midi.h"
```
- Dependencies:
  - Linux: -lasound
  - macOS: -framework CoreFoundation -framework CoreMIDI
  - Windows (MSYS2/MinGW): -lwinmm
  - Windows (MSVC): winmm
- MIDI port API: count, name, open, close, recv, send. Returns: >=0 OK, <0 error.
- Ring buffer API: push, pop. Returns: 0 OK, -1 error.
- Limitations: No MIDI 2.0, SysEx, virtual ports, timestamps, scheduled sends, callbacks, hotplug.
- Examples: See the `example` directory.
