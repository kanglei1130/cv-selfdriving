#include "headers.h"
#include "lane_marker_detector.h"
#include "receiver_socket.h"
#include "data_pool.h"
#include "udp_socket.h"
#include "car_control.h"

static void* UDPReceiver(void* dataPool);
static void* ControlPanel(void* dataPool);

void startThreads() {

	DataPool* dataPool = new DataPool();
	int num = 2;

	pthread_t threads[num];
	pthread_create (&(threads[0]), NULL, &UDPReceiver, dataPool);
	pthread_create (&(threads[1]), NULL, &ControlPanel, dataPool);

	for(int i = 0; i < num; ++i) {
		pthread_join(threads[i], NULL);
	}
	delete dataPool;
}

static void* UDPReceiver(void* param)
{
	cout<<"Enter UDP Receiver"<<endl;;
	DataPool *dataPool = (DataPool*)param;

	string remoteip;
	int32_t remoteport;
	string data;
	while(true) {
		dataPool->udpsocket_->ReceiveFrom(remoteip, remoteport, data);
		//cout<<data<<endl;

		Json::Value parsedFromString;
		Json::Reader reader;
		assert(reader.parse(data, parsedFromString));

		//cout<<parsedFromString["type_"]<<endl;
		//cout<<parsedFromString["data_"]<<endl;

	}
	pthread_exit(NULL);
}

static void* ControlPanel(void* param)
{
	cout<<"Enter Scheduler"<<endl;;
	DataPool *dataPool = (DataPool*)param;
	struct termios old_tio, new_tio;
	unsigned char c;
	/* get the terminal settings for stdin */
	tcgetattr(STDIN_FILENO,&old_tio);
	/* we want to keep the old setting to restore them a the end */
	new_tio=old_tio;
	/* disable canonical mode (buffered i/o) and local echo */
	new_tio.c_lflag &=(~ICANON & ~ECHO);
	/* set the new settings immediately */
	tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);
	do {
		c=getchar();
		printf("%c\n",c);

		CarControl carControl;
		carControl.parseCommand(c);
		Json::Value data;
		data["speed_"] = carControl.speed_;
		data["rotation_"] = carControl.rotation_;
		string str = data.toStyledString();
		dataPool->udpsocket_->SendTo(kRemoteIP, kRemotePort, str);
	} while(c!='q');

	cout<<"ControlPanel exit"<<endl;
	/* restore the former settings */
	tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);
	pthread_exit(NULL);
}



void startUdpServer() {
	const int kUdpPort = 55555;
	const string ip = "127.0.0.1";
	UdpSocket* udpsocket = new UdpSocket(kPacketSize);
	udpsocket->UdpSocketSetUp(ip, kUdpPort);
	while(true) {
		string remoteip;
		int32_t remoteport;
		string data;
		udpsocket->ReceiveFrom(remoteip, remoteport, data);
		cout<<data<<endl;

		Json::Value parsedFromString;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse(data, parsedFromString);
		if(parsingSuccessful) {
			cout<<parsedFromString["type_"]<<endl;
			cout<<parsedFromString["data_"]<<endl;
		} else {
			cout<<"json format error:" + data<<endl;
		}

	}
	/*
	ReceiverSocket socket(kUdpPort);
	if (socket.BindSocketToListen()) {
		std::cout << "Listening on port " << kUdpPort << "." << std::endl;
	    while (true) {  // TODO: break out cleanly when done.
	    	std::vector<unsigned char> data = socket.GetPacket();
	    	cv::Mat frame = cv::imdecode(data, cv::IMREAD_COLOR);
	    	if (!frame.data) {
	    		std::cerr << "Could not decode image data." << std::endl;
	    		return;
	    	}
	    	cv::imshow("streaming video", frame);
	    	cv::waitKey(15);
	    }
	}
	*/
}




