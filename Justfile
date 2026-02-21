run: build
    ./build/ui

build: configure
    cmake --build build

configure:
    cmake -B build -G Ninja
