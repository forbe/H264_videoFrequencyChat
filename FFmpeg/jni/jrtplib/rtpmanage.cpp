#include "rtpmanage.h"

pthread_mutex_t mutexpluss = PTHREAD_MUTEX_INITIALIZER;


H264RtpFun::H264RtpFun() {
}

H264RtpFun::~H264RtpFun() {
}

int H264RtpFun::rtp_params_init(string ipStr, unsigned int destport,
		unsigned int portbase, double frate) {

	if (ipStr.empty() || destport <= 0 || portbase <= 0) {
//		__android_log_print(ANDROID_LOG_INFO, "ktian", "please input: ip destport and portbse!");
		return -1;
	}

	uint32_t IP; // = ipStr.toInt();

	IP = inet_addr(ipStr.c_str());

	if (IP == INADDR_NONE) {
//		__android_log_print(ANDROID_LOG_INFO, "ktian", "ERROR:--Bad IP address specifie");
		return -1;
	}

	IP = ntohl(IP);

	sessparams.SetOwnTimestampUnit((double) (1.0f / 90000.0f));
	//sessparams.SetAcceptOwnPackets(true);
	sessparams.SetUsePollThread(true);
	sess.SetDefaultPayloadType(H264);
	sess.SetMaximumPacketSize(UDP_MAX_SIZE);

	transparams.SetPortbase(portbase);
//	__android_log_print(ANDROID_LOG_INFO, "ktian", "SetPortbass\n");
	status = sess.Create(sessparams, &transparams);
//	__android_log_print(ANDROID_LOG_INFO, "ktian",
//						"Creat session\n");
	checkerror(status);

	RTPIPv4Address addr(IP, destport);

	status = sess.AddDestination(addr);
//	__android_log_print(ANDROID_LOG_INFO, "ktian", "Add to Destination\n");
	checkerror(status);

	//sess.SetDefaultTimestampIncrement(90000/201);

	//264packet
	framerate = frate;
	timestamp_increse = (unsigned int) (90000.00 / framerate); //+0.5);
	packetNum = 0;

//	__android_log_print(ANDROID_LOG_INFO, "ktian", "rtp init success!");

//	mutexpluss = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&mutexpluss, NULL);
	h264Buf = (unsigned char *) malloc(ONE_FRAME_LENGTH);
	h264Size = 0;
	dataflag = 0;

	return 0;
}

void H264RtpFun::rtp_params_uninit() {
	isReceive = false;
	sess.BYEDestroy(RTPTime(3, 0), "rtpDestroy", 10);
	pthread_mutex_destroy(&mutexpluss);

//	__android_log_print(ANDROID_LOG_INFO, "ktian", "rtp uninit success!");
}

int H264RtpFun::checkerror(int rtperr) {
	if (rtperr < 0) {
//		__android_log_print(ANDROID_LOG_INFO, "ktian", "ERROR:--%s", RTPGetErrorString(rtperr).c_str());
		return -1;
	}
	return 0;
}


///////////////////////////////////////////h264 packet send////////////////////////////////////////////

unsigned int H264RtpFun::setTimeIncrese(time_t start, time_t end,
		unsigned int packets) {
	double time = difftime(end, start);
	framerate = (double) packets / time;
	__android_log_print(ANDROID_LOG_INFO, "ktian", "framerate:%.2f\n", framerate);
	unsigned int timeIncrese = (unsigned int) (90000.00 / framerate);
	return timeIncrese;
}



void H264RtpFun::rtp_send(unsigned char *nalu, int length) {

	unsigned int validLength = length - 4;
	unsigned char *nal = &nalu[4];
	unsigned char *sendbuf = (unsigned char*) calloc(length, 1);

	if (sendbuf == NULL) {
		return;
	}
	if (0 == packetNum) {
		timestart = time(NULL);
	}

	//30s*30侦
	if (packetNum >= REFERENCE_PACKET) {
		timeend = time(NULL);
		timestamp_increse = setTimeIncrese(timestart, timeend, packetNum);
		packetNum = 0;
		timestart = time(NULL);
	}

	if (validLength <= MAX_RTP_PKT_LENGTH) {
		memcpy(sendbuf, nal, validLength);
		status = sess.SendPacket((void *) sendbuf, validLength, H264, true,
				timestamp_increse);
		packetNum++;
		checkerror(status);
	} else if (validLength > MAX_RTP_PKT_LENGTH) {
		int k = 0, l = 0;
		validLength -= 1;
		k = validLength / MAX_RTP_PKT_LENGTH;
		l = validLength % MAX_RTP_PKT_LENGTH;
		if (l != 0) {
			k = k + 1;
		}
		int t = 0; //用于指示当前发送的是第几个分片RTP包
		unsigned char nalHeader = nal[0]; // NALU 头

		//while(t<k||(t==k&&l>0))
		while (t < k) {
			sendbuf[0] = (nalHeader & 0xe0) | 0x1c; //1C
			sendbuf[1] = (nalHeader & 0x1f); // S=0 E=0 R=0

			//if((0==t)||(t<k&&0!=t))//第一包到最后一包的前一包
			if (t < k - 1) {
				if (0 == t) { //first packet
					sendbuf[1] |= 0x80; //S=1
				}
				memcpy(&sendbuf[2], &nal[1 + MAX_RTP_PKT_LENGTH * t],
						MAX_RTP_PKT_LENGTH);
				status = sess.SendPacket((void *) sendbuf,
						MAX_RTP_PKT_LENGTH + 2, H264, false, timestamp_increse);
			} else { //最后一包  //if((k==t&&l>0)||(t==(k-1)&&l==0))
				int endLen = 0;
				if (l > 0) {
					endLen = validLength - t * MAX_RTP_PKT_LENGTH;
				} else {
					endLen = MAX_RTP_PKT_LENGTH;
				}

				sendbuf[1] |= 0x40; //E=1
				memcpy(&sendbuf[2], &nal[1 + MAX_RTP_PKT_LENGTH * t], endLen);

				status = sess.SendPacket((void *) sendbuf, endLen + 2, H264,
						true, timestamp_increse);

			}
			checkerror(status);
			t++;
			packetNum++;

		} //end while
		  // RTPTime::Wait(RTPTime(0,1000));   //第一个参数为秒，第二个参数为wei秒
	}

	free(sendbuf);
}


