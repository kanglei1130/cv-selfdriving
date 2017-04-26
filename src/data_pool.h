#ifndef DATA_POOL_H_
#define DATA_POOL_H_


#include "headers.h"
#include "udp_socket.h"

const int kLocalPort = 4444;
const string kLocalIP = "192.168.1.100";

const int kRemotePort = 55555;
const string kRemoteIP = "192.168.1.101";

class DataPool {
private:
  //meta information, always there
  //mac address of node, and the node type


public:
	//buffer information, updated every cycle
	pthread_mutex_t lock;
	pthread_spinlock_t spinlock;

	UdpSocket* udpsocket_;
	bool running;

	DataPool() {
		udpsocket_ = new UdpSocket(kPacketSize);
		udpsocket_->UdpSocketSetUp(kLocalIP, kLocalPort);
		running = true;
	}

	~DataPool() {

	}
};

#endif
