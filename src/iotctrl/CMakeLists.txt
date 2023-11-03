set(CMAKE_POSITION_INDEPENDENT_CODE ON)


find_library(MODBUS_LIB modbus)
find_library(GPIOD_LIB gpiod)


add_library(temp-sensor temp-sensor.c)
add_library(relay relay.c)
add_library(beep beep.c)
add_library(iotctrl SHARED relay temp-sensor beep)

target_link_libraries(iotctrl modbus gpiod)

set_target_properties(
    iotctrl PROPERTIES PUBLIC_HEADER "temp-sensor.h;relay.h;beep.h"
)

install(TARGETS iotctrl 
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/iotctrl
)

add_custom_command(
    TARGET iotctrl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:iotctrl>  ${CMAKE_SOURCE_DIR}/src/bindings/python
)
