# TODO

# later TODO (not for now)
- multitouch support when erasing
- ~~**Text tool (ABC) keyboard issue**~~ - **ROOT CAUSE FIXED!** ✅
  - TEXT/LABEL were disabled in Child Mode
  - Enabled in `apply_child_mode_tool_filter()` and `child_tools` arrays
  - Also implemented keyboard debounce (500ms grace period)
  - See: `dev/TEXT_TOOL_KEYBOARD_FIX.md`
  - **Needs testing on Android device** - should now work!
- check why stamp and magic is not working (disabled all the time)

- die haupt datei in mehrere sinnvoll splitten

- die app umbenennen in "TuxPaint Kids"
- fastlane beschriebungen anpassen
- Die Brush Kategorien auch im Expert Mode umsortieren. Global definieren
- Beim allerersten Mal die App im Kids Mode in Kategorie 3 starten und gesperrt (locked).
- Meldung beim Start, die das 3s Longpress erklärt, falls der in den preferences aktiv ist
- beim laden schon bei einfach klick (nicht erst, wie jetzt bei doppelclick) das bild laden bzw. den dialog ob man vorher speichern will öffnen
- Label mit Text stürzt noch ab beim verlassen des labels

@TODO.md#L17-18 - Die Brush Kategorien auch im Expert Mode umsortieren. Global definieren, also wenn man auf paint drückt, dann tauchen rechts ja alle brushes auf, aber die sortierung ist dort nach brush-id, soll aber nun nach kategorien sortiert werden. @tuxpaint.c#L11980-12085  hier sind die hard codiert, aber die sollen einmal global definiert werden und dann auch in der brushes auswahl ansicht benuttz werden. beachte, dass jeder brush ein icon hat, eine unterschrift und einen extra bereich, der den touch-bereich ermittelt, damit man auch auf den richtigen button drückt

