/*
 * data_model.cpp
 *
 *  Created on: Oct 24, 2017
 *      Author: lkang
 */

#include "data_model.h"

FrameData::FrameData() {

}
FrameData::~FrameData() {

}

string FrameData::toJson() {
	Json::Value jsonData;
	jsonData["videoSendTime"] = (Json::Value::UInt64)this->videoSendTime;
	jsonData["PCtime"] = (Json::Value::UInt64)currentTimeMillis();
	jsonData["Sequence"] = Json::Value(this->Sequence);
	jsonData["isIFrame"] = Json::Value(this->isIFrame);
	jsonData["originalDataSize"] = Json::Value(this->originalDataSize);
	jsonData["compressedDataSize"] = Json::Value(this->compressedDataSize);
	jsonData["type"] = utility::FrameDataFromServer;
	// cout<<jsonData.toStyledString()<<endl;
	// write JSON object to a string
	Json::FastWriter fastWriter;
	std::string output = fastWriter.write(jsonData);
	return output;
}

void FrameData::fromJson(const std::string& json) {
	Json::Value parsedFromString;
	Json::Reader reader;
	assert(reader.parse(json, parsedFromString));

	//parse json data and read more data
	this->videoSendTime = parsedFromString["videoSendTime"].asUInt64();
	this->Sequence = parsedFromString["Sequence"].asUInt();
	this->isIFrame = parsedFromString["isIFrame"].asBool();
	this->originalDataSize = parsedFromString["originalDataSize"].asUInt();
	//long videoLength  = parsedFromString["frameData"].toStyledString().length()-2;
	this->compressedDataSize = parsedFromString["compressedDataSize"].asUInt();


	this->steering = parsedFromString["steering"].asDouble();
	this->speed = parsedFromString["speed"].asDouble();
}


ControlCommand::ControlCommand() {

}

ControlCommand::~ControlCommand() {

}

string ControlCommand::toJson() {
	Json::Value jsonData;
	jsonData["timeStamp"] = (Json::Value::UInt64)this->timeStamp;
	jsonData["steering"] = Json::Value(this->steering);
	jsonData["throttle"] = Json::Value(this->throttle);
	jsonData["type"] = utility::ControlMessageFromServer;
	// cout<<jsonData.toStyledString()<<endl;
	Json::FastWriter fastWriter;
	std::string output = fastWriter.write(jsonData);
	return output;
}

void ControlCommand::fromJson(const string& json) {
	Json::Value parsedFromString;
	Json::Reader reader;
	assert(reader.parse(json, parsedFromString));
	//parse json data and read more data
	this->timeStamp = parsedFromString["timeStamp"].asUInt64();
	this->steering = parsedFromString["steering"].asDouble();
	this->throttle = parsedFromString["throttle"].asDouble();

}
