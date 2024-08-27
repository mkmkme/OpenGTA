#!/bin/sh

find programs src tests tools \
    \( -name '*.c' -or -name '*.cpp' -or -name '*.h' -or -name '*.hpp' \) \
    -exec clang-format -i {} + ;
