#ifndef _RTNETLINK_H
#define _RTNETLINK_H

#include <napi.h>
#include <linux/netlink.h>

class RTNetlink : public Napi::ObjectWrap<RTNetlink> {
public:
    static Napi::Function Init(Napi::Env env);
    RTNetlink(const Napi::CallbackInfo &info);
    ~RTNetlink();

private:
    static Napi::FunctionReference constructor;
    int fd;
    sockaddr_nl local_addr;
    sockaddr_nl kernel_addr;

    void ReadOnlyProperty(const Napi::CallbackInfo &info, const Napi::Value &value);
    Napi::Value DoThings(const Napi::CallbackInfo &info);
    Napi::Value GetFd(const Napi::CallbackInfo &info);
};

#endif
