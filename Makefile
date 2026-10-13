CXX = clang++
RC = llvm-rc
WINPATH = src
FILE = clock

# ---------- Windows (Clang + LLVM, MSVC ABI) ----------
CLANG_FLAGS = -std=c++17 -O3 -mavx2 -fms-runtime-lib=static -D_CRT_SECURE_NO_WARNINGS -Wno-microsoft-cast
CLANG_LIBS = -luser32 -lgdi32 -lopengl32 -lgdiplus -lshlwapi -ldwmapi -lwinmm
# /SUBSYSTEM:WINDOWS hides the console; /ENTRY keeps "int main()" as entry point
GUI_LINK = -fuse-ld=lld -Wl,/SUBSYSTEM:WINDOWS -Wl,/ENTRY:mainCRTStartup
CONSOLE_LINK = -fuse-ld=lld -Wl,/SUBSYSTEM:CONSOLE

# ---------- Linux (GCC) ----------
LINUX_CXX = g++
LFLAGS = -lX11 -lGL -lpthread -lpng -lstdc++fs -lpulse -lpulse-simple -std=c++17 -O3 -mavx2

ifeq ($(OS),Windows_NT)
SHELL = cmd.exe
.SHELLFLAGS = /c
endif

all: windows run_windows

bin:
	if not exist bin mkdir bin

# The .rc file embeds the icon and the .wav files into the .exe
bin/project.res: $(WINPATH)/project.rc $(WINPATH)/resource.h $(WINPATH)/pomodoro.ico $(WINPATH)/notification.wav $(WINPATH)/trumpets.wav | bin
	$(RC) -nologo -fo bin/project.res $(WINPATH)/project.rc

windows: $(WINPATH)/$(FILE).cpp bin/project.res
	$(CXX) $(CLANG_FLAGS) $(WINPATH)/$(FILE).cpp bin/project.res $(CLANG_LIBS) $(GUI_LINK) -o bin/$(FILE).exe

windows_console: $(WINPATH)/$(FILE).cpp bin/project.res
	$(CXX) $(CLANG_FLAGS) $(WINPATH)/$(FILE).cpp bin/project.res $(CLANG_LIBS) $(CONSOLE_LINK) -o bin/$(FILE).exe

windows_debug: $(WINPATH)/$(FILE).cpp bin/project.res
	$(CXX) -std=c++17 -O0 -g -mavx2 -fms-runtime-lib=static_dbg -D_CRT_SECURE_NO_WARNINGS $(WINPATH)/$(FILE).cpp bin/project.res $(CLANG_LIBS) $(CONSOLE_LINK) -o bin/$(FILE).exe

run_windows: bin/$(FILE).exe
	bin\$(FILE).exe

linux: $(WINPATH)/$(FILE).cpp
	$(LINUX_CXX) $(WINPATH)/$(FILE).cpp $(LFLAGS) -o bin/$(FILE)

run: bin/$(FILE)
	./bin/$(FILE)

install:
	sudo apt-get install build-essential libglu1-mesa-dev libpng-dev libmpich12 libpulse-dev

ifeq ($(OS),Windows_NT)
clean:
	if exist bin\$(FILE).exe del /q bin\$(FILE).exe
	if exist bin\$(FILE).pdb del /q bin\$(FILE).pdb
	if exist bin\$(FILE).ilk del /q bin\$(FILE).ilk
	if exist bin\project.res del /q bin\project.res
else
clean:
	rm -f bin/$(FILE) bin/project.res
endif

.PHONY: all windows windows_console windows_debug run_windows linux run install clean
