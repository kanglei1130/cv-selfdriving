#include "headers.h"
#include "lane_marker_detector.h"
#include "receiver_socket.h"
#include "data_pool.h"
#include "udp_socket.h"
#include "car_control.h"

#include <string>
#include <sstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <sstream>



static const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
            	char_array_4[i] = base64_chars.find(char_array_4[i]);
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;
        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        for (j = 0; (j < i - 1); j++)
        	ret += char_array_3[j];
    }
    return ret;
}

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
//this thread process data from the car, which include the image and speed status data.
void* CarControl::UDPReceiver(void* param){
	cout<<"Enter UDP Receiver from Car"<<endl;;
	DataPool *dataPool = (DataPool*)param;

	string kRemoteIP;
	int32_t kRemotePort;
	string data;

	while(dataPool->running) {
		dataPool->udpsocketCar_->ReceiveFrom(kRemoteIP, kRemotePort, data);
		if(data.length()>0){
			//send data back to android
			dataPool->udpsocketCar_->SendTo(kRemoteIP, kRemotePort,parseH264Data(data));

			//string imageData = parsedFromString["video_"].asString();
			//dataPool->udpsocketController_->SendTo(PC_IP, PC_Port, imageData);
			cout<<"received video data"<<endl;
		}
/*
 * This is for parse data from controller and OpenCV data
 * before using , change IP and Port Number and comment
 * "dataPool->udpsocketCar_->SendTo(kRemoteIP, kRemotePort,parseH264Data(data));"
 */
		  /*Json::Value parsedFromString;
			Json::Reader reader;
			assert(reader.parse(data, parsedFromString));

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
	cout<<"UDPReceiver exit"<<endl;
	pthread_exit(NULL);
}



