#include <assert.h>
#include "utils.h"

/*
    WARNING: Type check is omitted!
*/
char* extract_str(napi_env env, napi_value js_value)
{
    size_t str_size;
    // Returns string size without NULL
    napi_status status = napi_get_value_string_utf8(env, js_value, nullptr, 0, &str_size);
    assert(status == napi_ok);

    char* buffer = new char[str_size + 1];
    // Writes string + NULL
    status = napi_get_value_string_utf8(env, js_value, buffer, str_size + 1, &str_size);
    assert(status == napi_ok);

    return buffer;
}

