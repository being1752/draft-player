#pragma once

#include <QObject>
#include <QThread>
#include <QByteArray>
#include "Queue.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

class AVCodecContext;
class SwrContext;
class AudioThread  : public QThread
{
	Q_OBJECT

public:
	AudioThread();
	~AudioThread();

	int AudioResample(AVCodecContext* ctx, char* out, AVFrame* avFrame);
	void setQueue(Queue<AVPacket*>* que);

	bool init();

signals:
	void sgnSendAudio(QByteArray data);
	void sgnSetCodecContext(AVCodecContext* pCodecCtx);

protected:
	void run();

private:
	AVFormatContext* ic = nullptr;
	AVInputFormat* iformat = nullptr;

	AVCodecParameters* pCodecParam = nullptr;
	AVCodecContext* pCodecCtx = nullptr;
	AVCodec* pCodec = nullptr;
	SwrContext* m_pSwrContext = nullptr;

	AVFrame* pFrame = nullptr;
	AVPacket* pkt = nullptr;

	SwrContext* m_swrContext = nullptr;
	Queue<AVPacket*>* m_queue = nullptr;
};
