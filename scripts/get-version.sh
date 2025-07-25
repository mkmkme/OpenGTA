#!/bin/sh -fu

die() {
    echo "$@" >&2
    exit 1
}

version_file=.version

if [ ! -d .git ]; then
    if [ -f "$version_file" ]; then
        cat "$version_file"
        exit 0
    else
        die "cannot get version info"
    fi
fi

version="$(git describe --tags --dirty=-dirty 2>/dev/null)"
res="$?"
if [ "${res}" -ne 0 ]; then
    num_commits="$(git log --oneline | wc -l)"
    res="$?"
    [ "${res}" -eq 0 ] || die "cannot get version, is git installed?"

    latest_commit="$(git rev-parse HEAD | head -c 7)"
    res="$?"
    [ "${res}" -eq 0 ] || die "cannot get commit hash"

    version="(none)-${num_commits}-g${latest_commit}"
fi
version="$(echo "$version" | sed -e 's/-/+/' -e 's/-/_/g')"

echo "${version}" | tee "${version_file}"
