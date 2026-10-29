// Usage: ./sim [dry_guitar.wav] [settings.cfg]   -> writes renders/input.wav + renders/out_*.wav for each effect combination.
// Without an input file it uses the synthesized guitar solo (guitar_synth.h).
// The .cfg's on/off flags are ignored here: each combination sets its own.
#include "guitar_synth.h"
#include "pedal.h"
#include "wav.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

static std::string readText(const char *path) {
  std::ifstream f(path);
  if (!f) { fprintf(stderr, "can't read %s\n", path); exit(1); }
  return std::string((std::istreambuf_iterator<char>(f)), {});
}

int main(int argc, char **argv) {
  {  // config parser self-check
    Pedal p(48000);
    assert(p.apply("tsDrive = 12  # note\n\n# comment\nbogus = 1\nklBpHz=300\n") == 1);
    assert(p.tsDrive == 12 && p.klBpHz == 300);
  }

  int fs = 48000;
  const char *wav = nullptr, *cfgPath = nullptr;
  for (int i = 1; i < argc; i++) (strstr(argv[i], ".cfg") ? cfgPath : wav) = argv[i];
  std::string err;
  std::vector<float> in = wav ? readWav(wav, fs, err) : guitar::solo(fs);
  if (in.empty()) { fprintf(stderr, "%s: %s\n", wav, err.c_str()); return 1; }
  std::string cfg = cfgPath ? readText(cfgPath) : "";
  printf("%zu samples @ %d Hz, config: %s\n", in.size(), fs, cfgPath ? cfgPath : "defaults");
  std::filesystem::create_directories("renders");
  writeWav("renders/input.wav", in, fs);

  struct Combo { const char *name; bool ts, klon, katana; };
  Combo combos[] = {{"dry", 0, 0, 0}, {"ts", 1, 0, 0}, {"klon", 0, 1, 0}, {"katana", 0, 0, 1},
                    {"ts_klon", 1, 1, 0}, {"all", 1, 1, 1}};
  for (auto &c : combos) {
    Pedal p(fs);
    if (int bad = p.apply(cfg.c_str())) { fprintf(stderr, "%d bad line(s) in %s\n", bad, cfgPath); return 1; }
    p.tsOn = c.ts; p.klonOn = c.klon; p.katanaOn = c.katana;
    std::vector<float> out(in.size());
    for (size_t i = 0; i < in.size(); i++) {
      out[i] = p.process(in[i]);
      assert(std::isfinite(out[i]) && fabsf(out[i]) <= 1.f);
      if (!c.ts && !c.klon && !c.katana) assert(fabsf(out[i] - Pedal::sat(in[i])) < 1e-6f);  // bypass = transparent
    }
    writeWav(std::string("renders/out_") + c.name + ".wav", out, fs);
    printf("wrote renders/out_%s.wav\n", c.name);
  }
}
