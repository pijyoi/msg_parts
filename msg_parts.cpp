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
    msg.parts.push_back("Hello");
    rc = msg.send(output);
    assert (rc == 0);

    rc = msg.recv(input);
    assert (rc == 0);
    std::string str = msg.parts.front().as_string();
    printf("%s\n", str.c_str());

    //  Test send and receive of multi-frame message
    msg.reset();
    msg.parts.push_back(msg_single_t("Frame0", 6));
    msg.parts.push_back(msg_single_t("Frame1"));

    // Following doesn't work because msg_single_t has no constructor 
    // that takes a std::string by _value_
    //msg.parts.push_back(std::string("Frame2"));
    // The following uses the msg_single_t constructor that takes a 
    // std::string by _reference_
    std::string str2("Frame2");
    msg.parts.push_back(str2);

    // No need to create a msg_single_t explicitly
    msg.parts.push_back("Frame3");
    msg.parts.push_back("Frame4");
    msg.parts.push_back("Frame5");
    msg.parts.push_back("Frame6");
    msg.parts.push_back("Frame7");
    msg.parts.push_back("Frame8");
    msg.parts.push_back("Frame9");
    rc = msg.send(output);
    assert (rc == 0);

    rc = msg.recv(input);
    assert (rc == 0);
    assert (msg.parts.size() == 10);

    for (auto it=msg.parts.begin(); it!=msg.parts.end(); ++it) {
        std::string str = (*it).as_string();
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
