#!/bin/bash

set -e

echo target sampling rate is $1
echo input file is $2
echo output file is $3

echo first decoding mp3...
amp -convert -w $2 delme.wav
echo now converting into raw and resampling
sox delme.wav -r $1 -t raw -u -b -c 1 $3
rm delme.wav
echo done.
