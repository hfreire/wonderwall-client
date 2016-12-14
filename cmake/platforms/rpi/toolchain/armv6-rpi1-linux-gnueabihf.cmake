set(CMAKE_C_COMPILER armv6-rpi1-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER armv6-rpi1-linux-gnueabihf-g++)
set(CMAKE_ASM_COMPILER armv6-rpi1-linux-gnueabihf-gcc)

add_definitions("-mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(SYSROOT "${PROJECT_SOURCE_DIR}/platforms/rpi/rootfs")

set(CMAKE_SYSTEM_PREFIX_PATH "${SYSROOT}/lib/arm-linux-gnueabihf;${SYSROOT}/usr/lib/arm-linux-gnueabihf")

set(CMAKE_FIND_ROOT_PATH "${SYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(FLAGS "-isystem ${SYSROOT}/usr/include/arm-linux-gnueabihf -Wl,-rpath-link,${SYSROOT}/opt/vc/lib -Wl,-rpath-link,${SYSROOT}/lib/arm-linux-gnueabihf -Wl,-rpath-link,${SYSROOT}/usr/lib/arm-linux-gnueabihf -Wl,-rpath-link,${SYSROOT}/usr/local/lib -Wl,-rpath-link,${SYSROOT}/usr/lib/arm-linux-gnueabihf/pulseaudio")

unset(CMAKE_C_FLAGS CACHE)
unset(CMAKE_CXX_FLAGS CACHE)

set(CMAKE_CXX_FLAGS ${FLAGS} CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS ${FLAGS} CACHE STRING "" FORCE)

# rdynamic means the backtrace should work
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-rdynamic)
endif ()

# avoids annoying and pointless warnings from gcc
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -c")
