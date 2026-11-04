# --- settings before `project()` ---

# minimum supported CMake version
cmake_minimum_required(VERSION 3.13)

# generate compile_commands.json for clangd / Neovim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# import the Pico SDK before configuring the compiler
include(pico_sdk_import.cmake)

# --- project configuration and SDK setup ---

# define project name, version, and enabled languages
project(rp2040_trigger_controller
  VERSION 0.1.0
  LANGUAGES C CXX ASM
)

# specify language standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# require an exact Pico SDK version string
set(REQUIRED_PICO_SDK_VERSION "2.3.1")

if(NOT PICO_SDK_VERSION_STRING STREQUAL REQUIRED_PICO_SDK_VERSION)
  message(FATAL_ERROR
    "Pico SDK version mismatch.\n"
    "Expected: ${REQUIRED_PICO_SDK_VERSION}\n"
    "Actual: ${PICO_SDK_VERSION_STRING}"
  )
endif()

# initialize the Pico SDK
pico_sdk_init()

# --- application target settings ---

# define the application executable and its source files
add_executable(${PROJECT_NAME} src/main.c)

# link required pico sdk libraries
target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib)

# configure stdio backends for printf()
pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 0)

# embed application identity and version metadata
pico_set_program_name(${PROJECT_NAME} "${PROJECT_NAME}")
pico_set_program_version(${PROJECT_NAME} "${PROJECT_VERSION}")

# generate additional firmware output formats
pico_add_extra_outputs(${PROJECT_NAME})
