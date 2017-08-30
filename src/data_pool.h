#ifndef DATA_POOL_H_
#define DATA_POOL_H_


#include "headers.h"
#include "udp_socket.h"

const int kLocalPort = 5000;
//this IP address is automotive shown when tethering on. When turning tethering on, this IP always changes.
const string kLocalIPFromController = "192.168.42.60";
//this IP address changes when you change your lan cable.
const string kLocalIPFromCar = "192.168.10.103";


//Car's IP address via the WIFI
const int kRemotePort = 55555;
const string kRemoteIP = "192.168.10.104";

//this is controller's IP address. When turning tethering on, this IP is always the same connected with the PC' same USB port.
//const int kRemotePortControllor = 55555;
//const string kRemoteIPControllor = "192.168.42.129";

class DataPool {
private:
  //meta information, always there
  //mac address of node, and the node type

public:
	//buffer information, updated every cycle
	pthread_mutex_t lock;
	pthread_spinlock_t spinlock;

	UdpSocket* udpsocketController_;
	UdpSocket* udpsocketCar_;

	bool running;

	DataPool() {
		//start two socket with different IP
		udpsocketController_ = new UdpSocket(kPacketSize);
		udpsocketController_->UdpSocketSetUp(kLocalIPFromController, kLocalPort);

		udpsocketCar_ = new UdpSocket(kPacketSize);
		udpsocketCar_->UdpSocketSetUp(kLocalIPFromCar, kLocalPort);

		running = true;
		cout<<"DataPool is runing"<<endl;
	}

	~DataPool() {

	}
};

#endif
