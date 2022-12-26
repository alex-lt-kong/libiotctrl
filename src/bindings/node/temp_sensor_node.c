#include <assert.h>
#include <node_api.h>
#include <stdio.h>
#include <iotctrl/temp-sensor.h>
#include <string.h>

// Refer to here for more examples: https://github.com/nodejs/node-addon-examples


static napi_value get_temperature_napi(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 2;
  napi_value args[2];
  status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  assert(status == napi_ok);

  if (argc < 2) {
    napi_throw_type_error(env, NULL, "Wrong number of arguments");
    return NULL;
  }

  napi_valuetype valuetype0;
  status = napi_typeof(env, args[0], &valuetype0);
  assert(status == napi_ok);

  napi_valuetype valuetype1;
  status = napi_typeof(env, args[1], &valuetype1);
  assert(status == napi_ok);

  if (valuetype0 != napi_string || valuetype1 != napi_number) {
    napi_throw_type_error(env, NULL, "Wrong arguments");
    return NULL;
  }

  const size_t buf_size = PATH_MAX + 1;
  size_t result;
  char device_path[buf_size];
  status = napi_get_value_string_latin1(env, args[0], device_path, buf_size, &result);
  if (result >= buf_size - 1) {
    fprintf(stderr, "WARNING: device_path will be truncated to %lu characters\n", buf_size - 1);
  }
  assert(status == napi_ok);

  int debug_mode;
  status = napi_get_value_int32(env, args[1], &debug_mode);
  assert(status == napi_ok);

  int temp_raw = get_temperature(device_path, debug_mode);

  napi_value temp_raw_napi;
  status = napi_create_int32(env, temp_raw, &temp_raw_napi);
  assert(status == napi_ok);

  return temp_raw_napi;
}

#define DECLARE_NAPI_METHOD(name, func)                                        \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor get_temperature_descriptor = DECLARE_NAPI_METHOD("get_temperature", get_temperature_napi);
  status = napi_define_properties(env, exports, 1, &get_temperature_descriptor);
  assert(status == napi_ok);
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)