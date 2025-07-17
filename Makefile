all: clean build .PHONY

build:
	cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug

.PHONY: 
	cmake --build build -v

clean:
	rm -rf build
