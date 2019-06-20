#include "fork.h"
#include "throwerror.h"
#include <unistd.h>
#include <string.h>

// I know what you're thinking. "Why does this module need fork()?"
// You'd be correct in your thinking. This module has absolutely zero need for
// fork(). But I implemented it anyways.
Napi::Value Fork::Fork(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    int pid = fork();
    if (pid == -1) return throwError(env, "fork: " + (std::string)strerror(errno));
    return Napi::Number::New(env, pid);
}
