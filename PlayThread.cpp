#include "PlayThread.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>

PlayThread::PlayThread()
{}

PlayThread::~PlayThread()
{}

void PlayThread::run()
{
	QAudioFormat format;
	format.setSampleRate(48000);                  // ���ò�����
	format.setChannelCount(2);
	format.setSampleFormat(QAudioFormat::Int16);   // ���ò�����ʽΪ�з��� 16 λ����

	QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
	if (!info.isFormatSupported(format)) { 
		format = info.nearestFormat(format); 
	}
	QAudioOutput* audioOutput = new QAudioOutput(format, this);
	QIODevice* audioDevice = audioOutput->start();
	while (1)
	{
		if (m_que->isEmpty())
		{
			QThread::msleep(1);
			continue;
		}
		AVFrame* frame;
		m_que->pop(frame);
		audioDevice->write((char*)frame->data[0], frame->linesize[0]);
		av_frame_unref(frame);
	}
	audioOutput->stop();
	delete audioOutput;
}

void PlayThread::setQueue(Queue<AVFrame*>* que)
{
	m_que = que;
}
