#include <napi.h>

inline Napi::Value throwError(Napi::Env env, std::string message) {
    Napi::Error::New(env, message).ThrowAsJavaScriptException();
    return env.Null();
}

inline Napi::Value throwTypeError(Napi::Env env, std::string message) {
    Napi::TypeError::New(env, message).ThrowAsJavaScriptException();
    return env.Null();
}
