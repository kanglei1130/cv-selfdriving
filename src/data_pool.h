#ifndef DATA_POOL_H_
#define DATA_POOL_H_


#include "headers.h"
#include "udp_socket.h"

const int kLocalPort = 55555;
const string kLocalIP = "127.0.0.1";

const int kRemotePort = 4444;
const string kRemoteIP = "127.0.0.1";

class DataPool {
private:
  //meta information, always there
  //mac address of node, and the node type


public:
	//buffer information, updated every cycle
	pthread_mutex_t lock;
	pthread_spinlock_t spinlock;

	UdpSocket* udpsocket_;

	DataPool() {
		udpsocket_ = new UdpSocket(kPacketSize);
		udpsocket_->UdpSocketSetUp(kLocalIP, kLocalPort);
	}

	~DataPool() {

	}
};

#endif
