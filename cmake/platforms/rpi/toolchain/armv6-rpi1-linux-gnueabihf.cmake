set(CMAKE_C_COMPILER armv6-rpi1-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER armv6-rpi1-linux-gnueabihf-g++)
set(CMAKE_ASM_COMPILER armv6-rpi1-linux-gnueabihf-gcc)

add_definitions("-mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm")

include("${CMAKE_CURRENT_LIST_DIR}/arm-linux-gnueabihf.cmake")