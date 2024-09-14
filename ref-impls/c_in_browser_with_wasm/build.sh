#!/bin/sh

set -eu

clang --target=wasm32 \
    -nostdlib -Wl,--no-entry \
    main.c \
    -o main.wasm

