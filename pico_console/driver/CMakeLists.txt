message(STATUS "building drivers...")

add_subdirectory(LED_STATUS)
add_subdirectory(MCP4725)
add_subdirectory(PCA9554)
add_subdirectory(TM022HDH26)
add_subdirectory(SD_SPI)
add_subdirectory(IR_REMOTE)
add_subdirectory(LI_BATTERY)
add_subdirectory(PICO_W_WIFI)
add_subdirectory(PICO_W_BT)
add_subdirectory(USB_SW)

add_library(driver INTERFACE)

target_link_libraries(driver INTERFACE
  LED_STATUS
  MCP4725
  PCA9554
  TM022HDH26
  LI_BATTERY
  SD_SPI
  IR_REMOTE
  PICO_W_WIFI
  PICO_W_BT
  USB_SW
)

message(STATUS "building drivers completed!")
