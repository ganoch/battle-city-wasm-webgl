## setup

install `emscripten`

```sh
em++ src/main.cpp \
    -O2 \
    -sWASM=1 \
    -sMIN_WEBGL_VERSION=2 \
    -sMAX_WEBGL_VERSION=2 \
    -sALLOW_MEMORY_GROWTH=1 \
    -o main.js
```

run with python. wasm requires http(s) to be run
```sh
python3 -m http.server 8000
```