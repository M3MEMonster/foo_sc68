#!/usr/bin/python
#
# dump average spectrum of a raw pcm file
#
# by Antti Lankila
#

from scipy.signal import get_window, fft
from scipy import array
import math, sys, struct

def get_one_spectrum(raw, FFT_SIZE):
    acc = array([0.0] * FFT_SIZE)
    old = (0.0,) * FFT_SIZE;
    window = get_window('hanning', FFT_SIZE)

    def dofft(ar):
        return abs(fft(array(ar) * window)) ** 2

    num = 0
    while True:
        # read FFT_SIZE frames of the s16 2ch output
        bytes = raw.read(FFT_SIZE * 4)

        # we could 0-pad the last frame, but fuck it
        if len(bytes) != 4 * FFT_SIZE:
            break

        # read the other channel
        new = struct.unpack(("<%dh" % (FFT_SIZE*2)), bytes)[::2]
        # window with half of the previous frame
        acc += dofft(old[FFT_SIZE/2 : FFT_SIZE] + new[0 : FFT_SIZE/2])
        # and now alone
        acc += dofft(new)
        old = new
        num += 2

    return acc / num

def pow2db(x):
    if x == 0:
        return -999
    return math.log(x) / math.log(10) * 10

def main():
    channel = None
    if len(sys.argv) != 4:
        raise RuntimeError, "usage: spectral-content.py <rawfile> <fft size> <freq>"

    fft = get_one_spectrum(file(sys.argv[1]), FFT_SIZE=int(sys.argv[2]))
    freq = int(sys.argv[3])

    for i in range(len(fft) / 2 + 1):
        print "%f %f" % (float(i) / len(fft) * freq, pow2db(fft[i]))

if __name__ == '__main__':
    main()
