message(STATUS "building middlewears...")

add_subdirectory(USB_SW)
add_subdirectory(PICO_W_BT)
add_subdirectory(PICO_W_WIFI)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    USB_SW
    PICO_W_BT
    PICO_W_WIFI
)

target_link_directories(${PROJECT_NAME}
  PRIVATE
    USB_SW
    PICO_W_BT
    PICO_W_WIFI
)

target_link_libraries(${PROJECT_NAME}
  USB_SW
  PICO_W_BT
  PICO_W_WIFI
)

message(STATUS "building middlewears completed!")
