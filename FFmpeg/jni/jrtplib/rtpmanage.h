#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpmemorymanager.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>

#include <android/log.h>

using namespace std;

/******************************* RTP **********************************/
#define REFERENCE_PACKET    30*30*7  // 30s的包数
#define MAX_RTP_PKT_LENGTH  1380
#define H264                96
#define UDP_MAX_SIZE 		1400
#define ONE_FRAME_LENGTH 	70000

using namespace jrtplib;

class H264RtpFun {

public:
	H264RtpFun();
	~H264RtpFun();

	int rtp_params_init(string ipStr, unsigned int destport, unsigned int baseport,  double framerate);
	void rtp_params_uninit();
	int checkerror(int rtperr);

	// 发送RTP
	unsigned int setTimeIncrese(time_t start, time_t end, unsigned int packets);
	void rtp_send(unsigned char *buf, int len);

	// 接收RTP
	int UnpackRTPH264(unsigned char *packet, int length, unsigned char *h264data, int *datasize);
	void clearH264data();
	void rtp_receive();
	int rtp_read(unsigned char *data);

private:
	RTPSession sess;
	RTPUDPv4TransmissionParams transparams;
	RTPSessionParams sessparams;

	uint32_t destip;
	string ipstr;
	int status;
	unsigned int packetNum;

	time_t timestart, timeend;
	double framerate;
	unsigned int timestamp_increse;

	bool isReceive;

	// 接收H264缓冲区
	unsigned char *h264Buf;
	int h264Size;
	int dataflag;


	unsigned int h264datanum;
	unsigned char *h264packet;
	unsigned char *h264packetHead;
	bool FRAME_START;

//private slots:

};

