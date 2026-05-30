#!/usr/bin/env bash
# render.sh — compile a Faust DSP and render it to a WAV.
#
#   ./render.sh <dsp_path> <out.wav> <seconds> [param=value ...]
#
# dsp_path may be a bare name resolved against ../dsp and ../experiments,
# or an explicit path to a .dsp file. Param defaults: gate=1, amp=0.8.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
dsp_arg="$1"; out="$2"; secs="$3"; shift 3

# Resolve DSP path
if [[ -f "$dsp_arg" ]]; then dsp="$dsp_arg"
elif [[ -f "$here/../dsp/$dsp_arg.dsp" ]]; then dsp="$here/../dsp/$dsp_arg.dsp"
elif [[ -f "$here/../experiments/$dsp_arg.dsp" ]]; then dsp="$here/../experiments/$dsp_arg.dsp"
else echo "DSP not found: $dsp_arg" >&2; exit 1; fi

faust_inc="$(faust --includedir)"
gen="$(mktemp --suffix=_gen.cpp)"
bin="$(mktemp)"

# -I lets the DSP import justifier_filter.lib from native/dsp
faust -lang cpp -i -I "$here/../dsp" "$dsp" -o "$gen"
g++ -std=c++17 -O2 -I "$faust_inc" -DDSP_INCLUDE="\"$gen\"" "$here/render.cpp" -o "$bin"
"$bin" "$out" "$secs" 48000 "$@"
rm -f "$gen" "$bin"
