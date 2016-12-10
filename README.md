## Features
* Blazing fast :dizzy: C code :neckbeard::astonished: with low memory footprint :white_check_mark:

## Dependencies
* librabbitmq1
* libcurl3
* libjansson4
* fib

### How to build (cross-compile)
* `Note: Need to install raspbian sysroot in order to cross-compile.`
```
mkdir -p build && \
cd build && \
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/platforms/raspberrypi/toolchain/arm-linux-gnueabihf.cmake .. && \
make
```