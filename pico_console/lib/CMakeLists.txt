message(STATUS "building libs...")

add_subdirectory(LED_STATUS)
add_subdirectory(MCP4725)
add_subdirectory(PCA9554)
add_subdirectory(TM022HDH26)
add_subdirectory(SD_SPI)
add_subdirectory(IR_REMOTE)
add_subdirectory(LI_BATTERY)
add_subdirectory(USB_SW)
add_subdirectory(PICO_W_BT)
add_subdirectory(PICO_W_WIFI)

# 헤더파일을 추가하는 곳을 지정. #include "Adder/adder.h"라고 안적을 수 있다.
target_include_directories(${PROJECT_NAME}
  PRIVATE
    LED_STATUS
    MCP4725
    PCA9554
    TM022HDH26
    LI_BATTERY
    SD_SPI
    IR_REMOTE
    USB_SW
    PICO_W_BT
    PICO_W_WIFI
)

# 링크할 라이브러리를 찾는곳. 디렉토리를 빌드하기 위해 경로를 지정한다. 이때 경로는 build된 파일이 있는 경로. 즉, out/build 기준이다.
target_link_directories(${PROJECT_NAME}
  PRIVATE
    LED_STATUS
    MCP4725
    PCA9554
    TM022HDH26
    LI_BATTERY
    SD_SPI
    IR_REMOTE
    USB_SW
    PICO_W_BT
    PICO_W_WIFI
)

# exe파일에 링크할 라이브러리
target_link_libraries(${PROJECT_NAME}
  LED_STATUS
  MCP4725
  PCA9554
  TM022HDH26
  LI_BATTERY
  SD_SPI
  IR_REMOTE
  USB_SW
  PICO_W_BT
  PICO_W_WIFI
)

message(STATUS "building libs completed!")
