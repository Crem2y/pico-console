message(STATUS "building drivers...")

add_subdirectory(led_status)
add_subdirectory(li_battery)
add_subdirectory(ir_remote)
add_subdirectory(mcp4725)
add_subdirectory(pca9554)
add_subdirectory(tm022hdh26)
add_subdirectory(sd_spi)
add_subdirectory(pico_w_wifi)
add_subdirectory(pico_w_bt)
add_subdirectory(usb_sw)

add_library(driver INTERFACE)

target_link_libraries(driver INTERFACE
  led_status
  li_battery
  ir_remote
  mcp4725
  pca9554
  tm022hdh26
  sd_spi
  pico_w_wifi
  pico_w_bt
  usb_sw
)

message(STATUS "building drivers completed!")
