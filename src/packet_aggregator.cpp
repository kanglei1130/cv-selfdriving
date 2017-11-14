/*
 * packet_aggregator.cpp
 *
 *  Created on: Nov 13, 2017
 *      Author: lkang
 */

#include "packet_aggregator.h"

PacketAggregator::PacketAggregator() {
  FEClib::fec_init();
}

PacketAggregator::~PacketAggregator() {

}

string PacketAggregator::generateFrame(FramePacket header, unsigned char **data) {
  int len = header.packetLength;
  int k = header.k;
  string payload = "";
  for (int i = 0; i < k; ++i) {
    payload += string(reinterpret_cast<char*>(data[i]), len);
  }
  return payload;
}


void PacketAggregator::aggregatePackets(set<PacketAndData, classComp>& videoPackets, int sequence) {
  int sz = videoPackets.size();
  assert(sz > 0);
  FramePacket samplePkt = (*videoPackets.begin()).first;
  FrameData frameData;
  frameData.frameSendTime = samplePkt.packetSendTime;
  frameData.transmitSequence = samplePkt.frameSequence;

  int k = samplePkt.k;
  unsigned int len = samplePkt.packetLength;

  int dataCnt = 0;
  int fecCnt = 0;
  // survey
  for (PacketAndData curPkt: videoPackets) {
    FramePacket packet = curPkt.first;
    assert (sequence == packet.frameSequence);
    int index = packet.index;
    if (index < k) {
      dataCnt ++;
    } else {
      fecCnt ++;
    }
  }

  if (fecCnt == 0 || fecCnt + dataCnt < k) {
    // process data blocks since all orignal packets are received or
    string payload = "";
    for (PacketAndData curPkt: videoPackets) {
      FramePacket packet = curPkt.first;
      int index = packet.index;
      if (index < k) {
        payload += curPkt.second;
      }
    }
    //
    this->videoFrames.push_back(make_pair(frameData, payload));
    return;
  }

  // recover needed
  unsigned char *data_blocks[k];
  for (int i = 0; i < k; ++i) {
    data_blocks[i] = new unsigned char[len];
  }

  set<int> dataIndexes;

  unsigned int * fec_block_nos = new unsigned int[dataCnt];
  unsigned int * erased_blocks = new unsigned int[k - dataCnt];

  unsigned char *fec_blocks[fecCnt];
  for (int i = 0; i < fecCnt; ++i) {
    fec_blocks[i] = new unsigned char[len];
  }

  int j = 0;
  for (PacketAndData curPkt: videoPackets) {
    FramePacket packet = curPkt.first;
    int index = packet.index;
    if (index < k) {
      memcpy((void*)data_blocks[index], (void*)curPkt.second.c_str(), len);
      dataIndexes.insert(index);
    } else {
      fec_block_nos[j++] = index;
      memcpy((void*)fec_blocks[j], (void*)curPkt.second.c_str(), len);
    }
  }

  for (int i = 0, j = 0; i < k; ++i) {
    if (dataIndexes.count(i) == 0) {
      erased_blocks[j++] = i;
    }
  }
  FEClib::fec_decode(len, data_blocks, dataCnt, fec_blocks, fec_block_nos, erased_blocks, fecCnt);

  string payload = generateFrame(samplePkt, data_blocks);
  this->videoFrames.push_back(make_pair(frameData, payload));

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
  }
  if (sequenceCounter.first == sequence) {
    sequenceCounter.second ++;
    if (sequenceCounter.second == k) {
      // aggregate current one
      aggregatePackets(this->videoPackets, sequence);
      this->videoPackets.clear();
      sequenceCounter.first ++;
      sequenceCounter.second = 0;
    }
  } else {
    if (sequenceCounter.second != 0) {
      // aggregate last one
      aggregatePackets(this->videoPackets, this->sequenceCounter.first);
    }
    this->videoPackets.clear();
    this->videoPackets.insert(make_pair(header, data));
    sequenceCounter.first = sequence;
    sequenceCounter.second = 1;
  }
}

void PacketAggregator::deaggregatePackets(FrameData& frameData, string& payload) {
  int sz = frameData.compressedDataSize;
  uint64_t sendTime = frameData.frameSendTime;
  const int referencePktSize = 2000;
  // minimize padding
  int k = sz/referencePktSize + 1;
  int blockSize = sz / k + 1;
  payload += string(k * blockSize - sz, 'p');


  int n = k + 2;

  if (payload.size() / 1500 < 3) {

  } else {

  }
  unsigned char *data_blocks[k];
  for (int i = 0; i < k; ++i) {
    data_blocks[i] = new unsigned char[blockSize];
  }
  int fecCnt = 0;
  unsigned char *fec_blocks[fecCnt];
  for (int i = 0; i < fecCnt; ++i) {
    fec_blocks[i] = new unsigned char[blockSize];
  }
  FEClib::fec_encode(blockSize, data_blocks, k, fec_blocks, n - k);

  for (int i = 0; i < k; ++i) {
    delete [] data_blocks[i];
  }
  for (int i = 0; i < fecCnt; ++i) {
    delete [] fec_blocks[i];
  }
}


