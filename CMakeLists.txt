cmake_minimum_required(VERSION 3.4)
project(wonderwalld)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

set(SOURCE_FILES src/wonderwalld.c)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

find_package(RabbitMQ REQUIRED)
if (RABBITMQ_FOUND)
    include_directories(${PROJECT_NAME} ${RABBITMQ_HEADERS})
    target_link_libraries(${PROJECT_NAME} ${RABBITMQ_LIBRARIES})
endif (RABBITMQ_FOUND)

find_package(Jansson REQUIRED)
if (JANSSON_FOUND)
    include_directories(${PROJECT_NAME} ${JANSSON_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${JANSSON_LIBRARIES})
endif (JANSSON_FOUND)

find_package(CURL REQUIRED)
if (CURL_FOUND)
    include_directories(${PROJECT_NAME} ${CURL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
endif (CURL_FOUND)

find_package(SDL REQUIRED)
if (SDL_FOUND)
    include_directories(${PROJECT_NAME} ${SDL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL_LIBRARY})
endif (SDL_FOUND)

find_package(SDL_image REQUIRED)
if (SDLIMAGE_FOUND)
    include_directories(${PROJECT_NAME} ${SDLIMAGE_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDLIMAGE_LIBRARY})
endif (SDLIMAGE_FOUND)