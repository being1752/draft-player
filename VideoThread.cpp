#include "videoThread.h"
#include "Queue.h"
#include <QCameraInfo>
#include <QImage>
#include <QDebug>

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

VideoThread::VideoThread()
{
	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
}

VideoThread::~VideoThread()
{}

bool VideoThread::init()
{
	//注册组件：libavdevice
	avdevice_register_all();

	//创建设备上下文
	ic = avformat_alloc_context();

	if (!ic) {
		// 如果分配失败，则记录错误并设置返回错误码
		av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
		return false;
	}
	return true;
}

void VideoThread::run(){

	int nVideoIndex = -1;
	std::string fileName = "video=";

	QList<QCameraInfo> qcamerainfos = QCameraInfo::availableCameras();
	if (qcamerainfos.isEmpty()) {
		av_log(NULL, AV_LOG_FATAL, "No cameras found.\n");
		return;
	}

	fileName += qcamerainfos.at(0).description().toStdString().c_str();


	//查找摄像头设备
	iformat = const_cast<AVInputFormat*>(av_find_input_format("dshow"));
	if (!iformat) {
		av_log(NULL, AV_LOG_FATAL, "Could not find dshow input format.\n");
		return;
	}

	//打开输入设备
	int err = avformat_open_input(&ic, fileName.c_str(), iformat, nullptr);
	if (err < 0) {
		// 如果打开失败，则打印错误信息并设置返回错误码
		//print_error(fileName.c_str(), err);
		return;
	}

	//查找流信息
	avformat_find_stream_info(ic, nullptr);

	for (int i = 0; i < ic->nb_streams; i++) {
		if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			nVideoIndex = i;
			break;
		}
	}

	if (nVideoIndex == -1) {
		av_log(NULL, AV_LOG_ERROR, "Did not find a video stream.\n");
		return;
	}

	//查找编码器
	pCodecParam = ic->streams[nVideoIndex]->codecpar;
	pCodec = const_cast<AVCodec*>(avcodec_find_decoder(pCodecParam->codec_id));
	if (!pCodec) {
		av_log(NULL, AV_LOG_FATAL, "Codec not found.\n");
		return;
	}

	//创建编码器上下文
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		av_log(NULL, AV_LOG_FATAL, "Could not allocate video codec context.\n");
		return;
	}
	avcodec_parameters_to_context(pCodecCtx, pCodecParam);

	//打开编码器
	avcodec_open2(pCodecCtx, pCodec, nullptr);

	//设置帧数据转换上下文，将YUV422转化为RGB24
	struct SwsContext* img_convert_ctx = nullptr;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24,
		SWS_BICUBIC, NULL, NULL, NULL);

	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);

	uint8_t* out_buffer = (unsigned char*)av_malloc(static_cast<unsigned long long>(numBytes) * sizeof(unsigned char));

	//绑定内存块
	if (av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize,
		out_buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1) < 0)
	{
		qDebug() << "av_image_fill_arrays failed." << endl;
		return;
	}

	pkt = av_packet_alloc();
	av_new_packet(pkt, pCodecCtx->width * pCodecCtx->height);
	while (av_read_frame(ic, pkt) >= 0) {
		if (pkt->stream_index == nVideoIndex) {
			if (avcodec_send_packet(pCodecCtx, pkt) >= 0) {
				while (avcodec_receive_frame(pCodecCtx, pFrame) >= 0) {
					//将解码后数据转换为Rgb格式数据
					sws_scale(img_convert_ctx,
						pFrame->data, pFrame->linesize,
						0, pCodecCtx->height,
						pFrameRGB->data, pFrameRGB->linesize);

					QImage img(out_buffer, pFrame->width, pFrame->height, QImage::Format_RGB888);
					emit sgnSendImage(img);
				}
			}
			av_packet_unref(pkt);
		}
	}
	av_packet_free(&pkt);
	avcodec_close(pCodecCtx);
	avcodec_parameters_free(&pCodecParam);
	av_frame_free(&pFrame);
	avformat_close_input(&ic);
	avformat_free_context(ic);
}