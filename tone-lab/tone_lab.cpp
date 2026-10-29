// Tone Lab: desktop app to tune pedal.h by ear and by eye.
// Audio plays through the real pedal code in real time; knobs change the sound while it plays.
// Usage: ./tuner [guitar.wav [real_pedal_output.wav]]   (or drag a WAV onto the window)
// A file named *_in.wav is paired automatically with *_out.wav: a recording of a real pedal to compare against.
#include "amp_sim.h"
#include "guitar_synth.h"
#include "pedal.h"
#include "wav.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "implot.h"
#include <SDL.h>
#include <SDL_opengl.h>

#include <atomic>
#include <complex>
#include <filesystem>
#include <regex>

namespace fs_ = std::filesystem;
constexpr int FS = 48000;
constexpr size_t SCOPE = 4096;              // live waveform ring buffer (power of two)
constexpr size_t EXCERPT = 10 * FS;         // analysis length: keeps knob drags responsive on long files
enum Source { DRY, MODEL, REAL };
static const char *SOURCE_NAMES[] = {"Dry", "Model", "Real pedal"};

// ---------- knobs: ranges and hints for each pedal.h parameter ----------
struct Knob { float Pedal::*m; const char *label, *fmt; float lo, hi; bool log, gain; const char *hint; };
struct PedalUI {
  const char *name; bool Pedal::*on; ImVec4 color;
  std::vector<Knob> panel;      // the real pedal's front panel, 0-10
  bool Pedal::*mode;            // a front-panel switch, if the real pedal has one
  const char *modeLabel, *modeHint;
  std::vector<Knob> internals;  // circuit values the panel sets; fine-tune here
};
#define PANEL(m, label, hint) {&Pedal::m, label, "%.1f", 0, 10, false, false, hint}

static const std::vector<PedalUI> PEDALS = {
    {"Tube Screamer (TS808/TS9)", &Pedal::tsOn, {0.30f, 0.77f, 0.45f, 1},
     {PANEL(tsDriveKnob, "DRIVE", "Clipping amount. Sets the circuit gain: 1 + (51k + 500k audio pot) / 4.7k = ~12x to ~118x."),
      PANEL(tsToneKnob, "TONE", "Darker <-> brighter: the low-pass after clipping (500 Hz .. 6 kHz)."),
      PANEL(tsLevelKnob, "LEVEL", "Output volume.")},
     nullptr, nullptr, nullptr,
     {{&Pedal::tsDrive, "Gain", "%.1fx", 1, 120, true, true, "Gain into the clipper (set by DRIVE)."},
      {&Pedal::tsHpHz, "Mid hump", "%.0f Hz", 150, 2000, true, false, "Lows below this pass clean. Higher = tighter, thinner. Original ~720 Hz (4.7k + 47nF)."},
      {&Pedal::tsToneHz, "Tone corner", "%.0f Hz", 400, 10000, true, false, "Low-pass after clipping (set by TONE)."},
      {&Pedal::tsLevel, "Output gain", "%.2fx", 0.01f, 2, true, true, "Set by LEVEL."}}},
    {"Klon Centaur", &Pedal::klonOn, {0.90f, 0.70f, 0.23f, 1},
     {PANEL(klGainKnob, "GAIN", "Dual-gang pot: raises the drive AND fades from clean into dirty, like the real Klon."),
      PANEL(klTrebleKnob, "TREBLE", "Treble cut <-> boost, flat at noon."),
      PANEL(klOutputKnob, "OUTPUT", "Output volume. The Klon has lots of clean headroom on tap.")},
     nullptr, nullptr, nullptr,
     {{&Pedal::klGain, "Gain", "%.1fx", 1, 60, true, true, "Drive into the germanium clipper (set by GAIN)."},
      {&Pedal::klBlend, "Clean <-> dirty", "%.2f", 0, 1, false, false, "Mix of the clean and clipped paths (set by GAIN)."},
      {&Pedal::klClip, "Clip threshold", "%.2f", 0.05f, 1, true, false, "Germanium diode voltage. Lower = clips earlier and grittier."},
      {&Pedal::klBpHz, "Low cut", "%.0f Hz", 40, 1500, true, false, "Lows below this skip the clipper, so low strings stay tight."},
      {&Pedal::klTreble, "Treble amount", "%.2fx", 0, 3, false, false, "1 = flat (set by TREBLE)."},
      {&Pedal::klTrebleHz, "Treble corner", "%.0f Hz", 200, 6000, true, false, "Where the treble control starts acting."},
      {&Pedal::klLevel, "Output gain", "%.2fx", 0.01f, 4, true, true, "Set by OUTPUT."}}},
    {"Keeley Katana", &Pedal::katanaOn, {0.42f, 0.61f, 1.00f, 1},
     {PANEL(kaVolumeKnob, "VOLUME", "Boost amount, up to ~+35 dB. Pushes the next pedal or the amp.")},
     &Pedal::kaDirty, "Pull: dirty boost", "The push/pull knob: pushed = clean boost, pulled = more gain with some grit.",
     {{&Pedal::kaBoost, "Boost gain", "%.2fx", 0.01f, 60, true, true, "Set by VOLUME."},
      {&Pedal::kaBright, "Bright", "%.2fx", 0.5f, 3, false, false, "1 = flat. Above 1 adds sparkle."},
      {&Pedal::kaShelfHz, "Bright corner", "%.0f Hz", 500, 10000, true, false, "Where the brightness starts."}}},
};
static const ImVec4 COLORS[] = {{0.60f, 0.64f, 0.70f, 1}, {1.00f, 0.42f, 0.51f, 1}, {0.35f, 0.85f, 0.95f, 1}};

