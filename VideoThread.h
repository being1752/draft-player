#pragma once

#include <QObject>
#include <QImage>
#include <QThread>

class AVFormatContext;
class AVInputFormat;
class AVCodecParameters;
class AVCodecContext;
class AVCodec;
class AVFrame;
class AVPacket;

class VideoThread : public QThread
{
	Q_OBJECT

public:
	VideoThread();
	~VideoThread();

	bool init();

signals:
	void sgnSendImage(QImage img);

protected:
	void run();

private:
	AVFormatContext* ic = nullptr;
	AVInputFormat* iformat = nullptr;

	AVCodecParameters* pCodecParam = nullptr;
	AVCodecContext* pCodecCtx = nullptr;
	AVCodec* pCodec = nullptr;

	AVFrame* pFrame = nullptr;
	AVFrame* pFrameRGB = nullptr;
	AVPacket* pkt = nullptr;
};
