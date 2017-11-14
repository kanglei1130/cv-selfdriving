/*
 * packet_aggregator.cpp
 *
 *  Created on: Nov 13, 2017
 *      Author: lkang
 */

#include "packet_aggregator.h"

PacketAggregator::PacketAggregator() {

}

PacketAggregator::~PacketAggregator() {

}

FrameAndData PacketAggregator::generateFrame(FramePacket header, unsigned char **data) {
  int len = header.packetLength;
  int k = header.k;
  string payload = "";
  for (int i = 0; i < k; ++i) {
    payload += string(reinterpret_cast<char*>(data[i]), len);
  }
  FrameData frameData;
  return make_pair(frameData, payload);
}


void PacketAggregator::aggregatePackets(set<PacketAndData>& videoPackets, const int sequence) {
  int sz = videoPackets.size();
  assert(sz > 0);
  FramePacket samplePkt = (*videoPackets.begin()).first;

  int k = samplePkt.k;
  unsigned int len = samplePkt.packetLength;
  unsigned char *data_blocks[k];
  for (int i = 0; i < k; ++i) {
    data_blocks[i] = new unsigned char[len];
  }
  set<int> dataIndexes;
  set<int> fecIndexes;
  for (PacketAndData curPkt: videoPackets) {
    FramePacket packet = curPkt.first;
    assert (sequence == packet.frameSequence);
    int index = packet.index;
    if (index < k) {
      memcpy((void*)data_blocks[index], (void*)curPkt.second.c_str(), len);
      dataIndexes.insert(index);
    } else {
      fecIndexes.insert(index);
    }
  }

  int dataCnt = dataIndexes.size();
  if (dataIndexes.size() == k) {
    // process data blocks since all orignal packets are received

    return;
  }

  unsigned int * fec_block_nos = new unsigned int[dataCnt];
  unsigned int * erased_blocks = new unsigned int[k - dataCnt];
  for (int i = 0, j = 0; i < k; ++i) {
    if (dataIndexes.count(i) == 0) {
      erased_blocks[j++] = i;
    }
  }
  int j = 0;
  for (int index: fecIndexes) {
    fec_block_nos[j++] = index;
  }


  int fecCnt = fecIndexes.size();
  unsigned char *fec_blocks[fecCnt];
  for (int i = 0; i < fecCnt; ++i) {
    fec_blocks[i] = new unsigned char[len];
  }
  j = 0;
  for (PacketAndData curPkt: videoPackets) {
    FramePacket packet = curPkt.first;
    int index = packet.index;
    fec_block_nos[j] = index;
    memcpy((void*)fec_blocks[j], (void*)curPkt.second.c_str(), len);
  }
  FEClib::fec_decode(len, data_blocks, dataCnt, fec_blocks, fec_block_nos,erased_blocks, fecCnt);
  for (int i = 0; i < k; ++i) {
    delete [] data_blocks[i];
  }
  for (int i = 0; i < fecCnt; ++i) {
    delete [] fec_blocks[i];
  }
  delete [] fec_block_nos;
  delete [] erased_blocks;
}


void PacketAggregator::insertPacket(FramePacket header, string& data) {
  int sequence = header.frameSequence;
  int k = header.k;
  if (sequenceCounter.first > sequence) {
    return;
  } else if (sequenceCounter.first == sequence) {
    sequenceCounter.second ++;
    if (sequenceCounter.second == k) {
      // aggregate current one
    }
  } else {
    if (sequenceCounter.second != 0) {
      // aggregate last one

    }
    this->videoPackets.insert(make_pair(header, data));
    sequenceCounter.first = sequence;
    sequenceCounter.second = 1;

  }
}


