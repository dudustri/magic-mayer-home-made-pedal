#pragma once
#include <math.h>
#include <stdio.h>
#include <string.h>

// Shared by the PC simulator, the tuner and the ESP32 firmware: keep it plain C++, no std containers.
struct Pedal {
  float fs;
  bool tsOn = false, klonOn = false, katanaOn = false;

  // Front panels, 0-10 like the real pedals. applyPanel() turns them into the circuit parameters below.
  float tsDriveKnob = 7, tsToneKnob = 5, tsLevelKnob = 6;      // TS808/TS9: DRIVE, TONE, LEVEL
  float klGainKnob = 5, klTrebleKnob = 5, klOutputKnob = 6;    // Klon: GAIN, TREBLE, OUTPUT
  float kaVolumeKnob = 5;                                       // Katana: VOLUME
  bool kaDirty = false;                                         // Katana: push/pull mode, clean or dirty boost

  // Circuit parameters (set by the panel; editable directly for fine-tuning)
  float tsDrive, tsLevel, tsHpHz = 720, tsToneHz;
  float klGain, klBlend, klClip = 0.35f, klTreble, klLevel, klBpHz = 400, klTrebleHz = 1000;
  float kaBoost, kaBright = 1.3f, kaShelfHz = 3000;

  float aTsHp, aTsTone, aKlBp, aKlTreble, aKaShelf;
  float zTsHp = 0, zTsTone = 0, zKlBp = 0, zKlTr = 0, zKaSh = 0;

  explicit Pedal(float fs) : fs(fs) { applyPanel(); }

  static float logTaper(float p) { return (powf(100, p) - 1) / 99; }  // audio-taper (A) pot, p = 0..1
  static float knobDb(float knob, float dbAt0, float dbAt10) {        // 0 = silent, like a volume pot at zero
    return knob <= 0 ? 0 : powf(10, (dbAt0 + (dbAt10 - dbAt0) * knob / 10) / 20);
  }

  // Front-panel knobs -> circuit parameters. Approximate mappings, tune them against real pedals.
  void applyPanel() {
    tsDrive = 1 + (51e3f + 500e3f * logTaper(tsDriveKnob / 10)) / 4.7e3f;  // TS808 gain: 1 + (51k + 500k-A pot) / 4.7k
    tsToneHz = 500 * powf(12, tsToneKnob / 10);                              // dark 500 Hz .. bright 6 kHz
    tsLevel = knobDb(tsLevelKnob, -24, 6);
    klGain = powf(60, klGainKnob / 10);                                        // dual-gang pot: more gain ...
    klBlend = 0.15f + 0.08f * klGainKnob;                                      // ... and more dirty in the mix
    klTreble = powf(3, klTrebleKnob / 5 - 1);                                  // 1/3 .. 3, flat at noon
    klLevel = knobDb(klOutputKnob, -24, 12);                                   // the Klon has lots of output on tap
    kaBoost = knobDb(kaVolumeKnob, -20, 35);                                   // up to ~+35 dB
    update();
  }

  // recompute filter coefficients after changing any *Hz parameter
  void update() {
    auto a = [this](float fc) { return 1 - expf(-2 * 3.14159265f * fc / fs); };
    aTsHp = a(tsHpHz); aTsTone = a(tsToneHz); aKlBp = a(klBpHz); aKlTreble = a(klTrebleHz); aKaShelf = a(kaShelfHz);
  }

