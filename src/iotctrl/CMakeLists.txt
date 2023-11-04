set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

find_library(MODBUS_LIB modbus)
find_library(GPIOD_LIB gpiod)


#add_library(relay relay.c)
add_library(iotctrl 7segment-display.c buzzer.c temp-sensor.c relay.c)
#target_link_libraries(iotctrl SHARED relay temp-sensor buzzer 7segment-display)


target_link_libraries(iotctrl modbus gpiod)

set_target_properties(
    iotctrl PROPERTIES PUBLIC_HEADER "temp-sensor.h;relay.h;buzzer.h;7segment-display.h"
)

install(TARGETS iotctrl 
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/iotctrl
)

add_custom_command(
    TARGET iotctrl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:iotctrl>  ${CMAKE_SOURCE_DIR}/src/bindings/python
)
