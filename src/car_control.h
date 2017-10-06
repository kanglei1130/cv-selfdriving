#ifndef CAR_CONTROL_H_
#define CAR_CONTROL_H_

#include "headers.h"

class CarControl {
public:
	uint64_t time_;
	float throttle_;
	float steering_;

	uint64_t carStatus_time_;
	float carStatus_speed_;
	float carStatus_steering_;

    string timeStamp_;
    string SequenceNo_;
    string dataLength_;

    CarControl() {
    	throttle_ = 0.5;
        steering_ = 0.5;
        time_ = currentTimeMillis();

    	carStatus_speed_ = 0.0;
        carStatus_steering_ = 0.0;
    	carStatus_time_= currentTimeMillis();

    	timeStamp_ = "";
        SequenceNo_ = "";
        dataLength_ = "";
    }
    void parseController(String cmd){

    }

    void DataProcess() {

    }

    static void* UDPReceiver(void* dataPool);
    static void* ControlPanel(void* dataPool);
    static void* sendTimeBack(void* dataPool);

};


#endif
