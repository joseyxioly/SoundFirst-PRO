from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'src/csurf_soundfirst.cpp').read_text()
f=s.split('void CSurf_SoundFirst::RefreshNiFocus')[1].split('void CSurf_SoundFirst::ReportNiConnect')[0]
(r/'tests/focus_source.inc').write_text('void CSurf_SoundFirst::RefreshNiFocus'+f)
