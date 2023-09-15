#!/usr/bin/env python3

#      Filename: ftqviz.py
#        Author: Darren Hart <dvhltc@us.ibm.com>
#   Description: Plot the time and frequency domain plots of a times and
#                counts log file pair from the FTQ benchmark.
# Prerequisites: numpy, scipy, and pylab packages.  For debian/ubuntu:
#                o python-numeric
#                o python-scipy
#                o python-matplotlib
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Copyright (C) IBM Corporation, 2007
#
# 2007-Aug-30:  Initial version by Darren Hart <dvhltc@us.ibm.com>


from numpy import *
from numpy.fft import *
from scipy import *
from pylab import *
from sys import *
from getopt import *

NS_PER_S  = 1000000000
NS_PER_MS = 1000000
NS_PER_US = 1000

def smooth(x, wlen):
    if x.size < wlen:
        raise ValueError("Input vector needs to be bigger than window size.")

    # reflect the signal to avoid transients... ?
    s = r_[2*x[0]-x[wlen:1:-1], x, 2*x[-1]-x[-1:-wlen:-1]]
    w = hamming(wlen)

    # generate the smoothed signal
    y = convolve(w/w.sum(), s, mode='same')
    # recenter the smoothed signal over the originals (slide along x)
    y1 = y[wlen-1:-wlen+1]
    return y1


def my_fft(x, sample_hz):
    X = abs(fftshift(fft(x)))
    freq = fftshift(fftfreq(len(x), 1.0/sample_hz))
    return array([freq, abs(X)/len(x)])


def smooth_fft(timefile, countfile, sample_hz, wlen):
    # The higher the sample_hz, the larger the required wlen (used to generate
    # the hamming window).  It seems that each should be adjusted by roughly the
    # same factor
    ns_per_sample = NS_PER_S / sample_hz

    print("Interpolated Sample Rate: ", sample_hz, " HZ")
    print("Hamming Window Length: ", wlen)

    t = fromfile(timefile, dtype=int64, sep='\n')
    x = fromfile(countfile, dtype=int64, sep='\n')

    # interpolate the data to achieve a uniform sample rate for use in the fft
    xi_len = (t[len(t)-1] - t[0])/ns_per_sample
    xi = zeros(xi_len)
    last_j = 0
    for i in range(0, len(t)-1):
        j = (t[i] - t[0])/ns_per_sample
        xi[j] = x[i]
        m = (xi[j]-xi[last_j])/(j-last_j)
        for k in range(last_j + 1, j):
            xi[k] = m * (k - last_j) + xi[last_j]
        last_j = j

    # smooth the signal (low pass filter)
    try:
        y = smooth(xi, wlen)
    except ValueError as e:
        exit(e)

    # generate the fft
    X = my_fft(xi, sample_hz)
    Y = my_fft(y, sample_hz)

    # plot the hamming window
    subplot(311)
    plot(hamming(wlen))
    axis([0,wlen-1,0,1.1])
    title(str(wlen)+" Point Hamming Window")

    # plot the signals
    subplot(312)
    ts = arange(0, len(xi), dtype=float)/sample_hz # time signal in units of seconds
    plot(ts, xi, alpha=0.2)
    plot(ts, y)
    legend(['interpolated', 'smoothed'])
    title("Counts (interpolated sample rate: "+str(sample_hz)+" HZ)")
    xlabel("Time (s)")
    ylabel("Units of Work")

    # plot the fft
    subplot(313)
    plot(X[0], X[1], ls='steps', alpha=0.2)
    plot(Y[0], Y[1], ls='steps')
    ylim(ymax=20)
    xlim(xmin=-3000, xmax=3000)
    legend(['interpolated', 'smoothed'])
    title("FFT")
    xlabel("Frequency")
    ylabel("Amplitude")

    show()


def usage():
        print("usage: "+argv[0]+" -t times-file -c counts-file [-s SAMPLING_HZ] [-w WINDOW_LEN] [-h]")


if __name__=='__main__':

    try:
        opts, args = getopt(argv[1:], "c:hs:t:w:")
    except GetoptError:
        usage()
        exit(2)

    sample_hz = 10000
    wlen = 25
    times_file = None
    counts_file = None
    for o, a in opts:
        if o == "-c":
            counts_file = a
        if o == "-h":
            usage()
            exit()
        if o == "-s":
            sample_hz = int(a)
        if o == "-t":
            times_file = a
        if o == "-w":
            wlen = int(a)

    if not times_file or not counts_file:
        usage()
        exit(1)

    smooth_fft(times_file, counts_file, sample_hz, wlen)
