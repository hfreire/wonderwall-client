set(CMAKE_C_COMPILER armv7-rpi2-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER armv7-rpi2-linux-gnueabihf-g++)
set(CMAKE_ASM_COMPILER armv7-rpi2-linux-gnueabihf-gcc)

add_definitions("-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -mlittle-endian -munaligned-access -marm")

include("${CMAKE_CURRENT_LIST_DIR}/arm-linux-gnueabihf.cmake")