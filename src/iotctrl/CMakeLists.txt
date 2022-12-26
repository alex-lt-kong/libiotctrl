set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_library(MODBUS_LIB modbus)
if(NOT MODBUS_LIB)
    message(FATAL_ERROR "modbus library not found, temp-sensor won't be compiled. Get libmodbus from: https://github.com/stephane/libmodbus")
endif()

add_library(temp-sensor temp-sensor.c)
add_library(relay relay.c)
add_library(iotctrl SHARED relay temp-sensor)

target_link_libraries(iotctrl modbus)

set_target_properties(iotctrl PROPERTIES PUBLIC_HEADER "temp-sensor.h;relay.h;")

install(TARGETS iotctrl 
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/iotctrl
)

add_custom_command(
    TARGET iotctrl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:iotctrl>  ${CMAKE_SOURCE_DIR}/src/bindings/python
)
