#include <czmq.h>

#include "msg_parts.hpp"

int msg_parts_selftest()
{
    printf (" * msg_parts: ");

    int rc = 0;
    //  @selftest
    zctx_t *ctx = zctx_new ();
    assert (ctx);

    void *output = zsocket_new (ctx, ZMQ_PAIR);
    assert (output);
    zsocket_bind (output, "inproc://zmsg.test");
    void *input = zsocket_new (ctx, ZMQ_PAIR);
    assert (input);
    zsocket_connect (input, "inproc://zmsg.test");

    msg_multi_t msg;
    
    //  Test send and receive of single-frame message
    msg.reset();
    msg.parts.push_back(new msg_single_t("Hello", 5));
    rc = msg.send(output);
    assert (rc == 0);

    rc = msg.recv(input);
    assert (rc == 0);
    std::string str = msg.parts.front()->as_string();
    printf("%s\n", str.c_str());

    //  Test send and receive of multi-frame message
    msg.reset();
    msg.parts.push_back(new msg_single_t("Frame0", 6));
    msg.parts.push_back(new msg_single_t("Frame1", 6));
    msg.parts.push_back(new msg_single_t("Frame2", 6));
    msg.parts.push_back(new msg_single_t("Frame3", 6));
    msg.parts.push_back(new msg_single_t("Frame4", 6));
    msg.parts.push_back(new msg_single_t("Frame5", 6));
    msg.parts.push_back(new msg_single_t("Frame6", 6));
    msg.parts.push_back(new msg_single_t("Frame7", 6));
    msg.parts.push_back(new msg_single_t("Frame8", 6));
    msg.parts.push_back(new msg_single_t("Frame9", 6));
    rc = msg.send(output);
    assert (rc == 0);

    rc = msg.recv(input);
    assert (rc == 0);
    assert (msg.parts.size() == 10);

    for (auto it=msg.parts.begin(); it!=msg.parts.end(); ++it) {
        std::string str = (*it)->as_string();
        printf("%s\n", str.c_str());
    }


    zctx_destroy (&ctx);
    //  @end
    printf ("OK\n");
    return 0;
}

int main()
{
	msg_parts_selftest();
}
