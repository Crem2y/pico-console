message(STATUS "building middlewares...")

add_subdirectory(sound_system)

add_library(middleware INTERFACE)

target_link_libraries(middleware INTERFACE
  driver
  sound_system
)

message(STATUS "building middlewares completed!")
