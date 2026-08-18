#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"
export LD_LIBRARY_PATH="$HERE:$LD_LIBRARY_PATH"
cd "$HERE"
exec ./f1spirit "$@"
