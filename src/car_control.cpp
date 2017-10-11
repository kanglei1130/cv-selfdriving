#include "headers.h"
#include "lane_marker_detector.h"
#include "data_pool.h"
#include "udp_socket.h"
#include "car_control.h"

#include <string>
#include <sstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <sstream>



std::string parseH264Data (string data){
	Json::Value parsedFromString;
	Json::Reader reader;
	assert(reader.parse(data, parsedFromString));
	bool parsingSuccessful = reader.parse(data, parsedFromString);
	if(parsingSuccessful) {
		//parse json data and read more data
		std::string timeStamp = parsedFromString["videoSendTime"].asString();
		std::string SequenceNo = parsedFromString["Sequence"].asString();
		std::string isIFrame = parsedFromString["isIFrame"].asString();
		std::string originalDataSize = parsedFromString["originalDataSize"].asString();
		//long videoLength  = parsedFromString["frameData"].toStyledString().length()-2;
		std::string comDataSize = parsedFromString["compressedDataSize"].asString();
		std::string roundBackTime = parsedFromString["roundBackTime"].asString();

		//package data to json
		Json::Value jsonData;
		jsonData["videoSendTime"] = timeStamp;
		jsonData["PCtime"] = to_string(currentTimeMillis());
	    jsonData["Sequence"] = SequenceNo;
		jsonData["isIFrame"] = isIFrame;
		jsonData["originalDataSize"] = originalDataSize;
		jsonData["compressedDataSize"] = comDataSize;
		jsonData["roundBackTime"] = roundBackTime;

		cout<<jsonData.toStyledString()<<endl;
		// write JSON object to a string
		Json::FastWriter fastWriter;
		std::string output = fastWriter.write(jsonData);
		return output;
	}
}


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
void* CarControl::UDPReceiver(void* param){
	cout<<"Enter UDP Receiver from Car"<<endl;;
	DataPool *dataPool = (DataPool*)param;

	string kRemoteIP;
	int32_t kRemotePort;
	string data;

	std::ofstream ofs (dataPool->gst_h264_raw_data_, std::ifstream::app);

	while(dataPool->running) {
		dataPool->udpsocketCar_->ReceiveFrom(kRemoteIP, kRemotePort, data);
		if(data.length()>0){
			//send data back to android
			int hend = getJsonHeaderIndex(data);
			int len = data.size();
			assert(hend != 0);
			std::string header = data.substr(0, hend + 1);
			std::string body = data.substr(hend + 1);

			dataPool->udpsocketCar_->SendTo(kRemoteIP, kRemotePort,parseH264Data(header));

			if (dataPool->use_gst_) {
				dataPool->udpsocketCar_->SendTo("127.0.0.1", dataPool->gst_port_, body);
			}

			string frame_separate = to_string(len) + "\n";
			ofs.write(frame_separate.c_str(), frame_separate.size());
			ofs.write(body.c_str(), len);
		}

		/*
			bool parsingSuccessful = reader.parse(data, parsedFromString);

			if(parsingSuccessful) {
				std::string parsedType = parsedFromString["type_"].toStyledString();
				std::string image = "\"image\"\n";
				std::string status = "\"status\"\n";

				//if received data is image, then show the video
				if(parsedType == image){
					cout<<"read image"<<endl;
					std::string parsedData = parsedFromString["imgData_"].toStyledString();
					std::string parsedTimeData = parsedFromString["time_"].toStyledString();
					//there is always "" outside the string, so use string from 1 to length()-1.
					string encoded_string = parsedData.substr(1,parsedData.length()-1);
					//make sure the encode method is compared to the decode method, java have different encode method with c++
					string decoded_string = base64_decode(encoded_string);
					vector<uchar> dataVec(decoded_string.begin(), decoded_string.end());
					Mat img = imdecode(dataVec, IMREAD_UNCHANGED);
					if(!img.empty()){
						//send time data and image length data back to car. time with {} and length with [].
						//this time include the time of image processing to video.
						std::string imageTranTime = "time-"+parsedTimeData.substr(1,parsedTimeData.length()-1)+",length:"+ to_string(parsedData.length());
						cout<<imageTranTime<<endl;
						dataPool->udpsocketCar_->SendTo(kRemoteIP, kRemotePort, imageTranTime);
						//show the image as video
						cout<<"Height: " << img.rows <<" Width: "<<img.cols<<endl;
						imshow("window", img);
					}
					waitKey(33);

				//if received data reports the car speed, just print it out
				}else if(parsedType == status){
					CarControl carControl;
					carControl.carStatus_speed_ = parsedFromString["speed_"].asFloat();
					carControl.carStatus_steering_ = parsedFromString["steering_"].asFloat();
					carControl.carStatus_time_ = parsedFromString["time_"].asInt64 ();
					cout<<"received car status: speed-"<<endl;
					printf( "%6.4lf", carControl.carStatus_speed_ );
				}else{
					cout<<"unknown impute"<<endl;
				}
			} else {
				cout<<"udpsocketCar json format error:" + data<<endl;
			}*/
	}
	ofs.close();
	cout<<"UDPReceiver exit"<<endl;
	pthread_exit(NULL);
}

//this thread only process data from controller
string senddata = "";
void* CarControl::ControlPanel(void* param)
{
	cout<<"receive controllor Thread"<<endl;;
	DataPool *dataPool = (DataPool*)param;

	string kRemoteIPController;
	int32_t kRemotePortController;
	string data;

	while(!dataPool->running) {
		dataPool->udpsocketController_->ReceiveFrom(kRemoteIPController, kRemotePortController, data);
		if(data.length()>0){
			//we don't need to parse the data to check if it is controller data, because it comes from controller.
			//we can directly send what received to car, however in this case, we can't know if there is an error.
			//so I still parse the data and keep the code form line 138-148.
			Json::Value parsedFromString;
			Json::Reader reader;
			assert(reader.parse(data, parsedFromString));

			bool parsingSuccessful = reader.parse(data, parsedFromString);

			if(parsingSuccessful) {
				std::string parsedType = parsedFromString["type_"].toStyledString();
				std::string controller = "\"controller\"\n";
				if(parsedType == controller){
					//update controller data. Only changed data will be sent to car;

					if (data != senddata){
						dataPool->udpsocketCar_->SendTo(kRemoteIP, kRemotePort, senddata);
						senddata =data;
					}
					//print value only not equal to 0.5. Useful when you want to save controller data in PC.
					//you can comment it if you just want to pass the controller data directly to car.
					CarControl carControl;
				    carControl.throttle_ = parsedFromString["throttle_"].asFloat();
				    carControl.steering_ = parsedFromString["steering_"].asFloat();
				    carControl.time_ = parsedFromString["time_"].asInt64 ();
					if (carControl.steering_ != 0.5||carControl.throttle_ != 0.5){
						cout<<carControl.steering_<<endl;
						cout<<carControl.throttle_<<endl;
						cout<<carControl.time_<<endl;
					}
				}
			}else{
				cout<<"unknown impute from controller"<<endl;
			}

			cout<<"data"<<endl;

		} else {
			cout<<"No data is been received from Controller" + data<<endl;
		}
			//long time = (long)parsedFromString["time_"].asInt64 ();
			//cout<<currentTimeMillis() - time<<endl;
	}
	cout<<"ControlPanel exit"<<endl;
	pthread_exit(NULL);
}


void* CarControl::GstreamerReceiver(void* param){
	DataPool *dataPool = (DataPool*)param;
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
			+ ",framerate=10/1,aligment=au,stream-format=avc";
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




