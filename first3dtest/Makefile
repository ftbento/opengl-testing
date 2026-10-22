.PHONY: build run clean

build:
	cmake -S . -B build -G Ninja
	cmake --build build
	ln -sfn build/compile_commands.json compile_commands.json

run: build
	./build/3DTest

clean:
	rm -rf build compile_commands.json
