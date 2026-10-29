#pragma once
// Minimal WAV I/O for the PC tools (not the ESP32). Reads 16/24-bit PCM or 32-bit float, first channel only.
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

// Returns an empty vector and sets `err` on failure.
inline std::vector<float> readWav(const std::string &path, int &fs, std::string &err) {
  std::ifstream f(path, std::ios::binary);
  std::vector<uint8_t> b((std::istreambuf_iterator<char>(f)), {});
  if (b.size() < 12 || memcmp(&b[0], "RIFF", 4) || memcmp(&b[8], "WAVE", 4)) { err = "not a WAV file"; return {}; }
  int fmt = 0, ch = 0, bits = 0;
  std::vector<float> out;
  for (size_t p = 12; p + 8 <= b.size();) {
    uint32_t len; memcpy(&len, &b[p + 4], 4);
    const uint8_t *d = &b[p + 8];
    if (p + 8 + len > b.size()) len = b.size() - p - 8;
    if (!memcmp(&b[p], "fmt ", 4) && len >= 16) {
      fmt = d[0] | d[1] << 8; ch = d[2] | d[3] << 8; memcpy(&fs, d + 4, 4); bits = d[14] | d[15] << 8;
      if (fmt == 0xFFFE && len >= 26) fmt = d[24] | d[25] << 8;  // WAVE_FORMAT_EXTENSIBLE: real format in the GUID
    } else if (!memcmp(&b[p], "data", 4)) {
      int frame = bits / 8 * ch;
      bool ok = (fmt == 1 && (bits == 16 || bits == 24)) || (fmt == 3 && bits == 32);
      if (!frame || !ok) { err = "unsupported WAV (need 16/24-bit PCM or 32-bit float)"; return {}; }
      for (size_t i = 0; i + frame <= len; i += frame) {
        const uint8_t *s = d + i;
        if (bits == 16) out.push_back(int16_t(s[0] | s[1] << 8) / 32768.f);
        else if (bits == 24) out.push_back(int32_t(s[0] << 8 | s[1] << 16 | s[2] << 24) / 2147483648.f);
        else { float v; memcpy(&v, s, 4); out.push_back(v); }
      }
    }
    p += 8 + len + (len & 1);
  }
  if (out.empty()) err = "no audio data";
  return out;
}

inline void writeWav(const std::string &path, const std::vector<float> &x, int fs) {
  std::ofstream f(path, std::ios::binary);
  auto u32 = [&](uint32_t v) { f.write((char *)&v, 4); };
  auto u16 = [&](uint16_t v) { f.write((char *)&v, 2); };
  uint32_t n = x.size() * 2;
  f << "RIFF"; u32(36 + n); f << "WAVEfmt "; u32(16); u16(1); u16(1); u32(fs); u32(fs * 2); u16(2); u16(16);
  f << "data"; u32(n);
  for (float v : x) u16(uint16_t(int16_t(std::fmax(std::fmin(v, 1.f), -1.f) * 32767)));
}