// ---------- audio: the callback runs the live pedal; everything it reads is changed under SDL_LockAudioDevice ----------
struct Audio {
  SDL_AudioDeviceID dev = 0;
  std::vector<float> in, ref;  // ref: a real pedal's output, sample-aligned with `in` (optional)
  std::atomic<size_t> pos{0};
  bool playing = false, ampOn = true;
  int source = MODEL;
  float inGain = 1, vol = 0.4f, match[3] = {1, 1, 1};
  Pedal live{FS};
  AmpSim amp{FS};
  float scope[3][SCOPE] = {};
  size_t scopeW = 0;
} audio;

static void audioCallback(void *, Uint8 *stream, int len) {
  float *out = (float *)stream;
  for (int i = 0; i < len / 4; i++) {
    if (!audio.playing || audio.in.empty()) { out[i] = 0; continue; }
    size_t p = audio.pos;
    float x = audio.in[p] * audio.inGain;
    float src[3] = {x, audio.live.process(x), p < audio.ref.size() ? audio.ref[p] : 0};  // model always runs, so switching sources has no gap
    for (int k = 0; k < 3; k++) audio.scope[k][audio.scopeW & (SCOPE - 1)] = src[k];
    audio.scopeW++;
    audio.pos = (p + 1) % audio.in.size();
    float s = src[audio.source] * audio.match[audio.source];
    if (audio.ampOn) s = audio.amp.process(s);
    out[i] = std::fmax(-1.f, std::fmin(1.f, s * audio.vol));
  }
}
struct AudioLock { AudioLock() { SDL_LockAudioDevice(audio.dev); } ~AudioLock() { SDL_UnlockAudioDevice(audio.dev); } };

// ---------- analysis (offline renders through copies of the pedal) ----------
static void fft(std::vector<std::complex<double>> &a) {
  size_t n = a.size();
  for (size_t i = 1, j = 0; i < n; i++) {
    size_t bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) std::swap(a[i], a[j]);
  }
  for (size_t len = 2; len <= n; len <<= 1) {
    std::complex<double> wl = std::polar(1.0, -2 * M_PI / len);
    for (size_t i = 0; i < n; i += len) {
      std::complex<double> w = 1;
      for (size_t k = 0; k < len / 2; k++, w *= wl) {
        auto u = a[i + k], v = a[i + k + len / 2] * w;
        a[i + k] = u + v; a[i + k + len / 2] = u - v;
      }
    }
  }
}
// |X[k]| of x[0..n); with hann, scaled so a full-scale sine reads 1.0
static std::vector<double> mags(const float *x, size_t n, bool hann) {
  std::vector<std::complex<double>> a(n);
  double wsum = 0;
  for (size_t i = 0; i < n; i++) {
    double w = hann ? 0.5 - 0.5 * cos(2 * M_PI * i / (n - 1)) : 1;
    a[i] = x[i] * w; wsum += w;
  }
  fft(a);
  std::vector<double> m(n / 2);
  for (size_t k = 0; k < n / 2; k++) m[k] = std::abs(a[k]) * (hann ? 2 / wsum : 1);
  return m;
}
static double db(double v) { return 20 * log10(std::max(v, 1e-9)); }
static std::vector<double> welchDb(const std::vector<float> &x, size_t n = 4096) {
  std::vector<double> acc(n / 2, 0);
  int frames = 0;
  for (size_t s = 0; s + n <= x.size(); s += n / 2, frames++) {
    auto m = mags(&x[s], n, true);
    for (size_t k = 0; k < n / 2; k++) acc[k] += m[k] * m[k];
  }
  for (auto &v : acc) v = db(sqrt(v / std::max(frames, 1)));
  return acc;
}
static float rms(const std::vector<float> &x) {
  double s = 0;
  for (float v : x) s += v * v;
  return float(sqrt(s / std::max<size_t>(x.size(), 1)));
}
template <class P> static std::vector<float> run(P p, const std::vector<float> &x) {
  std::vector<float> y(x.size());
  for (size_t i = 0; i < x.size(); i++) y[i] = p.process(x[i]);
  return y;
}
static std::vector<float> excerpt(const std::vector<float> &v, size_t start, size_t n) {
  std::vector<float> e;
  for (size_t i = 0; i < std::min(n, v.size()); i++) e.push_back(v[(start + i) % v.size()]);
  return e;
}

