#!/bin/bash
# format.sh: Automatically format all C++ source and header files using clang-format

files=$(find . -type f \( -name "*.cpp" -or -name "*.h" \))
for file in $files; do
    clang-format -i "$file"
done
