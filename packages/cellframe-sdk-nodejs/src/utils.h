#pragma once

#include <node_api.h>

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENV64
#else
#define ENV32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENV64
#else
#define ENV32
#endif
#endif


#ifdef ENV64
  #ifdef _WIN64
    #define napi_create_int napi_create_int64
    #define napi_get_value_int napi_get_value_int64
  #else
    #define napi_create_int napi_create_int32
    #define napi_get_value_int napi_get_value_int32
  #endif
#else
  #define napi_create_int napi_create_int32
  #define napi_get_value_int napi_get_value_int32
#endif



#define DECLARE_NAPI_METHOD(name, func) \
  { name, 0, func, 0, 0, 0, napi_default, 0 }
#define DECLARE_NAPI_JS_VALUE(name, js_value) \
  { name, 0, 0, 0, 0, js_value, napi_default, 0 }
#define DECLARE_NAPI_PROPERTY(name, getter, setter) \
  {name, 0, 0, getter, setter, 0, napi_default, 0}


/*!
    Checks arguments count.

    Expects declared variables: env, info
    Declares variables: argc, args, jsthis
*/
#define ARG_COUNT_CHECK_UNIQUE(__expected_count_const) \
  size_t argc = (__expected_count_const); \
  napi_value args[(__expected_count_const)]; \
  napi_value jsthis; \
  status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr); \
  assert(status == napi_ok); \
  if (argc != (__expected_count_const)) { \
    napi_throw_type_error(env, nullptr, "Wrong number of arguments"); \
    return nullptr; \
  }

/*!
    Checks argument type.

    Expects declared variables: env, args, arg_type and status;

    TODO: better error string based on argument's number and type;
*/
#define ARG_TYPE_CHECK(__arg_num, __arg_type) \
  status = napi_typeof(env, args[(__arg_num)], &arg_type); \
  assert(status == napi_ok); \
  if (arg_type != (__arg_type)) { \
    napi_throw_type_error(env, nullptr, "Wrong argument type"); \
    return nullptr; \
  }

#define CHECK(x) status = (x); assert(status == napi_ok);

/*
    WARNING: Type check is omitted!
*/
char* extract_str(napi_env env, napi_value js_value, size_t* size = nullptr);

/*
    WARNING: Can raise js exception
*/
int extract_array_buffer(napi_env env, napi_value js_value, void** buffer, size_t* size);

napi_status napi_get_value_size(napi_env env, napi_value js_value, size_t* result);
napi_status napi_create_size(napi_env env, size_t value, napi_value* result);


template<class T>
bool InstanceOf(napi_env env, napi_value object)
{
    napi_status status;
    napi_value cons;
    CHECK(napi_get_reference_value(env, T::constructor, &cons));

    bool result = false;
    CHECK(napi_instanceof(env, object, cons, &result));

    return result;
}
