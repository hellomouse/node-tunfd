#include "rtnetlink.h"
#include "throwerror.h"
#include <napi.h>
#include <asm/types.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <unistd.h>

// very todo

#define RTNL_REPLY_LEN 8192

struct nl_req {
    nlmsghdr header;
    rtgenmsg gen;
};

Napi::FunctionReference RTNetlink::constructor;

void RTNetlink::ReadOnlyProperty(const Napi::CallbackInfo &info, const Napi::Value &value) {
    Napi::Env env = info.Env();
    throwTypeError(env, "Property is read-only");
    return;
}

Napi::Function RTNetlink::Init(Napi::Env env) {
    Napi::Function func = DefineClass(env, "RTNetlink", {
        InstanceAccessor("fd", &RTNetlink::GetFd, &RTNetlink::ReadOnlyProperty)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    return func;
}

RTNetlink::RTNetlink(const Napi::CallbackInfo &info) : Napi::ObjectWrap<RTNetlink>(info) {
    Napi::Env env = info.Env();
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        throwError(env, "socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE) " +
            (std::string)strerror(errno));
        return;
    }
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.nl_family = AF_NETLINK;
    local_addr.nl_pad = 0;
    local_addr.nl_pid = getpid();
    local_addr.nl_groups = 0;
    memset(&kernel_addr, 0, sizeof(kernel_addr));
    kernel_addr.nl_family = AF_NETLINK;
    kernel_addr.nl_groups = 0;
    if (bind(fd, (sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        throwError(env, "bind netlink socket: " + (std::string)strerror(errno));
        return;
    }
}

Napi::Value RTNetlink::DoThings(const Napi::CallbackInfo &info) {
    msghdr rtnl_msg;
    iovec iov;
    nl_req req;
    memset(&rtnl_msg, 0, sizeof(rtnl_msg));
    memset(&req, 0, sizeof(req));
    req.header.nlmsg_len = NLMSG_LENGTH(sizeof(rtgenmsg));
    req.header.nlmsg_type = RTM_GETROUTE;
    req.header.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.header.nlmsg_seq = 1;
    req.header.nlmsg_pid = local_addr.nl_pid;
    req.gen.rtgen_family = AF_PACKET;
    iov.iov_base = &req;
    iov.iov_len = req.header.nlmsg_len;
    rtnl_msg.msg_iov = &iov;
    rtnl_msg.msg_iovlen = 1;
    rtnl_msg.msg_name = &kernel_addr;
    rtnl_msg.msg_namelen = sizeof(kernel_addr);
    sendmsg(fd, &rtnl_msg, 0);
    bool end = false;
    while (!end) {
        int len;
        msghdr rtnl_reply;
        iovec iov_reply;
        memset(&rtnl_reply, 0, sizeof(rtnl_reply));
        memset(&iov_reply, 0, sizeof(iov_reply));
        char reply[RTNL_REPLY_LEN];
        iov_reply.iov_base = reply;
        iov_reply.iov_len = RTNL_REPLY_LEN;
        rtnl_reply.msg_iov = &iov_reply;
        rtnl_reply.msg_iovlen = 1;
        rtnl_reply.msg_name = &kernel_addr;
        rtnl_reply.msg_namelen = sizeof(kernel_addr);

        len = recvmsg(fd, &rtnl_reply, 0);
        if (!len) continue;
        for (nlmsghdr *msg_ptr = (nlmsghdr *)reply; NLMSG_OK(msg_ptr, len); msg_ptr = NLMSG_NEXT(msg_ptr, len)) {
            switch (msg_ptr->nlmsg_type) {
                case NLMSG_ERROR:
                    end = true;
                    nlmsgerr *error = (nlmsgerr *)NLMSG_DATA(msg_ptr);
                    break;
                case NLMSG_DONE:
                    end = true;
                    break;
                case 16:
                    break;
                case 24:
                    break;
            }
        }
    }
}

Napi::Value RTNetlink::GetFd(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), fd);
}

RTNetlink::~RTNetlink() {
    // ignore errors (errors are not applicable)
    close(fd);
}
