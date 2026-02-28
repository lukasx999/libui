run: build
    ./build/ui

build: configure
    cmake --build build

web:
    emcmake cmake -Bbuild -GNinja
    cmake --build build

configure:
    cmake -Bbuild -GNinja
