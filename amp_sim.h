#pragma once
// Simple tube amp + 4x12 cab, for listening only (not part of the pedal chain).
// An overdrive is designed to push an amp: straight into headphones it sounds thin and fizzy.
#include "pedal.h"

struct Biquad {  // RBJ cookbook filters
  enum Type { LP, HP, PEAK };
  float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
  void set(Type t, float fs, float hz, float q, float gainDb = 0) {
    float A = powf(10, gainDb / 40), w = 2 * 3.14159265f * hz / fs, c = cosf(w), al = sinf(w) / (2 * q), a0;
    if (t == LP) { b0 = (1 - c) / 2; b1 = 1 - c; b2 = b0; a0 = 1 + al; a1 = -2 * c; a2 = 1 - al; }
    else if (t == HP) { b0 = (1 + c) / 2; b1 = -(1 + c); b2 = b0; a0 = 1 + al; a1 = -2 * c; a2 = 1 - al; }
    else { b0 = 1 + al * A; b1 = -2 * c; b2 = 1 - al * A; a0 = 1 + al / A; a1 = -2 * c; a2 = 1 - al / A; }
    b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
  }
  float process(float x) {
    float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1; x1 = x; y2 = y1; y1 = y;
    return y;
  }
};

struct AmpSim {
  float gain = 6;  // preamp: ~2 clean, ~6 edge of breakup, 20+ crunch
  Biquad inHp, scoop, cabHp, cabLow, cabPresence, cabLp1, cabLp2;

  explicit AmpSim(float fs) {
    inHp.set(Biquad::HP, fs, 80, 0.7f);
    scoop.set(Biquad::PEAK, fs, 500, 0.8f, -4);        // tone-stack mid scoop
    cabHp.set(Biquad::HP, fs, 90, 0.7f);
    cabLow.set(Biquad::PEAK, fs, 120, 1.2f, 4);        // cabinet low resonance
    cabPresence.set(Biquad::PEAK, fs, 2300, 1.5f, 4);  // speaker presence peak
    cabLp1.set(Biquad::LP, fs, 4500, 0.7f);            // speakers roll off steeply above ~5 kHz: this hides the fizz
    cabLp2.set(Biquad::LP, fs, 4500, 0.7f);
  }

  float process(float x) {
    float u = inHp.process(x) * gain;
    u = Pedal::sat(u + 0.2f) - Pedal::sat(0.2f);  // biased like a tube: asymmetric, adds even harmonics
    u = Pedal::sat(scoop.process(u) * 1.5f) * 0.6f;  // power amp
    return cabLp2.process(cabLp1.process(cabPresence.process(cabLow.process(cabHp.process(u))))) * 0.8f;
  }
};
