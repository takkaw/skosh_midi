# `skosh_midi.h`
A single-header, C11 MIDI 1.0 I/O library licensed under MIT.

- Skosh Supported Platform: ALSA sequencer (Linux).
- Skosh Features: No MIDI 2.0, SysEx, virtual ports, timestamps, scheduled sends, or callbacks.
- Skosh MIDI messages: up to 3 bytes.
- Skosh Usage:
```c
  #define SKOSH_MIDI_IMPLEMENTATION
  #include "skosh_midi.h"
```
- Skosh Dependencies: -lasound (Linux).
- Skosh MIDI port API: count, name, open, close, recv, send. Returns: 0=OK, negative=Error.
- Skosh Examples: See the `example` directory.

**Experimental:** The API may change without notice.
