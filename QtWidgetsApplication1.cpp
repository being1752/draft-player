#include "QtWidgetsApplication1.h"
#include <QThread>
#include <Qpainter>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioOutput>
#include "VideoThread.h"
#include "AudioThread.h"
#include "DecodeThread.h"
#include "PlayThread.h"
#include "Queue.h"

QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	connect(ui.pushButton, &QPushButton::clicked, this, &QtWidgetsApplication1::start);
	openCamera();
}

QtWidgetsApplication1::~QtWidgetsApplication1()
{}

void QtWidgetsApplication1::openCamera()
{
	Queue<AVPacket*>* pktQue = new Queue<AVPacket*>();
	Queue<AVFrame*>* frameQue = new Queue<AVFrame*>();

	m_videoThread = new VideoThread();
	m_audioThread = new AudioThread();
	m_decodeThread = new DecodeThread();
	m_playThread = new PlayThread();

	m_audioThread->setQueue(pktQue);
	m_decodeThread->setPktQueue(pktQue);
	m_decodeThread->setFrameQueue(frameQue);
	m_playThread->setQueue(frameQue);

	m_videoThread->init();
	m_audioThread->init();

// 	m_format = new QAudioFormat();
// 	m_format->setSampleRate(48000);  // ������
// 	m_format->setChannelCount(2);    // ������
// 	m_format->setSampleSize(16);     // ÿ��������λ��
// 	m_format->setCodec("audio/pcm"); // ������ʽ
// 	m_format->setByteOrder(QAudioFormat::LittleEndian);
// 	m_format->setSampleType(QAudioFormat::SignedInt);
// 
// 	audioOutput = new QAudioOutput(*m_format);
// 	audioDevice = audioOutput->start();  // ��ʼ����

	//������źŲ�
	connect(m_videoThread, &VideoThread::sgnSendImage, this, &QtWidgetsApplication1::slotDisplayCameraImage, Qt::BlockingQueuedConnection);
	//connect(m_audioThread, &AudioThread::sgnSendAudio, this, &QtWidgetsApplication1::slotDisplayAudio, Qt::BlockingQueuedConnection);
	connect(m_audioThread, &AudioThread::sgnSetCodecContext, m_decodeThread, &DecodeThread::setCodecContext, Qt::BlockingQueuedConnection);

	m_videoThread->start();
	m_audioThread->start();
	m_decodeThread->start();
	m_playThread->start();
}

void QtWidgetsApplication1::addMask(QPixmap& pm, const QString& text)
{
	QPainter painter(&pm);
	int fontSize = 25, spacing = 10;
	QFont font("΢���ź�", fontSize, QFont::Thin);
	font.setLetterSpacing(QFont::AbsoluteSpacing, spacing);
	painter.setFont(font);
	painter.setPen(QColor(150, 150, 150));
	painter.translate(pm.width() / 2, -pm.width() / 2);//����λ��
	painter.rotate(45);
	int squareEdgeSize = pm.width() * sin(45.0) + pm.height() * sin(45.0);//�Խ��߳���
	int hCount = squareEdgeSize / ((fontSize + spacing) * (text.size() + 1)) + 1;
	int x = squareEdgeSize / hCount + (fontSize + spacing) * 3;
	int y = x / 2;

	for (int i = 0; i < hCount; i++)
	{
		for (int j = 0; j < hCount * 2; j++)
		{
			painter.drawText(x * i, y * j, text);
		}
	}
}

void QtWidgetsApplication1::start()
{
	while (1);
}

// void QtWidgetsApplication1::slotDisplayAudio(QByteArray data)
// {
// 	audioDevice->write(data);
// }

void QtWidgetsApplication1::slotDisplayCameraImage(QImage img){
	QPixmap pixmap = QPixmap::fromImage(img);
	//addMask(pixmap, "Hello World");
	ui.label->setPixmap(pixmap);
}