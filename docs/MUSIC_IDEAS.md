# Music companion ideas — proposals, not implemented

## Visual metronome and section timer

A useful first music mode is fully offline: one large BPM number, four gentle beat dots for4/4, an accented first beat, Tap Tempo and Start/Stop. Use soft pulse/motion rather than repeatedly flashing the full screen. Different meters and subdivision dots can be added without filling the screen with text.

A practice preset could be: warm-up3min → chorus at80BPM5min → same passage at90BPM5min → full run → break. The display shows only the current section, time remaining and beat. On-device taps advance/repeat a section; saved presets survive unplugging. With no speaker the first version is visual only; audio clicks would require added audio hardware.

This is suitable for practice cues. DAW/recording synchronization is a separate feature: don't promise sample-accurate sync over Wi-Fi or treat a visual cue as an audio master clock. A later Mac link could follow the DAW's transport/tempo.

## Mac little remote

Start with USB: reliable local communication and power on one cable. The ESP32 displays large Play, Stop, Record and Marker controls. A small paired Mac companion maps those events to the chosen application's supported commands, MIDI/OSC or configured shortcuts. The board's USB is a serial bridge, not a native USB keyboard, so the companion is required.

Useful pages: recording transport, rehearsal playback/loop points, and a simple busy/recording status display. Start with one selected application and five controls rather than universal automation. Shortcut-based control may need explicit Mac permission; protocol-based integration can avoid some global keyboard control. No OS permissions or integrations have been changed.

Wi-Fi remote can follow later, paired to the chosen Mac and limited to the local network. A Mac must be awake and its companion running. This proposal is separate from the bus-clock release; no remote or metronome firmware has been built in this task.
