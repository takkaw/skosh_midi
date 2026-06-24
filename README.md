# `skosh_midi.h`
A single-header, MIDI 1.0 I/O library.

- Skosh Supported Platform: ALSA sequencer (Linux), CoreMIDI (macOS), WinMM (Windows).
- Skosh Features: No MIDI2.0, SysEx, virtual ports, timestamps, scheduled sends, callbacks, hotplug.
- Skosh MIDI messages: up to 3 bytes.
- Skosh Usage:
```c
  #define SKOSH_MIDI_IMPLEMENTATION
  #include "skosh_midi.h"
```
- Skosh Dependencies:
  - Linux: -lasound
  - macOS: -framework CoreFoundation -framework CoreMIDI
  - Windows(MSYS2/MinGW): -lwinmm
  - Windows(MSVC): winmm
- Skosh MIDI port API: count, name, open, close, recv, send. Returns: >=0 is OK, <0 is Error.
- Skosh Examples: See the `example` directory.

**Experimental:** The API may change without notice.
