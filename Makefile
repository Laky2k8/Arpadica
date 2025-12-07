# LAKY'S RAYLIB BUILDER MAKEFILE v1.0.0
# -------------------------------------

# Define variables
CXX = g++
INCLUDE = -Iinclude -Llib
SRC = $(wildcard src/*.cpp src/glad.c src/*.h src/*.hpp) 
LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm 
DEBUG_FLAGS = -g -std=c++17  -static-libgcc -static-libstdc++ -static # These will be used for the debug build (-g for debugging, -std=c++17 for C++17 standard)
RELEASE_FLAGS = -O2 -std=c++17 -static-libgcc -static-libstdc++ -static # These will be used for the release build (-O2 for optimization, -std=c++17 for C++17 standard)
DEBUG_OUT = main_debug.exe
RELEASE_OUT = main_release.exe

# Debug target
debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: LDFLAGS = 
debug: $(DEBUG_OUT)

$(DEBUG_OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC) $(LIBS) $(LDFLAGS) -o $(DEBUG_OUT)

# Release target
release: CXXFLAGS = $(RELEASE_FLAGS)
release: LDFLAGS = -mwindows
release: $(RELEASE_OUT)

$(RELEASE_OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC) $(LIBS) $(LDFLAGS) -o $(RELEASE_OUT)

# Clean target
clean:
	rm -f $(DEBUG_OUT) $(RELEASE_OUT)
