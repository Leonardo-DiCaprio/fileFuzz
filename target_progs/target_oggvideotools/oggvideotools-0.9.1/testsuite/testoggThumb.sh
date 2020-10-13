#!/bin/sh
../build/src/binaries/oggThumb -t 0.4,0.7,1 -s200x0 -n ducks_%.png ../testvideos/ducks_take_off_444_720p25.ogg
../build/src/binaries/oggThumb -t 0.4,0.7,1 -s0x200 -n aspect_%.png ../testvideos/pixel_aspect_ratio.ogg
../build/src/binaries/oggThumb -t 0.4,0.7,1,200 -n stockholm_%.png ../testvideos/stockholm-vfr.ogg
