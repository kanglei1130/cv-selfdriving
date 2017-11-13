/*
 * data_model.h
 *
 *  Created on: Oct 24, 2017
 *      Author: lkang
 */

#ifndef DATA_MODEL_H_
#define DATA_MODEL_H_


#include "utility.h"

struct FramePacket {
  uint64_t packetSendTime{0};
  uint32_t frameSequence{0}; // uniquely identify a frame
  short k{0};
  short n{0};
  short index{0}; // 0 ... n - 1


  FramePacket();
  ~FramePacket();
  string toJson();
  void fromJson(const std::string& json);
};

struct FrameData {
  uint64_t frameSendTime{0};
  uint32_t transmitSequence{0};
  bool isIFrame{false};
  uint32_t originalDataSize{0};
  uint32_t compressedDataSize{0};
  uint64_t serverTime{0};
  uint32_t rawFrameIndex{0};
  double speed{0.0};
  double steering{0.5};

  FrameData();
  ~FrameData();
  string toJson();
  void fromJson(const std::string& json);
};

struct ControlCommand {
  uint64_t timeStamp{0};
  double steering{0.5};
	double throttle{0.5};

	ControlCommand();
	~ControlCommand();
	string toJson();
	void fromJson(const std::string& json);
};


#endif /* DATA_MODEL_H_ */
