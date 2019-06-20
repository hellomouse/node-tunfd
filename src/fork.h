#ifndef _FORK_H
#define _FORK_H

#include <napi.h>

namespace Fork {
    Napi::Value Fork(const Napi::CallbackInfo &info);
}

#endif
