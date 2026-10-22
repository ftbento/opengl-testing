.PHONY: build run clean

build:
	cmake -S . -B build -G Ninja
	cmake --build build
	ln -sfn build/compile_commands.json compile_commands.json

run: build
	mangohud ./build/3DTest
#	vblank_mode=0 mangohud ./build/3DTest

run-wsl: build
	nix run --impure github:guibou/nixGL -- ./build/3DTest

clean:
	rm -rf build compile_commands.json
