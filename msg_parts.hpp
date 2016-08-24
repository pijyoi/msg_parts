#include <list>
#include <string>
#include <cstring>

#if __cplusplus >= 201402L
#include <experimental/string_view>
#endif

#include <zmq.h>

class msg_single_t
{
public:
    mutable zmq_msg_t msg;

    msg_single_t() { zmq_msg_init(&msg); }
    explicit msg_single_t(size_t size) { zmq_msg_init_size(&msg, size); }
    msg_single_t(const void* buffer, size_t size)
    {
        zmq_msg_init_size(&msg, size);
        memcpy(data(), buffer, size);
    }
    ~msg_single_t() { zmq_msg_close(&msg); }

    void reset() { zmq_msg_close(&msg); zmq_msg_init(&msg); }
    void reset(size_t size) { zmq_msg_close(&msg); zmq_msg_init_size(&msg, size); }
    void reset(const void* buffer, size_t size)
    {
        zmq_msg_close(&msg);
        zmq_msg_init_size(&msg, size);
        memcpy(data(), buffer, size);
    }

    size_t size() { return zmq_msg_size(&msg); }
    void* data() { return zmq_msg_data(&msg); }
    bool more() { return zmq_msg_more(&msg)!=0; }

    int recv(void *zsock, int flags=0) { return zmq_msg_recv(&msg, zsock, flags); }
    int send(void *zsock, int flags=0) { return zmq_msg_send(&msg, zsock, flags); }

    std::string str() { return std::string(reinterpret_cast<char*>(data()), size()); }

    #if __cplusplus >= 201402L
    std::experimental::string_view
    str_view() { return { static_cast<char*>(data()), size() }; }
    #else
    // fallback to a copy
    std::string
    str_view() { return std::string(static_cast<char*>(data()), size()); }
    #endif

    // string constructors
    msg_single_t(const char* str)
    {
        size_t size = strlen(str);
        zmq_msg_init_size(&msg, size);
        memcpy(data(), str, size);
    }
    msg_single_t(const std::string& str)
    {
        zmq_msg_init_size(&msg, str.size());
        memcpy(data(), str.data(), str.size());
    }
 
    // move constructors
    msg_single_t(msg_single_t&& other)
    {
        zmq_msg_init(&msg);
        zmq_msg_move(&msg, &other.msg);
    }
    msg_single_t& operator=(msg_single_t&& other)
    {
        if (this != &other) zmq_msg_move(&msg, &other.msg);
        return *this;
    }

    // copy constructor
    msg_single_t(const msg_single_t& other)
    {
        zmq_msg_init(&msg);
        zmq_msg_copy(&msg, &other.msg);
    }

    // copy assignment
    msg_single_t& operator=(const msg_single_t& other)
    {
        if (this != &other) zmq_msg_copy(&msg, &other.msg);
        return *this;
    }
};

class msg_multi_t
{
public:
    std::list<msg_single_t> parts;

    msg_multi_t() {}

    int recv(void *zsock, int flags=0)
    {
        parts.clear();
        bool success = true;
        bool more = true;
        while (more) {
            msg_single_t frame;
            int rc = frame.recv(zsock, flags);
            if (rc==-1) {
                success = false;
                break;
            }
            more = frame.more();
            parts.push_back(std::move(frame));
        }
        return success ? 0 : -1;
    }

    int send(void *zsock, int flags=0)
    {
        auto zit = std::begin(parts);
        auto eit = std::end(parts);
        for (; zit!=eit; ++zit)
        {
            auto next = zit;
            std::advance(next, 1);
            bool more = next != eit;

            int rc = zit->send(zsock, flags | (more ? ZMQ_SNDMORE : 0));
            if (rc==-1) {
                break;
            }
        }

        // erase all successfully sent frames
        parts.erase(std::begin(parts), zit);

        bool success = zit==eit;
        return success ? 0 : -1;
    }

private:
    msg_multi_t(const msg_multi_t&);
    msg_multi_t& operator=(const msg_multi_t&);
};

