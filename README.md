# wsprcap

This is a program that captures IQ samples from my FPGA based HF transciever project.

It filters and downsamples the signal in two minute chunks so that I can analyze it for WSPR signals.

https://en.wikipedia.org/wiki/WSPR_(amateur_radio_software)


## dependencies

clang with blocks and libblocksruntime.

* https://github.com/nickhutchinson/libdispatch
* http://alsa-project.org/
* http://liquidsdr.org/
* http://www.fftw.org/

## acknowledgements

This would of course not be possible without K1JT Joe Taylor and K9AN Steven Franke. They did all of the hard work.

* https://physics.princeton.edu/pulsar/k1jt/wsjtx.html
* https://github.com/Guenael/rtlsdr-wsprd


## building and using

```
# On Raspberry Pi build with meson.

CC="clang" CFLAGS="-fblocks -mfpu=neon-vfpv4 -mfloat-abi=hard" meson build
```

```
# ~/.asoundrc

pcm.vfzsdr {
    type hw
    card vfzfpga
    device 0
    rate 89286
    format S32_LE
    channels 2
}

pcm.vfzsdrf {
    type lfloat
    slave {
        pcm vfzsdr
        format S32_LE
    }
}
```
