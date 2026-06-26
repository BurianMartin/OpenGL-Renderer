BUILD_DIR := build

.PHONY: all run clean debug release event noevent fps nofps FORCE

all: $(BUILD_DIR)/OpenGL_App

$(BUILD_DIR)/Makefile:
	cmake -B $(BUILD_DIR) -S .

$(BUILD_DIR)/OpenGL_App: $(BUILD_DIR)/Makefile FORCE
	cmake --build $(BUILD_DIR) -j$$(nproc)

run: all
	./$(BUILD_DIR)/OpenGL_App

clean:
	rm -rf $(BUILD_DIR)

FORCE:

# --- Build type (configure only, cmake caches the value) ---

debug:
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug

release:
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Release

# --- Feature flags (sticky until toggled off or clean) ---

event:
	cmake -B $(BUILD_DIR) -S . -DLOG_EVENTS=ON

noevent:
	cmake -B $(BUILD_DIR) -S . -DLOG_EVENTS=OFF

fps:
	cmake -B $(BUILD_DIR) -S . -DSHOW_FPS=ON

nofps:
	cmake -B $(BUILD_DIR) -S . -DSHOW_FPS=OFF