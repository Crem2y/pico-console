# for pico W board
set(PICO_BOARD pico_w)
set(PICO_CYW43_SUPPORTED 1)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_SIZE arm-none-eabi-size)
set(PICO_SDK_PATH ../../pico-sdk)
include(pico_sdk_import.cmake)

get_filename_component(PROJ_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

message(STATUS "building project \"${PROJ_NAME}\"...")

# project([project_name] C CXX ASM)
# output : [project_name].uf2
project(${PROJ_NAME}
  LANGUAGES
    C
    CXX
    ASM
  VERSION
    1.0.0
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(SYS_CLK_MHZ=200) # set clock to 200MHz

pico_sdk_init()

include_directories(
    ./include
)

# add_executable(${PROJECT_NAME} [src1.c] [src2.c] [src3.c] ...)
#add_executable(${PROJECT_NAME} src/main.cpp)
add_executable(${PROJECT_NAME} src/hw_test.cpp)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf
    COMMENT "RAM/Flash Usage : "
)

add_subdirectory(driver)
add_subdirectory(middleware)

pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  pico_stdlib
  pico_multicore
  pico_bootsel_via_double_reset # Double tap reset into bootrom
  pico_cyw43_arch_none # for pico W board
)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

message(STATUS "building project \"${PROJ_NAME}\" completed!")
