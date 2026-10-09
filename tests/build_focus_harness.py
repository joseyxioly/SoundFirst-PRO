from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'src/csurf_soundfirst.cpp').read_text()
f=s.split('std::string CSurf_SoundFirst::FindNiInstance')[1].split('void CSurf_SoundFirst::ReportNiConnect')[0]
(r/'tests/focus_source.inc').write_text('std::string CSurf_SoundFirst::FindNiInstance'+f)