///////////////////////////////////////////h264 packet receive////////////////////////////////////////////

int H264RtpFun::UnpackRTPH264(unsigned char *packet, int length, unsigned char *h264data, int *datasize) {

	unsigned char FU_INDICATOR_TYPE = packet[0] & 0x1f;
	unsigned char FU_HEADER_SEI = packet[1] & 0xe0;
	unsigned char NAL_HEAD = (packet[0] & 0xe0) | (packet[1] & 0x1f);

	if (0x1c == FU_INDICATOR_TYPE) { //判断NAL的类型为0x1c=28，说明是FU-A分片
		if (0x80 == FU_HEADER_SEI) { //first packet
			memset(h264data, 0, 10);

			h264data[3] = 0x1;
			h264data[4] = NAL_HEAD;
			memcpy(&h264data[5], packet + 2, length - 2);
			*datasize = length - 2 + 5;

			FRAME_START = true;
		} else if (0x40 == FU_HEADER_SEI) { //last pscket
			memcpy(h264data, packet + 2, length - 2);
			*datasize = length - 2;

			FRAME_START = false;
		} else { //medial packet
			memcpy(h264data, packet + 2, length - 2);
			*datasize = length - 2;

			FRAME_START = false;
		}
	} else {
		memset(h264data, 0, 10);

		h264data[3] = 0x1;
		memcpy(&h264data[4], packet, length);
		*datasize = length + 4;
	}

	return 0;
}

void H264RtpFun::clearH264data() {
	h264packet = h264packetHead;
	h264datanum = 0;
}

void H264RtpFun::rtp_receive() {
	// 打开循环
	isReceive = true;

	unsigned char h264data[MAX_RTP_PKT_LENGTH + 10];
	int datasize = 0;

	h264packet = (unsigned char*)malloc(ONE_FRAME_LENGTH);
	if(NULL == h264packet) {
		return;
	}
	h264packetHead = h264packet;
	FRAME_START = false;

	unsigned char *packet;
	size_t length; //payload length

	//数据接收
	while (isReceive) {
		sess.BeginDataAccess();

		// check incoming packets
		if (sess.GotoFirstSourceWithData()) {
			do {
				RTPPacket *pack;

				while ((pack = sess.GetNextPacket()) != NULL) {
					// You can examine the data here
					if (H264 == pack->GetPayloadType()) {

						length = pack->GetPayloadLength();
						packet = pack->GetPayloadData();

//						sequenceNum = pack->GetSequenceNumber();
//						/*
//						 if(sequenceNum-1 != sequenceNumOld)	//丢包
//						 {
//						 newFrame = false;
//						 continue;
//						 }
//						 */

						if (pack->HasMarker()) {
							UnpackRTPH264(packet, length, h264data, &datasize);
							memcpy(h264packet, h264data, datasize);
							h264datanum += datasize;
							h264packet = h264packetHead;

							// 拷贝到缓冲区
							pthread_mutex_lock(&mutexpluss);
							memcpy(h264Buf, h264packet, h264datanum);	// 给缓冲区赋值
							h264Size = h264datanum;
							pthread_mutex_unlock(&mutexpluss);
							dataflag = 1;

							h264datanum = 0;
						} else {
							UnpackRTPH264(packet, length, h264data, &datasize);

							if (FRAME_START) { //防止包相连	//分片中间丢包暂未处理
								clearH264data();
							}

							memcpy(h264packet, h264data, datasize);
							h264packet += datasize;
							h264datanum += datasize;
						}

					}

					// we don't longer need the packet, so we'll delete it
					sess.DeletePacket(pack);
				}

			} while (sess.GotoNextSourceWithData()); //第二个会话的数据？

		}


		sess.EndDataAccess();

#ifndef RTP_SUPPORT_THREAD
		status = sess.Poll();
		checkerror(status);
#endif // RTP_SUPPORT_THREAD

		RTPTime::Wait(RTPTime(0, 1000 * 4));
	} //end while

	free(h264packet);
}

int H264RtpFun::rtp_read(unsigned char *data) {

	if(dataflag == 0) {
		return -1;
	}

	pthread_mutex_lock(&mutexpluss);
	memcpy(data, h264Buf, h264Size);
	pthread_mutex_unlock(&mutexpluss);

	dataflag = 0;

	return h264Size;
}
