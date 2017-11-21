
CXX=g++
CVFLAGS=$(shell pkg-config --cflags opencv)
CVLIBS=$(shell pkg-config --libs opencv)

DEBUGFLAGS = -g -w  #-DTEST 

GSTREAMERFLAGS=$(shell pkg-config --cflags gstreamer-1.0)
GSTREAMERLIBS=$(shell pkg-config --libs gstreamer-1.0)

CXXFLAGS   = $(CVFLAGS) $(GSTREAMERFLAGS) $(DEBUGFLAGS)
CXXFLAGS += --std=c++11

OTHERLIBS = -ljsoncpp -lpthread

LIBS = $(CVLIBS) $(GSTREAMERLIBS) $(OTHERLIBS)



remote_control: ./src/main.o ./src/remote_controller.o \
  ./src/lane_marker_detector.o ./src/udp_socket.o ./src/utility.o \
  ./src/data_model.o ./src/fec.o ./src/packet_aggregator.o ./src/tcp_server_socket.o
	$(CXX) $(CXXFLAGS) $^ -o remote_control $(LIBS)


$(BUILD_DIR)%.o: ./src/%.cpp ./src/%.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<
	
clean:
	rm remote_control ./src/*.o
	
	
