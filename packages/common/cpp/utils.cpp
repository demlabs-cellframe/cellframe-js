#include <assert.h>
#include "utils.h"

/*
    WARNING: Type check is omitted!
*/
char* extract_str(napi_env env, napi_value js_value, size_t* size)
{
    size_t str_size;
    // Returns string size without NULL
    napi_status status = napi_get_value_string_utf8(env, js_value, nullptr, 0, &str_size);
    assert(status == napi_ok);

    char* buffer = new char[str_size + 1];
    // Writes string + NULL
    status = napi_get_value_string_utf8(env, js_value, buffer, str_size + 1, &str_size);
    assert(status == napi_ok);

    if (size != nullptr)
    {
        *size = str_size;
    }

    return buffer;
}

/*
    WARNING: Can raise js exception
*/
int extract_array_buffer(napi_env env, napi_value js_value, void** buffer, size_t* size)
{
    napi_status status;
    bool is_array_buffer = false;
    CHECK(napi_is_arraybuffer(env, js_value, &is_array_buffer));
    if (is_array_buffer == false)
    {
        napi_throw_type_error(env, nullptr, "ArrayBuffer expected");
        return -1;
    }

    CHECK(napi_get_arraybuffer_info(env, js_value, buffer, size));

    return 0;
}

napi_status napi_get_value_size(napi_env env, napi_value js_value, size_t* result)
{
#ifdef ENV64
    int64_t _tmp;
    napi_status status = napi_get_value_int64(env, js_value, &_tmp);
    *result = (size_t)_tmp;
    return status;
#else
    return napi_get_value_uint32(env, js_value, result);
#endif
}

napi_status napi_create_size(napi_env env, size_t value, napi_value* result)
{
#ifdef ENV64
    return napi_create_int64(env, (int64_t)value, result);
#else
    return napi_create_uint32(env, value, result);
#endif
}