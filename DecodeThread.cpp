#include "DecodeThread.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libavutil/avutil.h"
#include "libavutil/ffversion.h"
#include <libavutil/imgutils.h>
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libpostproc/postprocess.h"
}

DecodeThread::DecodeThread()
{}

DecodeThread::~DecodeThread()
{}

void DecodeThread::setPktQueue(Queue<AVPacket*>* que)
{
	m_pktQue = que;
}

void DecodeThread::setFrameQueue(Queue<AVFrame*>* que)
{
	m_frametQue = que;
}

void DecodeThread::setCodecContext(AVCodecContext* pCtx)
{
	pCodecCtx = pCtx;
}

void DecodeThread::run()
{
	while (1)
	{
		if (!pCodecCtx || m_pktQue->isEmpty())
		{
			QThread::msleep(1);
			continue;
		}
		AVPacket* pkt;
		m_pktQue->pop(pkt);
		if (avcodec_send_packet(pCodecCtx, pkt) >= 0) {
			AVFrame* pFrame = av_frame_alloc();
			while (avcodec_receive_frame(pCodecCtx, pFrame) >= 0) {
				m_frametQue->push(pFrame);
				pFrame = av_frame_alloc();
			}
		}
		av_packet_unref(pkt);
	}
}
