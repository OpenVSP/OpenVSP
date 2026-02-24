#!/bin/bash
RESOURCES="$(dirname "$0")/../Resources"
cd "$RESOURCES"
exec ./vsp "$@"
