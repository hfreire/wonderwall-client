## Features
* Blazing fast :dizzy: C code :neckbeard::astonished: with low memory footprint :white_check_mark:
* Launch :rocket: inside a Docker container :whale: so you don't need to manage the dependencies :raised_hands: :white_check_mark:

## Dependencies
* librabbitmq1
* libcurl3
* libjansson4
* libsdl1.2
* libsdl-image1.2

### How to build (cross-compile)
Note: Requires an ARM cross-compiler and a raspbian sysroot.
```
mkdir build && \
cd build && \
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/platforms/rpi/toolchain/armv6-rpi1-linux-gnueabihf.cmake .. && \
make
```
