CXX = g++
CXXFLAGS = -fopenmp
LFLAGS = -lX11 -lGL -lpthread -lpng -lstdc++fs -lpulse -lpulse-simple -std=c++17 -O3 -mavx2 
WFLAGS = -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -static -std=c++17 -O3 -mavx2 
WINPATH = src
FILE = clock

all: msvc_debug run_windows

linux: $(WINPATH)/$(FILE).cpp
	$(CXX) $(CXXFLAGS) $(WINPATH)/$(FILE).cpp $(LFLAGS) -o bin/$(FILE)

msvc_release: $(WINPATH)/$(FILE).cpp
	cl /EHsc /openmp:llvm /O2 /Ot /std:c++17 /arch:AVX2 $(WINPATH)/$(FILE).cpp /Fdbin\vc100.pdb /Fobin\ /Fabin\ /link /SUBSYSTEM:windows /ENTRY:mainCRTStartup /out:bin\$(FILE).exe /pdb:bin\

msvc_debug: $(WINPATH)/$(FILE).cpp
	cl /EHsc /openmp:llvm /Ot /std:c++17 /arch:AVX2 $(WINPATH)/$(FILE).cpp project.res /Fdbin\vc100.pdb /Fobin\ /Fabin\ /link /SUBSYSTEM:windows /ENTRY:mainCRTStartup /out:bin\$(FILE).exe /pdb:bin\

msvc_debug_console: $(WINPATH)/$(FILE).cpp
	cl /EHsc /openmp:llvm /Ot /std:c++17 /arch:AVX2 $(WINPATH)/$(FILE).cpp project.res /Fdbin\vc100.pdb /Fobin\ /Fabin\ /link /out:bin\$(FILE).exe /pdb:bin\


windows: $(WINPATH)/$(FILE).cpp
	$(CXX) $(CXXFLAGS) $(WINPATH)/$(FILE).cpp $(WFLAGS) -o bin/$(FILE)

run: bin/$(FILE)
	./bin/$(FILE)

run_windows: bin/$(FILE).exe
	./bin/$(FILE).exe

install:
	sudo apt-get install build-essential libglu1-mesa-dev libpng-dev libmpich12 libpulse-dev

clean:
	rm ./bin
	rm -rf *.o
