set(CMAKE_C_COMPILER armv8-rpi3-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER armv8-rpi3-linux-gnueabihf-g++)
set(CMAKE_ASM_COMPILER armv8-rpi3-linux-gnueabihf-gcc)

add_definitions("-mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm")

include("${CMAKE_CURRENT_LIST_DIR}/arm-linux-gnueabihf.cmake")