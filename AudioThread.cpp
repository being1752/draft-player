#include "AudioThread.h"
#include <QAudioDeviceInfo>
#include "AudioThread.h"
#include <QDebug>
#include <QAudioOutput>

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

#define MAX_AUDIO_FRAME_SIZE	192000

AudioThread::AudioThread()
{
	pFrame = av_frame_alloc();
}

AudioThread::~AudioThread()
{}

int AudioThread::AudioResample(AVCodecContext * ctx, char* out, AVFrame * avFrame)
{
	if (!ctx || !avFrame || !out)
	{
		return 0;
	}

	if (m_swrContext == NULL)
	{
		m_swrContext = swr_alloc();

// 		swr_alloc_set_opts2(&m_swrContext, &(codecParams->ch_layout),
// 			AV_SAMPLE_FMT_S16,
// 			ctx->sample_rate,
// 			codecParams->ch_layout.nb_channels,
// 			ctx->sample_fmt,
// 			ctx->sample_rate,
// 			0, 0);
// 
// 		qDebug() << "sample_rate " << ctx->sample_rate;
// 
// 		//初始化样本转换
// 		swr_init(m_swrContext);
 	}

	uint8_t* data[1];
	data[0] = (uint8_t*)out;

	int len = swr_convert(m_swrContext, data, 10000, (const uint8_t**)avFrame->data, avFrame->nb_samples);
	if (len <= 0)
	{
		return 0;
	}

	//int outsize = av_samples_get_buffer_size(NULL, codecParams->ch_layout.nb_channels, avFrame->nb_samples, AV_SAMPLE_FMT_S16, 0);

	return 0;
}

void AudioThread::setQueue(Queue<AVPacket*>* que)
{
	m_queue = que;
}

bool AudioThread::init()
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

void AudioThread::run()
{
	int nAudioIndex = -1;
	std::string fileName = "audio=";

	QList<QAudioDeviceInfo> qaudioinfos = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	if (qaudioinfos.isEmpty()) {
		av_log(NULL, AV_LOG_FATAL, "No cameras found.\n");
		return;
	}

	fileName += qaudioinfos.at(0).deviceName().toStdString();

	//查找麦克风设备
	iformat = const_cast<AVInputFormat*>(av_find_input_format("dshow"));
	if (!iformat) {
		av_log(NULL, AV_LOG_FATAL, "Could not find alsa input format.\n");
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
		if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			nAudioIndex = i;
			break;
		}
	}

	if (nAudioIndex == -1) {
		av_log(NULL, AV_LOG_ERROR, "Did not find a audio stream.\n");
		return;
	}

	//查找编码器
	pCodecParam = ic->streams[nAudioIndex]->codecpar;
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
	if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
		return;
	}

	emit sgnSetCodecContext(pCodecCtx);

	//uint8_t* m_pSwrBuffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE);
	pkt = av_packet_alloc();
	while (av_read_frame(ic, pkt) >= 0) {
		if (pkt->stream_index == nAudioIndex) {
			m_queue->push(pkt);
			pkt = av_packet_alloc();
		}
	}
	av_packet_free(&pkt);
	avcodec_close(pCodecCtx);
	avcodec_parameters_free(&pCodecParam);
	av_frame_free(&pFrame);
	avformat_close_input(&ic);
	avformat_free_context(ic);
}
