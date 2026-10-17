#pragma once
// Synthesized electric-guitar solo, as a DI signal (straight from the pickup, no amp).
// Each string is a Karplus-Strong delay line read at a fractional, time-varying length,
// so bends, slides, vibrato and hammer-ons/pull-offs glide like a real string.
// shortcut: still a model, not a recording; record your own guitar for real tuning decisions.
#include <algorithm>
#include <cmath>
#include <vector>

namespace guitar {

constexpr int M = 8191;  // delay buffer mask (8192 samples: down to ~6 Hz at 48 kHz)

inline float rnd(unsigned &seed) { seed = seed * 1664525u + 1013904223u; return (seed >> 8) / float(1 << 24) * 2 - 1; }
inline float smooth(float x) { x = std::fmin(std::fmax(x, 0.f), 1.f); return x * x * (3 - 2 * x); }

struct String {
  std::vector<float> buf = std::vector<float>(M + 1, 0.f);
  int w = 0;

  // Injects one period of filtered noise. damp: the pick stops the old vibration (false for hammer-ons).
  void excite(float hz, float vel, float fs, unsigned &seed, bool damp) {
    int L = int(fs / hz);
    if (damp)
      for (int i = 0; i <= L + 2; i++) buf[(w - 1 - i) & M] *= 0.1f;
    std::vector<float> e(L);
    float z = 0, a = 0.2f + 0.6f * vel;  // harder pick = brighter attack
    for (auto &v : e) { z += a * (rnd(seed) - z); v = z; }
    int d = std::max(1, int(0.13f * L));  // picking near the bridge: comb filter
    for (int i = 0; i < L; i++) buf[(w - L + i) & M] += vel * (e[i] - (i >= d ? e[i - d] : 0));
  }

  float step(float hz, float t60, float fs) {
    float L = fs / hz - 0.5f;  // the 2-tap average below adds half a sample of delay
    float r = w - L;
    int i0 = int(std::floor(r));
    float fr = r - i0;
    float s0 = buf[i0 & M] + fr * (buf[(i0 + 1) & M] - buf[i0 & M]);
    float s1 = buf[(i0 - 1) & M] + fr * (buf[i0 & M] - buf[(i0 - 1) & M]);
    float y = std::pow(10.f, -3.f / (t60 * hz)) * 0.5f * (s0 + s1);  // decays 60 dB in t60 seconds
    buf[w++ & M] = y;
    return y;
  }
};

// kind: 'p' pick, 'h' hammer-on/pull-off (no pick), 's' slide from the string's previous pitch.
struct Note { float t; int str; float midi; char kind; float vel, dur, bend = 0, bendAt = 0, relAt = 0, vib = 0; };

inline float pitchAt(const Note &n, float prevMidi, float dt) {
  float m = n.midi;
  if (n.kind == 's') m = prevMidi + (n.midi - prevMidi) * smooth(dt / 0.08f);
  if (n.bend) {
    float b = smooth((dt - n.bendAt) / 0.13f);
    if (n.relAt > 0) b *= 1 - smooth((dt - n.relAt) / 0.15f);
    m += n.bend * b;
  }
  if (n.vib) m += n.vib * smooth((dt - 0.25f) / 0.35f) * std::sin(2 * 3.14159265f * 5.5f * dt);
  return m;
}

// A-minor pentatonic solo around the 5th fret, ~9 s. Strings: 0 = low E ... 5 = high E.
inline std::vector<float> solo(int fs) {
  const std::vector<Note> notes = {
      // opening whole-step bend D -> E with vibrato
      {0.00f, 3, 62, 'p', 0.95f, 1.00f, 2, 0.04f, 0, 0.35f},
      // descending pull-off run
      {1.05f, 4, 67, 'p', 0.80f, 0.30f}, {1.30f, 4, 64, 'h', 0.35f, 0.30f},
      {1.60f, 3, 62, 'p', 0.75f, 0.30f}, {1.85f, 3, 60, 'h', 0.35f, 0.30f},
      {2.15f, 2, 57, 'p', 0.85f, 1.00f, 0, 0, 0, 0.45f},
      // slide up, hammer-ons climbing
      {3.20f, 3, 60, 'p', 0.80f, 0.20f}, {3.38f, 3, 62, 's', 0.00f, 0.40f},
      {3.80f, 4, 64, 'p', 0.80f, 0.25f}, {4.02f, 4, 67, 'h', 0.40f, 0.30f},
      {4.35f, 5, 69, 'p', 0.85f, 0.25f}, {4.57f, 5, 72, 'h', 0.40f, 0.30f},
      // bend and release on the B string
      {4.95f, 4, 67, 'p', 1.00f, 0.90f, 2, 0.03f, 0.50f},
      // blues curl, then back down
      {5.90f, 3, 62, 'p', 0.80f, 0.30f, 0.5f, 0.02f},
      {6.20f, 3, 60, 'p', 0.70f, 0.28f}, {6.50f, 2, 57, 'p', 0.75f, 0.25f}, {6.75f, 2, 55, 'h', 0.35f, 0.25f},
      // slide into the root with wide vibrato
      {7.00f, 2, 57, 's', 0.00f, 1.90f, 0, 0, 0, 0.55f},
  };
  const float t60[6] = {4.0f, 3.6f, 3.2f, 2.8f, 2.4f, 2.0f};
  float end = 0;
  for (auto &n : notes) end = std::fmax(end, n.t + n.dur);
  std::vector<float> out(size_t((end + 0.6f) * fs), 0.f);

  String strs[6];
  const Note *cur[6] = {};
  float lastMidi[6] = {}, prevMidi[6] = {};
  unsigned seed = 12345;
  size_t next = 0;
  for (size_t i = 0; i < out.size(); i++) {
    float t = float(i) / fs;
    for (; next < notes.size() && notes[next].t <= t; next++) {
      const Note &n = notes[next];
      prevMidi[n.str] = lastMidi[n.str];
      cur[n.str] = &n;
      float hz = 440 * std::pow(2.f, (n.midi - 69) / 12);
      if (n.kind == 'p') strs[n.str].excite(hz, n.vel, fs, seed, true);
      if (n.kind == 'h') strs[n.str].excite(hz, n.vel, fs, seed, false);
    }
    for (int s = 0; s < 6; s++) {
      if (!cur[s]) continue;
      float dt = t - cur[s]->t;
      lastMidi[s] = pitchAt(*cur[s], prevMidi[s], dt);
      float hz = 440 * std::pow(2.f, (lastMidi[s] - 69) / 12);
      out[i] += strs[s].step(hz, dt > cur[s]->dur ? 0.05f : t60[s], fs);  // fretting hand lets go = muted
    }
  }

  // pickup + cable: resonant low-pass around 4 kHz (RBJ biquad)
  float w0 = 2 * 3.14159265f * 4000 / fs, al = std::sin(w0) / (2 * 2.0f), c = std::cos(w0);
  float b0 = (1 - c) / 2 / (1 + al), b1 = (1 - c) / (1 + al), a1 = -2 * c / (1 + al), a2 = (1 - al) / (1 + al);
  float x1 = 0, x2 = 0, y1 = 0, y2 = 0, peak = 1e-9f;
  for (auto &v : out) {
    float y = b0 * v + b1 * x1 + b0 * x2 - a1 * y1 - a2 * y2;
    x2 = x1; x1 = v; y2 = y1; y1 = y; v = y;
    peak = std::fmax(peak, std::fabs(y));
  }
  for (auto &v : out) v *= 0.5f / peak;  // DI-like level: peaks around -6 dBFS
  return out;
}

}  // namespace guitar
