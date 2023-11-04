include_directories (${PROJECT_SOURCE_DIR}/src/) 

add_executable(temp-sensor-tool temp-sensor-tool.c)
target_link_libraries(temp-sensor-tool iotctrl modbus)
install(TARGETS temp-sensor-tool LIBRARY DESTINATION bin)

add_executable(relay-tool relay-tool.c)
target_link_libraries(relay-tool iotctrl modbus)
install(TARGETS relay-tool LIBRARY DESTINATION bin)

add_executable(buzzer-tool buzzer-tool.c)
target_link_libraries(buzzer-tool iotctrl gpiod)
install(TARGETS buzzer-tool LIBRARY DESTINATION bin)

add_executable(7seg-disp-tool 7seg-disp-tool.c)
target_link_libraries(7seg-disp-tool iotctrl gpiod)
install(TARGETS 7seg-disp-tool LIBRARY DESTINATION bin)


