#ifndef CAR_CONTROL_H_
#define CAR_CONTROL_H_

#include "headers.h"

class CarControl {
public:
	uint64_t time_;
	int speed_;
    int steering_;
    CarControl() {
        speed_ = 0;
        steering_ = 0;
        time_ = currentTimeMillis();
    }

    void parseCommand(char cmd) {
    	switch(cmd) {
    	case 'w':
    		this->speed_ = 1;
    		break;
    	case 's':
    		this->speed_ = -1;
    		break;
    	case 'a':
    		this->steering_ = -1;
    		break;
    	case 'd':
    		this->steering_ = 1;
    		break;
    	default:
    		cout<<"unknown command:"<<cmd<<endl;
    		break;
    	}
    }
    static void* UDPReceiver(void* dataPool);
    static void* ControlPanel(void* dataPool);
};


#endif
