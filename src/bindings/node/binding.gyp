{
  "targets": [
    {
      "target_name": "temp_sensor",
      "sources": [ "temp_sensor_node.c" ],
      "libraries":  [ "-liotctrl", "-lmodbus" ]
    },{
         "target_name": "copy_binary",
         "type":"none",
         "dependencies" : [ "temp_sensor" ],
         "copies":
         [
            {
               'destination': '<(module_root_dir)/',
               'files': ['<(module_root_dir)/build/Release/temp_sensor.node']
            }
         ]
      }
  ]
}