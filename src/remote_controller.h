#ifndef REMOTE_CONTROLLER_H_
#define REMOTE_CONTROLLER_H_


#include "headers.h"
#include "udp_socket.h"
#include "utility.h"
#include "data_model.h"

//this IP address changes when you change your lan cable.
const string kLocalIPForCar = "127.0.0.1";
const int kLocalPortForCar = 55555;


//this IP address is automotive shown when tethering on. When turning tethering on, this IP always changes.
const string kLocalIPForController = "192.168.8.20";
const int kLocalPortForController = 5000;



class RemoteController {
private:
  //meta information, always there
  //mac address of node, and the node type

public:
	//buffer information, updated every cycle
	pthread_mutex_t lock;
	pthread_spinlock_t spinlock;

	UdpSocket* udpsocketController_;
	UdpSocket* udpsocketCar_;

	//Car's IP address via the WIFI

	string remoteIPCar = "";
	int remotePortCar = 5555;

	// string kRemoteIPController = "192.168.8.5";
	// int kRemotePortController = 1213;

	bool use_gst_ {false};
	bool display_video_ {false};
        bool store_video_ {false};

    // frame data for gstreamer
    int gst_width_ = 640;
    int gst_height_ = 480;
    int gst_frame_rate_ = 10;
    int gst_port_ = 6666;
    string gst_h264_video_file_;
    string gst_h264_raw_data_;

	bool running;

	int argc;
	char** argv;


	RemoteController(int argc, char** argv) {
		//start two socket with different IP

		udpsocketController_ = new UdpSocket(kPacketSize);
		udpsocketController_->UdpSocketSetUp(kLocalIPForController, kLocalPortForController);

		udpsocketCar_ = new UdpSocket(kPacketSize);
		udpsocketCar_->UdpSocketSetUp(kLocalIPForCar, kLocalPortForCar);

		running = true;
		cout<<"DataPool is runing"<<endl;

		long time = currentTimeMillis();
		gst_h264_raw_data_ = to_string(time) + ".raw";
		gst_h264_video_file_ = to_string(time) + ".h264";

		this->argc = argc;
		this->argv = argv;
	}

	~RemoteController() {

	}


	static void* UDPReceiverForCar(void* dataPool);
	static void* ControlPanel(void* dataPool);
	static void* GstreamerReceiver(void* dataPool);

};

#endif
