from pathlib import Path
root=Path(__file__).resolve().parents[1]
s=(root/'src/csurf_soundfirst.cpp').read_text()
handler=s.split('void CSurf_SoundFirst::HandleNiCommand')[1].split('void CSurf_SoundFirst::AdoptHidBaseline')[0]
hid=s.split('void CSurf_SoundFirst::HandleHidReport')[1].split('    // --- VERIFIED A61')[0]
# Run the real session/DAW handler and real Instrument HID branch with mocked
# effect sinks. Custom mode remainder stays in the production source.
body='void CSurf_SoundFirst::HandleNiCommand'+handler+'\nvoid CSurf_SoundFirst::HandleHidReport'+hid+'\n}\n'
(root/'tests/routing_source.inc').write_text(body)
