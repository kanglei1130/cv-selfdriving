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

    CarControl() {
    	throttle_ = 0.5;
        steering_ = 0.5;
        time_ = currentTimeMillis();

    	carStatus_speed_ = 0.0;
        carStatus_steering_ = 0.0;
    	carStatus_time_= currentTimeMillis();
    }
    void parseController(String cmd){

    }

    static void* UDPReceiver(void* dataPool);
    static void* ControlPanel(void* dataPool);
};


#endif
