#!/bin/bash
# lint.sh: Run cpplint on all C++ source and header files

files=$(find . -type f \( -name "*.cpp" -or -name "*.h" \))
for file in $files; do
    cpplint "$file"
done
