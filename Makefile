BUILD_DIR := build

.PHONY: all run clean debug release event noevent fps nofps doc FORCE test runtests demo lib

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

# --- Tests (BUILD_TESTS is sticky until toggled off or `make clean`) ---

test:
	cmake -B $(BUILD_DIR) -S . -DBUILD_TESTS=ON
	cmake --build $(BUILD_DIR) -j$$(nproc) --target EngineCore_tests
	./$(BUILD_DIR)/EngineCore_tests

# --- Demo app / library-only build (BUILD_DEMO is sticky until toggled off or `make clean`) ---

demo:
	cmake -B $(BUILD_DIR) -S . -DBUILD_DEMO=ON
	cmake --build $(BUILD_DIR) -j$$(nproc) --target OpenGL_App
	./$(BUILD_DIR)/OpenGL_App

lib:
	cmake -B $(BUILD_DIR) -S . -DBUILD_DEMO=OFF
	cmake --build $(BUILD_DIR) -j$$(nproc) --target EngineCore
	@echo "Library built at $(BUILD_DIR)/libEngineCore.a — demo app not built (BUILD_DEMO=OFF, sticky until 'make demo' or 'make clean')"

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

# --- Documentation ---

doc:
	@command -v doxygen >/dev/null 2>&1 || { echo "doxygen not found — install it (e.g. 'sudo apt install doxygen graphviz') then re-run 'make doc'"; exit 1; }
	@ver=$$(doxygen --version | grep -oE '^[0-9]+\.[0-9]+'); \
	major=$$(echo $$ver | cut -d. -f1); minor=$$(echo $$ver | cut -d. -f2); \
	if [ "$$major" -gt 1 ] || { [ "$$major" -eq 1 ] && [ "$$minor" -gt 15 ]; }; then \
		echo "Warning: doxygen $$ver is newer than 1.15, which is the newest version"; \
		echo "docs/doxygen-awesome-css (v2.4.2) is confirmed compatible with. Newer"; \
		echo "doxygen releases have broken the sidebar before (changed treeview HTML/CSS"; \
		echo "the theme doesn't know about yet) — if the sidebar looks empty/broken after"; \
		echo "this, that's almost certainly why. See docs/README.md."; \
	fi
	doxygen docs/Doxyfile
	@echo "Docs generated at docs/generated/html/index.html"