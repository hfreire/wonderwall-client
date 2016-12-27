set(CMAKE_C_COMPILER armv8-rpi3-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER armv8-rpi3-linux-gnueabihf-g++)
set(CMAKE_ASM_COMPILER armv8-rpi3-linux-gnueabihf-gcc)

add_definitions("-mcpu=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard -mlittle-endian -munaligned-access -marm")

include("${CMAKE_CURRENT_LIST_DIR}/arm-linux-gnueabihf.cmake")