#!/bin/bash

pushd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" || exit > /dev/null

mkdir -p build
cmake -B build
cmake --build build