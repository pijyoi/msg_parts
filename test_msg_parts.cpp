#include <iostream>

#include <zmq.h>
#include <assert.h>

#include "msg_parts.hpp"

int msg_parts_selftest()
{
    printf (" * msg_parts: ");

    int rc = 0;
    //  @selftest
    void *ctx = zmq_ctx_new();
    assert (ctx);

    void *output = zmq_socket (ctx, ZMQ_PAIR);
    assert (output);
    zmq_bind (output, "inproc://zmsg.test");
    void *input = zmq_socket (ctx, ZMQ_PAIR);
    assert (input);
    zmq_connect (input, "inproc://zmsg.test");

    {
        msg_single_t frame("Hello", 5);     // zframe_new
        frame.send(output);         // zframe_send

        frame.recv(input);          // zframe_recv
        std::string str = frame.str();      // zframe_strdup

        std::cout << str << std::endl;
    }

    {
        msg_multi_t msg;

        msg_single_t frame("Frame3", 6);
        msg.parts.push_back(std::move(frame));      // zmsg_add
        msg.parts.emplace_back("Frame4", 6);    // zmsg_addmem
        msg.parts.emplace_back("Frame5");       // zmsg_addstr

        frame.reset("Frame2", 6);
        msg.parts.push_front(std::move(frame));     // zmsg_push
        msg.parts.emplace_front("Frame1", 6);   // zmsg_pushmem
        msg.parts.emplace_front("Frame0");      // zmsg_pushstr

        msg.parts.push_back(std::string("Frame6"));

        rc = msg.send(output);          // zmsg_send
        assert (rc == 0);

        rc = msg.recv(input);           // zmsg_recv
        assert (rc == 0);
        assert (msg.parts.size() == 7);

        for (auto&& part: msg.parts) {
            auto str = part.str_view();
            std::cout << str << std::endl;
        }
    }

    zmq_close(input);
    zmq_close(output);
    zmq_ctx_destroy (ctx);
    //  @end
    printf ("OK\n");
    return 0;
}

int main()
{
    msg_parts_selftest();
}
