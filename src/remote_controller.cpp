#include "remote_controller.h"
#include "lane_marker_detector.h"
#include "udp_socket.h"
#include <string>
#include <sstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <sstream>


/**
 * The first character must be '{'
 */
int getJsonHeaderIndex(string& data) {
	int hend = 0;
	int num_left_brace = 0, num_right_brace = 0;
	for(int i = 0; i < data.size(); ++i) {
		char ch = data[i];
		if (ch == '{') {
			num_left_brace++;
		} else if (ch == '}') {
			num_right_brace++;
		} else {

		}
		// cout<<"ch:"<<ch<<" "<<num_left_brace<<" "<<num_right_brace<<endl;
		if (num_left_brace == num_right_brace) {
			hend = i;
			break;
		}
	}
	return hend;
}

//this thread process data from the car, which include the image and speed status data.
void* RemoteController::UDPReceiverForCar(void* param){
	cout<<"Enter UDP Receiver from Car"<<endl;;
	RemoteController *dataPool = (RemoteController*)param;
	string data;
	std::ofstream ofs (dataPool->gst_h264_raw_data_, std::ifstream::app);

	while(dataPool->running) {
		dataPool->udpsocketCar_->ReceiveFrom(dataPool->remoteIPCar, dataPool->remotePortCar, data);
		if (data.length() <= 0) {
			continue;
		}
		//send data back to android
		int hend = getJsonHeaderIndex(data);
		assert(hend != 0);
		std::string header = data.substr(0, hend + 1);
		std::string body = data.substr(hend + 1);

		std::cout<<header<<std::endl;

		Json::Value parsedFromString;
		Json::Reader reader;
		assert(reader.parse(header, parsedFromString));

		if (parsedFromString["type"].asString() == utility::FrameDataFromCar) {
			FrameData frameData;
			frameData.fromJson(header);
			dataPool->udpsocketCar_->SendTo(dataPool->remoteIPCar, dataPool->remotePortCar, frameData.toJson());

			if (dataPool->use_gst_) {
				dataPool->udpsocketCar_->SendTo("127.0.0.1", dataPool->gst_port_, body);
			}
                        if (dataPool->store_video_) {
			  string frame_separate = to_string(body.size()) + "\n";
			  ofs.write(frame_separate.c_str(), frame_separate.size());
			  ofs.write(body.c_str(), body.size());
                        }
		} else {
			cout<<"Unknown Type:"<<parsedFromString.toStyledString()<<endl;
		}
	}
	ofs.close();
	cout<<"UDPReceiver exit"<<endl;
	pthread_exit(NULL);
}

//this thread only process data from controller
void* RemoteController::ControlPanel(void* param)
{
	cout<<"receive controllor Thread"<<endl;;
	RemoteController *dataPool = (RemoteController*)param;

	string kRemoteIPController;
	int32_t kRemotePortController;
	string data;

	while(!dataPool->running) {
		dataPool->udpsocketController_->ReceiveFrom(kRemoteIPController, kRemotePortController, data);
		if (data.length() <= 0 || dataPool->remoteIPCar == "") {
			continue;
		}
		Json::Value parsedFromString;
		Json::Reader reader;
		assert(reader.parse(data, parsedFromString));

		if (parsedFromString["type"].asString() == utility::ControlMessageFromController) {
			ControlCommand controlCommand;
			controlCommand.fromJson(data);
			dataPool->udpsocketCar_->SendTo(dataPool->remoteIPCar, dataPool->remotePortCar, controlCommand.toJson());
		} else {
			cout<<"Unknown Type:"<<parsedFromString.toStyledString()<<endl;
		}

	}
	cout<<"ControlPanel exit"<<endl;
	pthread_exit(NULL);
}


void* RemoteController::GstreamerReceiver(void* param){
	RemoteController *dataPool = (RemoteController*)param;
	if (!dataPool->use_gst_) {
		pthread_exit(NULL);
	}
	cout<<"Enter Gstreamer Receiver from Car"<<endl;;

	GstElement *pipeline;
	GstBus *bus;
	GstMessage *msg;
	/* Initialize GStreamer */
	gst_init (&dataPool->argc, &dataPool->argv);

	/* Build the pipeline */
	std::string udpsrc = "udpsrc port=" + to_string(dataPool->gst_port_);
	std::string video = "video/x-h264,width=" + to_string(dataPool->gst_width_) + ",height=" + to_string(dataPool->gst_height_)
			+ ",framerate=" + to_string(dataPool->gst_frame_rate_) + "/1,aligment=au,stream-format=avc";
    std::string file = "filesink location=" + dataPool->gst_h264_video_file_;


    std:string input = "";

    if (dataPool->display_video_) {
    	input = udpsrc + " ! " + video + " ! " + "avdec_h264" + " ! " + "autovideosink";
    } else {
    	input = udpsrc + " ! " + video + " ! " + "avdec_h264" + " ! " + "avimux" + " ! " + file;
    }

	pipeline = gst_parse_launch (input.c_str(), NULL);


	/* Start playing */
	gst_element_set_state (pipeline, GST_STATE_PLAYING);

	/* Wait until error or EOS */
	bus = gst_element_get_bus (pipeline);

	msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType) (GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

	/* Free resources */
	if (msg != NULL) {
		gst_message_unref (msg);
	}
	gst_object_unref (bus);
	gst_element_set_state (pipeline, GST_STATE_NULL);
	gst_object_unref (pipeline);
	pthread_exit(NULL);
}




