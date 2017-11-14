/*
 * packet_aggregator.h
 *
 *  Created on: Nov 13, 2017
 *      Author: lkang
 */

#ifndef PACKET_AGGREGATOR_H_
#define PACKET_AGGREGATOR_H_

#include "data_model.h"
#include "fec.h"


/**
 * Assume frame comes in order, i.e., when the first packet of frame x + 1 comes,
 * we aggregate the packets for frame x, no matter there is enough packets
 * If we get enough packets for frame x, we will start aggregate, and ignore future packets
 * of frame x
 */

using PacketAndData = pair<FramePacket, string>;
using FrameAndData = pair<FrameData, string>;
struct classComp {
  bool operator() (const PacketAndData& a, const PacketAndData& b) const
  { return a.first.index > b.first.index; }
};


class PacketAggregator {
private:
  pair<int, int> sequenceCounter;
  set<PacketAndData, classComp> videoPackets;
public:
  PacketAggregator();
  ~PacketAggregator();
  FrameAndData generateFrame(FramePacket header, unsigned char **data);
  void insertPacket(FramePacket header, string& data);
  void aggregatePackets(set<PacketAndData>& videoPackets, const int sequence);

};


#endif /* PACKET_AGGREGATOR_H_ */
