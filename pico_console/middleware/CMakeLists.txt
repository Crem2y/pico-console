message(STATUS "building middlewears...")

add_subdirectory(USB_SW)
add_subdirectory(PICO_W_BT)
add_subdirectory(PICO_W_WIFI)

add_library(middleware INTERFACE)

target_link_libraries(middleware INTERFACE
  driver
  USB_SW
  PICO_W_BT
  PICO_W_WIFI
)

message(STATUS "building middlewears completed!")
