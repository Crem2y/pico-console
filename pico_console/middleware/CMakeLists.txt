message(STATUS "building middlewares...")

add_library(middleware INTERFACE)

target_link_libraries(middleware INTERFACE
  driver
)

message(STATUS "building middlewares completed!")
