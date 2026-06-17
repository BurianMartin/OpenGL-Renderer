BUILD_DIR := build

.PHONY: all run clean

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
