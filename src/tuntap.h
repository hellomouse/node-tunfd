#ifndef _TUNTAP_H
#define _TUNTAP_H

#include <napi.h>

class TunInterface : public Napi::ObjectWrap<TunInterface> {
public:
    static Napi::Function Init(Napi::Env env);
    TunInterface(const Napi::CallbackInfo &info);
    ~TunInterface();

private:
    static Napi::FunctionReference constructor;
    std::string name;
    int fd;

    void ReadOnlyProperty(const Napi::CallbackInfo &info, const Napi::Value &value);
    Napi::Value GetName(const Napi::CallbackInfo &info);
    Napi::Value GetFd(const Napi::CallbackInfo &info);
    Napi::Value SetPersist(const Napi::CallbackInfo &info);
};

#endif
