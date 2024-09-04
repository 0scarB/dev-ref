#!/bin/sh

set -eu

gcc -pg main.c -o polling-c-socket-server-example
./polling-c-socket-server-example $@
