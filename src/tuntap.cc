#include "tuntap.h"
#include "throwerror.h"
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

Napi::FunctionReference TunInterface::constructor;

void TunInterface::ReadOnlyProperty(const Napi::CallbackInfo &info, const Napi::Value &value) {
    Napi::Env env = info.Env();
    throwTypeError(env, "Property is read-only");
    return;
}

Napi::Function TunInterface::Init(Napi::Env env) {
    Napi::Function func = DefineClass(env, "TunInterface", {
        InstanceMethod("setPersist", &TunInterface::SetPersist),
        InstanceAccessor("name", &TunInterface::GetName, &TunInterface::ReadOnlyProperty),
        InstanceAccessor("fd", &TunInterface::GetFd, &TunInterface::ReadOnlyProperty)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    return func;
}

TunInterface::TunInterface(const Napi::CallbackInfo &info) : Napi::ObjectWrap<TunInterface>(info) {
    Napi::Env env = info.Env();
    Napi::Object options;
    if (info.Length() < 1) options = Napi::Object::New(env); 
    else {
        if (!info[0].IsObject()) {
            throwTypeError(env, "Argument should be an object");
            return;
        }
        options = info[0].As<Napi::Object>();
    }
    ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    // options.name (string, optional) name of interface
    // default (none) in which kernel allocates next available device
    if (options.Has("name")) {
        if (!options.Get("name").IsString()) {
            throwTypeError(env, "options.name should be a string");
            return;
        }
        std::string name = options.Get("name").As<Napi::String>();
        strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ);
    }
    // options.mode (string, optional) specifies type, default "tun"
    // can be either "tun" or "tap"
    if (!options.Has("mode")) ifr.ifr_flags |= IFF_TUN;
    else {
        if (!options.Get("mode").IsString()) {
            throwTypeError(env, "options.mode should be a string");
            return;
        }
        std::string mode = options.Get("mode").As<Napi::String>();
        if (mode == "tun") ifr.ifr_flags |= IFF_TUN;
        else if (mode == "tap") ifr.ifr_flags |= IFF_TAP;
        else {
            throwTypeError(env, "options.mode must be either 'tun' or 'tap'");
            return;
        }
    }
    // options.pi (boolean, optional) specifies whether or not the 4-byte protocol
    // information header should be prepended to raw packets by the kernel
    // default false
    if (!options.Has("pi")) ifr.ifr_flags |= IFF_NO_PI;
    else {
        if (!options.Get("pi").IsBoolean()) {
            throwTypeError(env, "options.pi should be a boolean");
            return;
        }
        if (!options.Get("pi").As<Napi::Boolean>()) ifr.ifr_flags |= IFF_NO_PI;
    }
    fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) {
        throwError(env, "open /dev/net/tun: " + (std::string)strerror(errno));
        return;
    }
    if (ioctl(fd, TUNSETIFF, &ifr) < 0) {
        throwError(env, "ioctl TUNSETIFF: " + (std::string)strerror(errno));
        return;
    }
    name = std::string(ifr.ifr_name);
}

Napi::Value TunInterface::GetName(const Napi::CallbackInfo &info) {
    return Napi::String::New(info.Env(), name);
}

Napi::Value TunInterface::GetFd(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), fd);
}

Napi::Value TunInterface::SetPersist(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1) return throwTypeError(env, "Needs 1 argument");
    if (!info[0].IsBoolean()) return throwTypeError(env, "Argument should be a boolean");
    int result = ioctl(fd, TUNSETPERSIST, (uintptr_t)(bool)info[0].As<Napi::Boolean>());
    if (result < 0) return throwError(env, "ioctl TUNSETPERSIST: " + (std::string)strerror(errno));
    return info[0];
}

TunInterface::~TunInterface() {
    // according to the linux programmer's manual, errors should be able to be
    // silently ignored in this case
    close(fd);
}
