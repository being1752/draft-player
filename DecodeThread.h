#pragma once

#include <QObject>
#include <QThread>
#include "Queue.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}


class DecodeThread  : public QThread
{
	Q_OBJECT

public:
	DecodeThread();
	~DecodeThread();

	void setPktQueue(Queue<AVPacket*>* que);
	void setFrameQueue(Queue<AVFrame*>* que);
	void setCodecContext(AVCodecContext* pCtx);

protected:
	void run();

private:
	Queue<AVPacket*>* m_pktQue = nullptr;
	Queue<AVFrame*>* m_frametQue = nullptr;
	AVCodecContext* pCodecCtx = nullptr;
};
