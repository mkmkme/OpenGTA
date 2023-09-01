#!/bin/sh
set -eux

cd "$(dirname "$0")"

for build_type in Debug Release; do
    conan install . \
        --update \
        --build=missing \
        --conf:host tools.cmake.cmake_layout:build_folder_vars="['settings.compiler']" \
        --settings:host "build_type=${build_type}"
done