struct Analysis {
  std::vector<float> toneIn, toneOut;
  std::vector<double> harmF, harmDb, freqs, chainDb, specF, spec[3];
  std::vector<std::vector<double>> soloDb;
  std::string harmText, matchText;
  double respTop = 12, specTop = 0;
};

static Analysis analyze(const Pedal &ui, float inGain, bool ampOn, float ampGain, float toneHz, float toneAmp) {
  Analysis a;
  bool hasRef = !audio.ref.empty();

  // excerpt from the playhead: dry, model, real pedal
  std::vector<float> src[3];
  src[DRY] = excerpt(audio.in, audio.pos, EXCERPT);
  for (auto &v : src[DRY]) v *= inGain;
  src[MODEL] = run(ui, src[DRY]);
  if (hasRef) src[REAL] = excerpt(audio.ref, audio.pos, EXCERPT);
  for (int k = 0; k < 3; k++) if (!src[k].empty()) a.spec[k] = welchDb(src[k]);
  for (size_t k = 0; k < a.spec[DRY].size(); k++) a.specF.push_back(double(k) * FS / 4096);
  for (int s = 0; s < 3; s++)
    for (size_t k = 1; k < a.spec[s].size(); k++) a.specTop = std::max(a.specTop, a.spec[s][k]);
  a.specTop = ceil(a.specTop / 10) * 10;

  // tone match: spectral difference model vs real, 100 Hz - 8 kHz, ignoring overall level
  if (hasRef) {
    std::vector<double> d;
    for (size_t k = 0; k < a.specF.size(); k++)
      if (a.specF[k] >= 100 && a.specF[k] <= 8000) d.push_back(a.spec[MODEL][k] - a.spec[REAL][k]);
    double mean = 0, dev = 0;
    for (double v : d) mean += v / d.size();
    for (double v : d) dev += fabs(v - mean) / d.size();
    char buf[160];
    snprintf(buf, sizeof buf, "   Model vs real pedal: tone difference %.1f dB (lower = closer), level %+.1f dB", dev, mean);
    a.matchText = buf;
  }

  // loudness matching (after the amp when it's on), so louder never sounds "better"
  float r[3];
  for (int k = 0; k < 3; k++) {
    if (src[k].empty()) { r[k] = 0; continue; }
    AmpSim amp(FS);
    amp.gain = ampGain;
    r[k] = ampOn ? rms(run(amp, src[k])) : rms(src[k]);
  }

  // test tone: steady-state sine for the clipping-shape and harmonics plots
  std::vector<float> sine(FS / 2);
  for (size_t i = 0; i < sine.size(); i++) sine[i] = toneAmp * sinf(2 * float(M_PI) * toneHz * i / FS);
  auto out = run(ui, sine);
  size_t cyc = size_t(3 * FS / toneHz);
  a.toneIn.assign(sine.end() - cyc, sine.end());
  a.toneOut.assign(out.end() - cyc, out.end());
  const size_t N = 8192;
  auto hm = mags(&out[out.size() - N], N, true);
  double bin = double(FS) / N;
  auto at = [&](double f) { long k = lround(f / bin); double m = 0; for (long j = k - 3; j <= k + 3; j++) if (j > 0 && j < (long)hm.size()) m = std::max(m, hm[j]); return m; };
  double h1 = std::max(at(toneHz), 1e-9), hp = 0;
  char buf[256];
  std::string parts;
  for (int k = 2; k * toneHz < FS / 2 && k <= 40; k++) {
    double h = at(k * toneHz);
    hp += h * h;
    if (k <= 5) { snprintf(buf, sizeof buf, "   H%d %.0f dB", k, db(h / h1)); parts += buf; }
  }
  snprintf(buf, sizeof buf, "THD %.1f %%", 100 * sqrt(hp) / h1);
  a.harmText = buf + parts;
  for (size_t k = 0; k < hm.size(); k++) { a.harmF.push_back(k * bin); a.harmDb.push_back(db(hm[k])); }

  // small-signal frequency response: impulse responses, chain + each pedal alone
  std::vector<float> imp(16384, 0.f);
  imp[0] = 1e-3f;
  auto resp = [&](const Pedal &p) {
    auto m = mags(run(p, imp).data(), imp.size(), false);
    std::vector<double> d;
    for (size_t k = 1; k < m.size(); k++) d.push_back(db(m[k] / 1e-3));
    return d;
  };
  for (size_t k = 1; k < imp.size() / 2; k++) a.freqs.push_back(double(k) * FS / imp.size());
  a.chainDb = resp(ui);
  for (auto &pu : PEDALS) {
    Pedal p = ui;
    p.tsOn = p.klonOn = p.katanaOn = false;
    p.*pu.on = true;
    a.soloDb.push_back(ui.*pu.on ? resp(p) : std::vector<double>());
  }
  double top = 6;
  auto upTo20k = [&](const std::vector<double> &d) { for (size_t k = 0; k < d.size() && a.freqs[k] < 20000; k++) top = std::max(top, d[k]); };
  upTo20k(a.chainDb);
  for (auto &d : a.soloDb) upTo20k(d);
  a.respTop = ceil(top / 6) * 6;

  AudioLock lock;
  for (int k = 0; k < 3; k++) audio.match[k] = r[k] > 0 ? r[DRY] / r[k] : 1;
  return a;
}

