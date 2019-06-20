#include <napi.h>
#include "tuntap.h"
// #include "rtnetlink.h"
#include "fork.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports["TunInterface"] = TunInterface::Init(env);
    // exports["RTNetlink"] = RTNetlink::Init(env);
    exports["fork"] = Napi::Function::New(env, Fork::Fork);
    return exports;
}

NODE_API_MODULE(addon, Init)
