#pragma once

#include <QObject>
#include <QThread>
#include "Queue.h"
extern "C" {
#include <libavutil/frame.h>
}

class PlayThread  : public QThread
{
	Q_OBJECT

public:
	PlayThread();
	~PlayThread();
	void run();

	void setQueue(Queue<AVFrame*>* que);
private:
	Queue<AVFrame*>* m_que = nullptr;
};
