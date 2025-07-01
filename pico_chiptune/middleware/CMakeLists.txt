message(STATUS "building middlewares...")

add_subdirectory(sound_system)
add_subdirectory(gfx)

add_library(middleware INTERFACE)

target_link_libraries(middleware INTERFACE
  driver
  sound_system
  gfx
)

message(STATUS "building middlewares completed!")