  // Every tunable value by name: config files, the tuner and (later) serial tuning on the ESP32.
  // Panel knobs come first: setting one re-derives the circuit parameters, and the lines after it can override them.
  struct Param { const char *name; float *f; bool *b; };
  template <class F> void eachParam(F fn) {
    Param p[] = {{"tsDriveKnob", &tsDriveKnob, nullptr}, {"tsToneKnob", &tsToneKnob, nullptr},
                 {"tsLevelKnob", &tsLevelKnob, nullptr}, {"klGainKnob", &klGainKnob, nullptr},
                 {"klTrebleKnob", &klTrebleKnob, nullptr}, {"klOutputKnob", &klOutputKnob, nullptr},
                 {"kaVolumeKnob", &kaVolumeKnob, nullptr}, {"kaDirty", nullptr, &kaDirty},
                 {"tsOn", nullptr, &tsOn},           {"tsDrive", &tsDrive, nullptr},
                 {"tsHpHz", &tsHpHz, nullptr},       {"tsToneHz", &tsToneHz, nullptr},
                 {"tsLevel", &tsLevel, nullptr},     {"klonOn", nullptr, &klonOn},
                 {"klGain", &klGain, nullptr},       {"klBlend", &klBlend, nullptr},
                 {"klClip", &klClip, nullptr},       {"klBpHz", &klBpHz, nullptr},
                 {"klTreble", &klTreble, nullptr},   {"klTrebleHz", &klTrebleHz, nullptr},
                 {"klLevel", &klLevel, nullptr},     {"katanaOn", nullptr, &katanaOn},
                 {"kaBoost", &kaBoost, nullptr},     {"kaBright", &kaBright, nullptr},
                 {"kaShelfHz", &kaShelfHz, nullptr}};
    for (auto &e : p) fn(e);
  }

  bool set(const char *key, float v) {
    bool found = false;
    eachParam([&](Param &p) {
      if (strcmp(p.name, key)) return;
      if (p.f) *p.f = v; else *p.b = v != 0;
      found = true;
    });
    if (found && strstr(key, "Knob")) applyPanel();
    else if (found) update();
    return found;
  }

  // Applies "name = value" lines ('#' comments, blank lines ok). Returns the number of bad lines.
  int apply(const char *text) {
    int bad = 0;
    while (*text) {
      const char *eol = strchr(text, '\n');
      int n = eol ? int(eol - text) : int(strlen(text));
      char line[128], key[32];
      float v;
      snprintf(line, sizeof line, "%.*s", n, text);
      const char *s = line + strspn(line, " \t\r");
      if (*s && *s != '#' && (sscanf(s, "%31[^= \t] = %f", key, &v) != 2 || !set(key, v))) bad++;
      text += n + (eol ? 1 : 0);
    }
    return bad;
  }

  // Writes the current settings in the same "name = value" format apply() reads.
  int dump(char *buf, int len) {
    int n = 0;
    eachParam([&](Param &p) {
      if (n < len) n += snprintf(buf + n, len - n, "%s = %g\n", p.name, p.f ? *p.f : float(*p.b));
    });
    return n;
  }

  // rational tanh approximation (tanhf is slow on ESP32)
  static float sat(float x) { x = fminf(fmaxf(x, -3.f), 3.f); return x * (27 + x * x) / (27 + 9 * x * x); }
  static float lp(float x, float &z, float a) { return z += a * (x - z); }

  float process(float x) {
    // Tube Screamer: clip only the mids, add the clean signal back, tone lowpass
    if (tsOn) {
      float hp = x - lp(x, zTsHp, aTsHp);
      x = lp(x + sat(tsDrive * hp), zTsTone, aTsTone) * tsLevel;
    }
    // Klon: clean + hard-clipped blend, treble shelf
    if (klonOn) {
      float dirty = fminf(fmaxf(klGain * (x - lp(x, zKlBp, aKlBp)), -klClip), klClip);
      float y = x * (1 - klBlend) + dirty * klBlend;
      float lo = lp(y, zKlTr, aKlTreble);
      x = (lo + klTreble * (y - lo)) * klLevel;
    }
    // Katana: clean boost with a slight bright tilt
    if (katanaOn) {
      float lo = lp(x, zKaSh, aKaShelf);
      x = (lo + kaBright * (x - lo)) * kaBoost;
      if (kaDirty) x = 0.7f * sat(2.5f * x);  // dirty mode: extra gain into soft clipping (approximate)
    }
    return sat(x);  // output safety limiter
  }
};
