include_directories (${PROJECT_SOURCE_DIR}/src/) 

add_executable(temp-sensor-tool temp-sensor-tool.c)
target_link_libraries(temp-sensor-tool iotctrl modbus)

install(TARGETS temp-sensor-tool 
        LIBRARY DESTINATION bin
)


add_executable(relay-tool relay-tool.c)
target_link_libraries(relay-tool iotctrl modbus)

install(TARGETS relay-tool 
        LIBRARY DESTINATION bin
)


