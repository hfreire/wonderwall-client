cmake_minimum_required(VERSION 3.4)
project(wonderwalld)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/wonderwalld.c
        src/amqp.c src/amqp.h
        src/download.c src/download.h
        src/display.c src/display.h)
add_executable(wonderwalld ${SOURCE_FILES})

find_package(RABBITMQ REQUIRED)
if (RABBITMQ_FOUND)
    include_directories(${PROJECT_NAME} ${RABBITMQ_HEADERS})
    target_link_libraries(${PROJECT_NAME} ${RABBITMQ_LIBRARIES})
endif ()

find_package(JANSSON REQUIRED)
if (JANSSON_FOUND)
    include_directories(${PROJECT_NAME} ${JANSSON_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${JANSSON_LIBRARIES})
endif ()

find_package(CURL REQUIRED)
if (CURL_FOUND)
    include_directories(${PROJECT_NAME} ${CURL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
endif ()
