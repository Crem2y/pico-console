message(STATUS "building drivers...")

add_subdirectory(LED_STATUS)
add_subdirectory(MCP4725)
add_subdirectory(PCA9554)
add_subdirectory(TM022HDH26)
add_subdirectory(SD_SPI)
add_subdirectory(IR_REMOTE)
add_subdirectory(LI_BATTERY)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    LED_STATUS
    MCP4725
    PCA9554
    TM022HDH26
    LI_BATTERY
    SD_SPI
    IR_REMOTE
)

target_link_directories(${PROJECT_NAME}
  PRIVATE
    LED_STATUS
    MCP4725
    PCA9554
    TM022HDH26
    LI_BATTERY
    SD_SPI
    IR_REMOTE
)

target_link_libraries(${PROJECT_NAME}
  LED_STATUS
  MCP4725
  PCA9554
  TM022HDH26
  LI_BATTERY
  SD_SPI
  IR_REMOTE
)

message(STATUS "building drivers completed!")