// ---------- input ----------
static std::string inputName, status;

static std::vector<float> readAt48k(const std::string &path) {
  int rate = FS;
  std::string err;
  auto x = readWav(path, rate, err);
  if (x.empty()) { status = path + ": " + err; return x; }
  if (rate == FS) return x;
  // shortcut: linear resampling, fine for listening; resample offline (e.g. sox) for precise measurements
  std::vector<float> r(size_t(double(x.size()) * FS / rate));
  for (size_t i = 0; i < r.size(); i++) {
    double s = double(i) * rate / FS;
    size_t j = size_t(s);
    float f = float(s - j);
    r[i] = x[j] * (1 - f) + (j + 1 < x.size() ? x[j + 1] : 0) * f;
  }
  return r;
}
static void setInput(std::vector<float> x, std::vector<float> ref, const std::string &name) {
  AudioLock lock;
  audio.in = std::move(x);
  audio.ref = std::move(ref);
  audio.pos = 0;
  if (audio.ref.empty() && audio.source == REAL) audio.source = MODEL;
  inputName = name;
}
static bool loadInput(const std::string &path, std::string refPath = "") {
  auto x = readAt48k(path);
  if (x.empty()) return false;
  std::string name = fs_::path(path).filename().string();
  size_t at = name.rfind("_in.");
  if (refPath.empty() && at != std::string::npos) {
    auto sibling = fs_::path(path).parent_path() / (name.substr(0, at) + "_out" + name.substr(at + 3));
    if (fs_::exists(sibling)) refPath = sibling.string();
  }
  std::vector<float> ref;
  if (!refPath.empty()) {
    ref = readAt48k(refPath);
    if (ref.size() != x.size()) { status = "reference length differs from input: ignored"; ref.clear(); }
    else name += "  +  real pedal: " + fs_::path(refPath).filename().string();
  }
  setInput(std::move(x), std::move(ref), name);
  status = "loaded " + fs_::path(path).filename().string();
  return true;
}

