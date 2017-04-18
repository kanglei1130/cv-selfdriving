#ifndef CAR_CONTROL_H_
#define CAR_CONTROL_H_

#include "headers.h"

class CarControl {
public:
	int speed_;
    int rotation_;
    CarControl() {
        speed_ = 0;
        rotation_ = 5;
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
    		this->rotation_ = -1;
    		break;
    	case 'd':
    		this->rotation_ = 1;
    		break;
    	default:
    		cout<<"unknown command"<<endl;
    		break;
    	}
    }
};


#endif
