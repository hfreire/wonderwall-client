#
# This CMake file tries to find the the RabbitMQ library
#
# The following variables are set:
#   RABBITMQ_FOUND - System has RabbitMQ client
#   RABBITMQ_LIBRARIES - The RabbitMQ client library
#   RABBITMQ_HEADERS - The RabbitMQ client headers

include(CheckCSourceCompiles)

find_library(RABBITMQ_LIBRARIES NAMES rabbitmq PATH_SUFFIXES arm-linux-gnueabihf)
find_path(RABBITMQ_HEADERS amqp.h)

if(${RABBITMQ_LIBRARIES} MATCHES "NOTFOUND")
    set(RABBITMQ_FOUND FALSE CACHE INTERNAL "")
    message(STATUS "RabbitMQ library not found.")
    unset(RABBITMQ_LIBRARIES)
else()
    set(RABBITMQ_FOUND TRUE CACHE INTERNAL "")
    message(STATUS "Found RabbitMQ library: ${RABBITMQ_LIBRARIES}")
endif()

set(CMAKE_REQUIRED_INCLUDES ${RABBITMQ_HEADERS})