// ---------- presets ----------
static std::vector<std::string> listPresets() {
  std::vector<std::string> names;
  if (fs_::is_directory("presets"))
    for (auto &e : fs_::directory_iterator("presets"))
      if (e.path().extension() == ".cfg") names.push_back(e.path().stem().string());
  std::sort(names.begin(), names.end());
  return names;
}

// ---------- plot helpers ----------
static void line(const char *label, const std::vector<double> &x, const std::vector<double> &y, ImVec4 c, float w = 1.5f) {
  if (y.empty()) return;
  ImPlot::SetNextLineStyle(c, w);
  ImPlot::PlotLine(label, x.data(), y.data(), int(std::min(x.size(), y.size())));
}
static void lineF(const char *label, const float *y, size_t n, ImVec4 c) {
  ImPlot::SetNextLineStyle(c, 1.5f);
  ImPlot::PlotLine(label, y, int(n), 1000.0 / FS);
}
static void freqAxis() {
  ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
  ImPlot::SetupAxisLimits(ImAxis_X1, 20, 20000, ImPlotCond_Always);
  static const double ticks[] = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
  static const char *labels[] = {"20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k"};
  ImPlot::SetupAxisTicks(ImAxis_X1, ticks, 10, labels);
}
static void caption(const char *text) {
  ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
  ImGui::TextWrapped("%s", text);
  ImGui::PopStyleColor();
}
static bool knobRow(const char *label, float *v, float lo, float hi, const char *fmt, bool log, const char *hint) {
  ImGui::SetNextItemWidth(-130);
  bool changed = ImGui::SliderFloat(label, v, lo, hi, fmt, log ? ImGuiSliderFlags_Logarithmic : 0);
  ImGui::SetItemTooltip("%s", hint);
  return changed;
}

