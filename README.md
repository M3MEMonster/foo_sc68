A foobar2000 input decodes `.sc68`, `.sndh`, `.snd` format.

Based on https://sourceforge.net/p/sc68/code/HEAD/tree r600 version (2015-11-28)

(Since I think this is the best version for emul effect through my test)

## Changes I made:

1. Move configuration from **right-click menu** to **Preferences → Playback → Decoding → SC68 input** and fixed the issue that the change in config can't be applied immediately.

2. Added seeking function. Now you can seeking sc68 songs!

3. Expand the project compatibility。

4. Fixed address loading issues for some replayers

5. Merged SNDH Database of r713 to this version 

## Compatibility
- foobar2000 **v2.0 or newer**, Windows 32-bit and 64-bit, MacOS
- Built against the foobar2000 SDK (2026-09-17).

## Building from source

### Prerequisites
- Visual Studio 2022 or above with the **Desktop development with C++** workload (and a Windows 10/11 SDK).
- The official **foobar2000 SDK (2026-09-17 or compatible)**.
- **WTL 10** (Windows Template Library).

### Build steps
Now the project uses CMake to build and manage.

The project files reference the SDK with system environment variable `FB2KSDK`. So, you just need to add the variable with SDK loacation to your system then the project will recognize it. 

Use WTL_ROOT CMake parameter to specify WTL location.

## Third-party code and acknowledgments

This repository includes `getopt-win/getopt.c` and `getopt-win/getopt.h` from [getopt-win](https://github.com/ludvikjerabek/getopt-win). Ludvik Jerabek adapted the Free Software Foundation's GNU getopt implementation for Microsoft Visual C. Copyright in these files remains with their respective copyright holders.

The bundled `getopt-win` code is licensed under the [GNU Lesser General Public License v3.0 (LGPL-3.0)](https://github.com/ludvikjerabek/getopt-win/blob/getopt_glibc_2.42_port/LICENSE). Thanks to Ludvik Jerabek and the GNU getopt contributors for their work.

