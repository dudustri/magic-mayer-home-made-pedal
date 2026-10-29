IMGUI  := third_party/imgui
IMPLOT := third_party/implot
CXXFLAGS := -O2 -Wall -std=c++17 -I..
GUIFLAGS := -I$(IMGUI) -I$(IMGUI)/backends -I$(IMPLOT) $(shell pkg-config --cflags sdl2)
GUILIBS  := $(shell pkg-config --libs sdl2) -lGL

LIBSRC := $(IMGUI)/imgui.cpp $(IMGUI)/imgui_draw.cpp $(IMGUI)/imgui_tables.cpp $(IMGUI)/imgui_widgets.cpp \
          $(IMGUI)/backends/imgui_impl_sdl2.cpp $(IMGUI)/backends/imgui_impl_opengl3.cpp \
          $(IMPLOT)/implot.cpp $(IMPLOT)/implot_items.cpp
LIBOBJ := $(patsubst %.cpp,build/%.o,$(notdir $(LIBSRC)))

all: sim tone-lab

sim: sim.cpp ../pedal.h wav.h guitar_synth.h
	$(CXX) $(CXXFLAGS) -o $@ sim.cpp

tone-lab: tone_lab.cpp ../pedal.h wav.h guitar_synth.h amp_sim.h $(LIBOBJ)
	$(CXX) $(CXXFLAGS) $(GUIFLAGS) -o $@ tone_lab.cpp $(LIBOBJ) $(GUILIBS)

# Dear ImGui + ImPlot compile once (pinned versions, fetched by `make deps`)
vpath %.cpp $(IMGUI) $(IMGUI)/backends $(IMPLOT)
build/%.o: %.cpp | build
	$(CXX) $(CXXFLAGS) $(GUIFLAGS) -w -c -o $@ $<
build:
	mkdir -p build

deps:
	git clone --depth 1 --branch v1.91.9b https://github.com/ocornut/imgui $(IMGUI)
	git clone --depth 1 --branch v0.16 https://github.com/epezent/implot $(IMPLOT)

# Real DI guitar + the same signal through a real Ibanez TS9 (GuitarML, GPL-3.0; see recordings/readme.md)
GUITARML := https://raw.githubusercontent.com/GuitarML/GuitarLSTM/main/data
recordings:
	curl -fL -o recordings/ts9_in.wav $(GUITARML)/ts9_test1_in_FP32.wav
	curl -fL -o recordings/ts9_out.wav $(GUITARML)/ts9_test1_out_FP32.wav

clean:
	rm -rf build sim tone-lab renders

.PHONY: all deps recordings clean
