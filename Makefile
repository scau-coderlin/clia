all: clean build .PHONY

build:
	cmake -S . -B build

.PHONY: 
	cmake --build build

clean:
	rm -rf build
