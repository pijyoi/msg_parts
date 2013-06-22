#include <list>
#include <string>
#include <cstring>

#include <zmq.h>

class msg_single_t
{
public:
	zmq_msg_t msg;
	
	msg_single_t() { zmq_msg_init(&msg); }	
	explicit msg_single_t(size_t size) { zmq_msg_init_size(&msg, size); }
	msg_single_t(const void* buffer, size_t size) 
	{ 
		zmq_msg_init_size(&msg, size); 
		memcpy(data(), buffer, size); 
	}
	msg_single_t(const char* str)
	{
		size_t size = strlen(str);
		zmq_msg_init_size(&msg, size);
		memcpy(data(), str, size);
	}
	msg_single_t(std::string& str)
	{
		zmq_msg_init_size(&msg, str.size());
		memcpy(data(), str.data(), str.size());
	}
	~msg_single_t() { zmq_msg_close(&msg); }

	void reset() { zmq_msg_close(&msg); zmq_msg_init(&msg); }
	void reset(size_t size) { zmq_msg_close(&msg); zmq_msg_init_size(&msg, size); }

	size_t size() {	return zmq_msg_size(&msg); }
	void* data() { return zmq_msg_data(&msg); }
	bool more() { return zmq_msg_more(&msg)!=0; }

	int recv(void *zsock, int flags=0) { return zmq_msg_recv(&msg, zsock, flags); }
	int send(void *zsock, int flags=0) { return zmq_msg_send(&msg, zsock, flags); }

	std::string as_string() { return std::string(reinterpret_cast<char*>(data()), size()); }

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

private:
	// copy constructors
	msg_single_t(const msg_single_t&);
	msg_single_t& operator=(const msg_single_t&);
};

class msg_multi_t
{
public:
	std::list<msg_single_t> parts;

	msg_multi_t() {}
	void reset() { parts.clear(); }

	int recv(void *zsock)
	{
		reset();
		bool success = true;
		bool more = true;
		while (more) {
			msg_single_t frame;
			int rc = frame.recv(zsock, 0);
			if (rc==-1) {
				success = false;
				break;
			}
			more = frame.more();
			parts.push_back(std::move(frame));
		}
		return success ? 0 : -1;
	}

	int send(void *zsock)
	{
		bool success = true;
		while (!parts.empty())
		{
			msg_single_t& frame = parts.front();
			int rc = frame.send(zsock, parts.size() > 1 ? ZMQ_SNDMORE : 0);
			if (rc==-1) {
				success = false;
				break;
			}
			parts.pop_front();
		}
		return success ? 0 : -1;
	}

private:
	msg_multi_t(const msg_multi_t&);
	msg_multi_t& operator=(const msg_multi_t&);
};