int main(int argc, char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) { fprintf(stderr, "SDL_Init: %s\n", SDL_GetError()); return 1; }
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_Window *window = SDL_CreateWindow("Tone Lab", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1500, 950,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  if (!window) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); return 1; }
  SDL_GLContext gl = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl);
  SDL_GL_SetSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr;
  for (const char *f : {"/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
                        "/usr/share/fonts/open-sans/OpenSans-Regular.ttf"})
    if (fs_::exists(f)) { io.Fonts->AddFontFromFileTTF(f, 17); break; }
  ImGui::StyleColorsDark();
  ImGui::GetStyle().FrameRounding = 4;
  ImGui::GetStyle().ChildRounding = 8;
  ImGui_ImplSDL2_InitForOpenGL(window, gl);
  ImGui_ImplOpenGL3_Init("#version 150");

  SDL_AudioSpec want{}, have{};
  want.freq = FS; want.format = AUDIO_F32SYS; want.channels = 1; want.samples = 256; want.callback = audioCallback;
  audio.dev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
  if (!audio.dev) status = std::string("no audio output: ") + SDL_GetError();
  else SDL_PauseAudioDevice(audio.dev, 0);

  Pedal ui(FS);
  const Pedal defaults(FS);
  auto presets = listPresets();
  int presetIdx = -1;
  char presetName[64] = "";
  float inDb = 0, toneHz = 220, toneAmp = 0.3f, ampGain = audio.amp.gain;

  auto push = [&] {  // ui knobs -> live pedal (by config text, so the live filter state is kept)
    ui.update();
    char cfg[2048];
    ui.dump(cfg, sizeof cfg);
    AudioLock lock;
    audio.live.apply(cfg);
  };
  auto loadPreset = [&](const std::string &name) {
    std::ifstream f("presets/" + name + ".cfg");
    std::string text((std::istreambuf_iterator<char>(f)), {});
    Pedal p(FS);
    int bad = p.apply(text.c_str());
    ui = p;
    push();
    snprintf(presetName, sizeof presetName, "%s", name.c_str());
    status = "preset " + name + (bad ? " (" + std::to_string(bad) + " bad lines ignored)" : "");
  };
  for (size_t i = 0; i < presets.size(); i++)
    if (presets[i] == "default") { presetIdx = int(i); loadPreset("default"); }

  if (argc > 1) loadInput(argv[1], argc > 2 ? argv[2] : "");
  if (audio.in.empty() && fs_::exists("recordings/ts9_in.wav")) loadInput("recordings/ts9_in.wav");
  if (audio.in.empty()) setInput(guitar::solo(FS), {}, "synth guitar solo");

  Analysis an;
  bool dirty = true, done = false;
  std::vector<float> scope[3];
  while (!done) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      ImGui_ImplSDL2_ProcessEvent(&e);
      if (e.type == SDL_QUIT) done = true;
      if (e.type == SDL_DROPFILE) { if (loadInput(e.drop.file)) dirty = true; SDL_free(e.drop.file); }
    }
    if (dirty) { an = analyze(ui, audio.inGain, audio.ampOn, ampGain, toneHz, toneAmp); dirty = false; }
    bool hasRef = !audio.ref.empty();
    {  // last 40 ms of what the audio thread played
      AudioLock lock;
      size_t n = FS / 25, w = audio.scopeW;
      for (int k = 0; k < 3; k++) {
        scope[k].resize(n);
        for (size_t i = 0; i < n; i++) scope[k][i] = audio.scope[k][(w - n + i) & (SCOPE - 1)];
      }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("main", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);
    bool keys = !io.WantTextInput;

    // ---- top bar: transport, sources, amp ----
    if (ImGui::Button(audio.playing ? "Stop (P)" : "Play (P)", {110, 0}) || (keys && ImGui::IsKeyPressed(ImGuiKey_P, false))) {
      AudioLock lock;
      audio.playing = !audio.playing;
    }
    int nSources = hasRef ? 3 : 2, pick = -1;
    if (keys && ImGui::IsKeyPressed(ImGuiKey_Space, false)) pick = (audio.source + 1) % nSources;
    for (int k = 0; k < 3; k++) {
      ImGui::SameLine();
      ImGuiKey key = ImGuiKey(ImGuiKey_1 + k);
      ImGui::BeginDisabled(k == REAL && !hasRef);
      bool on = audio.source == k;
      ImGui::PushStyleColor(ImGuiCol_Button, on ? ImVec4(COLORS[k].x * 0.55f, COLORS[k].y * 0.55f, COLORS[k].z * 0.55f, 1)
                                                : ImVec4(0.20f, 0.22f, 0.26f, 1));
      char label[48];
      snprintf(label, sizeof label, "%s (%d)", SOURCE_NAMES[k], k + 1);
      if (ImGui::Button(label, {130, 0}) || (keys && ImGui::IsKeyPressed(key, false))) pick = k;
      ImGui::PopStyleColor();
      ImGui::EndDisabled();
    }
    if (pick >= 0 && (pick != REAL || hasRef)) { AudioLock lock; audio.source = pick; }
    ImGui::SameLine();
    ImGui::TextDisabled("Space cycles");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110);
    ImGui::SliderFloat("Volume", &audio.vol, 0, 1, "%.2f");  // a float write the callback tolerates
    ImGui::SameLine();
    bool ampOn = audio.ampOn;
    if (ImGui::Checkbox("Amp + cab", &ampOn)) { AudioLock lock; audio.ampOn = ampOn; dirty = true; }
    ImGui::SetItemTooltip("Listening aid, not part of the pedal: an overdrive is made to push an amp.\nStraight into headphones it sounds thin and fizzy.");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    ImGui::BeginDisabled(!audio.ampOn);
    if (ImGui::SliderFloat("Amp gain", &ampGain, 1, 40, "%.1f", ImGuiSliderFlags_Logarithmic)) {
      AudioLock lock;
      audio.amp.gain = ampGain;
      dirty = true;
    }
    ImGui::SetItemTooltip("~2 clean, ~6 edge of breakup (classic for a Tube Screamer), 20+ crunch.");
    ImGui::EndDisabled();

    // ---- second row: position, input, presets ----
    float secs = audio.in.empty() ? 0 : float(audio.pos) / FS, len = float(audio.in.size()) / FS;
    ImGui::SetNextItemWidth(260);
    if (ImGui::SliderFloat("##pos", &secs, 0, len, "%.1f s")) {
      AudioLock lock;
      audio.pos = std::min(size_t(secs * FS), audio.in.size() - 1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Analyze here")) dirty = true;
    ImGui::SetItemTooltip("Plots and loudness matching use the 10 s from the playhead, taken when a knob last changed.");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(170);
    if (ImGui::Combo("##preset", &presetIdx, [](void *d, int i) { return (*(std::vector<std::string> *)d)[i].c_str(); },
                     &presets, int(presets.size())))
      { loadPreset(presets[presetIdx]); dirty = true; }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(140);
    ImGui::InputTextWithHint("##name", "preset name", presetName, sizeof presetName);
    ImGui::SameLine();
    if (ImGui::Button("Save preset")) {
      std::string name = presetName;
      if (!std::regex_match(name, std::regex("[A-Za-z0-9_-]{1,40}"))) status = "preset name: letters, digits, - and _ only";
      else {
        char cfg[2048];
        ui.dump(cfg, sizeof cfg);
        fs_::create_directories("presets");
        std::ofstream("presets/" + name + ".cfg") << "# saved by Tone Lab\n" << cfg;
        presets = listPresets();
        presetIdx = int(std::find(presets.begin(), presets.end(), name) - presets.begin());
        status = "saved presets/" + name + ".cfg";
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy config")) {
      char cfg[2048];
      ui.dump(cfg, sizeof cfg);
      ImGui::SetClipboardText(cfg);
      status = "config copied";
    }
    ImGui::SameLine();
    ImGui::TextDisabled("%s", status.c_str());

    ImGui::TextDisabled("Input: %s", inputName.c_str());
    ImGui::SameLine();
    if (ImGui::SmallButton("Real guitar (TS9 recording)")) { if (loadInput("recordings/ts9_in.wav")) dirty = true; }
    ImGui::SameLine();
    if (ImGui::SmallButton("Synth solo")) { setInput(guitar::solo(FS), {}, "synth guitar solo"); dirty = true; }
    ImGui::SameLine();
    ImGui::TextDisabled("or drag a WAV onto the window");
    ImGui::Separator();

    // ---- left: pedal cards ----
    ImGui::BeginChild("controls", {380, 0}, ImGuiChildFlags_None);
    for (auto &pu : PEDALS) {
      ImGui::PushID(pu.name);
      ImGui::BeginChild("card", {0, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
      ImGui::PushStyleColor(ImGuiCol_CheckMark, pu.color);
      if (ImGui::Checkbox("##on", &(ui.*pu.on))) { push(); dirty = true; }
      ImGui::PopStyleColor();
      ImGui::SameLine();
      ImGui::TextColored(pu.color, "%s", pu.name);
      ImGui::SameLine(ImGui::GetContentRegionAvail().x - 40);
      if (ImGui::SmallButton("Reset")) {
        for (auto &k : pu.panel) ui.*k.m = defaults.*k.m;
        for (auto &k : pu.internals) ui.*k.m = defaults.*k.m;
        if (pu.mode) ui.*pu.mode = defaults.*pu.mode;
        push(); dirty = true;
      }
      ImGui::BeginDisabled(!(ui.*pu.on));
      ImGui::PushStyleColor(ImGuiCol_SliderGrab, pu.color);
      ImGui::PushStyleColor(ImGuiCol_CheckMark, pu.color);
      for (auto &k : pu.panel)
        if (knobRow(k.label, &(ui.*k.m), k.lo, k.hi, k.fmt, k.log, k.hint)) { ui.applyPanel(); push(); dirty = true; }
      if (pu.mode) {
        if (ImGui::Checkbox(pu.modeLabel, &(ui.*pu.mode))) { push(); dirty = true; }
        ImGui::SetItemTooltip("%s", pu.modeHint);
      }
      if (ImGui::TreeNode("Circuit internals")) {
        for (auto &k : pu.internals) {
          char hint[256];
          snprintf(hint, sizeof hint, k.gain ? "%s\n= %+.1f dB" : "%s", k.hint, db(ui.*k.m));
          if (knobRow(k.label, &(ui.*k.m), k.lo, k.hi, k.fmt, k.log, hint)) { push(); dirty = true; }
        }
        caption("Turning a front-panel knob resets the values it controls.");
        ImGui::TreePop();
      }
      ImGui::PopStyleColor(2);
      ImGui::EndDisabled();
      ImGui::EndChild();
      ImGui::PopID();
    }
    ImGui::BeginChild("signal", {0, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
    ImGui::Text("Signal");
    if (knobRow("Input level", &inDb, -24, 12, "%+.1f dB", false,
                "Pickup output. Hotter pickups clip more: try +6 dB for humbuckers.\nKeep 0 dB when comparing against the real pedal recording.")) {
      AudioLock lock;
      audio.inGain = powf(10, inDb / 20);
      dirty = true;
    }
    if (knobRow("Test tone", &toneHz, 40, 2000, "%.0f Hz", true, "Frequency of the sine used by the clipping and harmonics plots.")) dirty = true;
    if (knobRow("Test level", &toneAmp, 0.01f, 1, "%.2f", true, "A hard strum is about 0.3-0.6.")) dirty = true;
    ImGui::Spacing();
    caption("Hover a knob for tuning hints. 1/2/3 or Space switch between dry, the model and the real pedal recording "
            "(loudness matched).");
    ImGui::EndChild();
    ImGui::EndChild();

    // ---- right: plots ----
    ImGui::SameLine();
    ImGui::BeginChild("plots");
    float h = std::max((ImGui::GetContentRegionAvail().y - 6 * ImGui::GetTextLineHeightWithSpacing()) / 3 - 8, 180.f);

    caption(audio.playing ? "Waveform: the last 40 ms you heard. Flattened peaks = clipping."
                          : "Waveform: the last 40 ms played (press Play). Flattened peaks = clipping.");
    if (ImPlot::BeginPlot("##wave", {-1, h})) {
      ImPlot::SetupAxes("ms", nullptr);
      ImPlot::SetupAxesLimits(0, 40, -1, 1, ImPlotCond_Always);
      for (int k = 0; k < (hasRef ? 3 : 2); k++) lineF(SOURCE_NAMES[k], scope[k].data(), scope[k].size(), COLORS[k]);
      ImPlot::EndPlot();
    }

    if (ImGui::BeginTable("grid", 2)) {
      ImGui::TableNextColumn();
      caption("Clipping shape: 3 cycles of the test tone. Round tops = soft clip (TS), flat tops = hard clip (Klon).");
      if (ImPlot::BeginPlot("##clip", {-1, h})) {
        ImPlot::SetupAxes("ms", nullptr);
        ImPlot::SetupAxesLimits(0, 1000.0 * an.toneIn.size() / FS, -1, 1, ImPlotCond_Always);
        lineF("sine in", an.toneIn.data(), an.toneIn.size(), COLORS[DRY]);
        lineF("model out", an.toneOut.data(), an.toneOut.size(), COLORS[MODEL]);
        ImPlot::EndPlot();
      }
      ImGui::TableNextColumn();
      caption(("Harmonics of the test tone: " + an.harmText).c_str());
      if (ImPlot::BeginPlot("##harm", {-1, h})) {
        ImPlot::SetupAxes("Hz", "dB");
        ImPlot::SetupAxesLimits(0, 10000, -100, 0, ImPlotCond_Always);
        line("model out", an.harmF, an.harmDb, COLORS[MODEL]);
        ImPlot::EndPlot();
      }
      ImGui::TableNextColumn();
      caption("Frequency response (small signal): the tone shaping before clipping. Look for the TS mid hump.");
      if (ImPlot::BeginPlot("##resp", {-1, h})) {
        ImPlot::SetupAxes("Hz", "dB");
        freqAxis();
        ImPlot::SetupAxisLimits(ImAxis_Y1, an.respTop - 48, an.respTop, ImPlotCond_Always);
        for (size_t i = 0; i < PEDALS.size(); i++)
          if (!an.soloDb[i].empty()) line((std::string(PEDALS[i].name) + " alone").c_str(), an.freqs, an.soloDb[i], PEDALS[i].color);
        line("full chain", an.freqs, an.chainDb, COLORS[MODEL], 2.5f);
        ImPlot::EndPlot();
      }
      ImGui::TableNextColumn();
      caption(("Tone spectrum (10 s from the playhead)." + an.matchText).c_str());
      if (ImPlot::BeginPlot("##spec", {-1, h})) {
        ImPlot::SetupAxes("Hz", "dB");
        freqAxis();
        ImPlot::SetupAxisLimits(ImAxis_Y1, an.specTop - 80, an.specTop, ImPlotCond_Always);
        for (int k = 0; k < 3; k++) line(SOURCE_NAMES[k], an.specF, an.spec[k], COLORS[k]);
        ImPlot::EndPlot();
      }
      ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::End();

    ImGui::Render();
    glViewport(0, 0, int(io.DisplaySize.x * io.DisplayFramebufferScale.x), int(io.DisplaySize.y * io.DisplayFramebufferScale.y));
    glClearColor(0.07f, 0.08f, 0.09f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }

  if (audio.dev) SDL_CloseAudioDevice(audio.dev);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(gl);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
