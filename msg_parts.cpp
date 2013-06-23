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
	msg_single_t frame("Hello", 5);		// zframe_new
	frame.send(output);			// zframe_send

	frame.recv(input);			// zframe_recv
	std::string str(frame.as_string());	// zframe_strdup

	printf("%s\n", str.c_str());
    }

    {
	msg_multi_t msg;

	msg_single_t frame("Frame3", 6);
	msg.parts.push_back(std::move(frame));		// zmsg_add
	msg.parts.push_back(msg_single_t("Frame4", 6));	// zmsg_addmem
	msg.parts.push_back("Frame5");			// zmsg_addstr

	frame.reset("Frame2", 6);
	msg.parts.push_front(std::move(frame));		// zmsg_push
	msg.parts.push_front(msg_single_t("Frame1", 6));// zmsg_pushmem
	msg.parts.push_front("Frame0");			// zmsg_pushstr

	std::string str("Frame6");
	msg.parts.push_back(str);

	rc = msg.send(output);			// zmsg_send
	assert (rc == 0);

    	rc = msg.recv(input);			// zmsg_recv
    	assert (rc == 0);
    	assert (msg.parts.size() == 7);

	for (auto it=msg.parts.begin(); it!=msg.parts.end(); ++it) {
		std::string str = (*it).as_string();
		printf("%s\n", str.c_str());
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
