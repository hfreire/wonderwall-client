#
# This CMake file tries to find the the RabbitMQ library
#
# The following variables are set:
#   RABBITMQ_FOUND - RabbitMQ client library has been found in the system
#   RABBITMQ_LIBRARIES - RabbitMQ client library path
#   RABBITMQ_INCLUDE_DIRS - RabbitMQ client headers path
#

if (RABBITMQ_LIBRARIES AND RABBITMQ_INCLUDE_DIRS)
    # in cache already
    set(RABBITMQ_FOUND TRUE)
else (RABBITMQ_LIBRARIES AND RABBITMQ_INCLUDE_DIRS)
    find_library(RABBITMQ_LIBRARIES
            NAMES
            rabbitmq
            )

    find_path(
            RABBITMQ_INCLUDE_DIRS
            amqp.h
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(RabbitMQ DEFAULT_MSG
            RABBITMQ_LIBRARIES RABBITMQ_INCLUDE_DIRS)

    # show the RABBITQ_INCLUDE_DIRS and RABBITMQ_LIBRARIES variables only in the advanced view
    mark_as_advanced(RABBITMQ_INCLUDE_DIRS RABBITMQ_LIBRARIES)

endif (RABBITMQ_LIBRARIES AND RABBITMQ_INCLUDE_DIRS)
