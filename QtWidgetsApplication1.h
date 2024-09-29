#pragma once

#include <QMainWindow>
#include <QImage>
#include "ui_QtWidgetsApplication1.h"

class VideoThread;
class AudioThread;
class DecodeThread;
class PlayThread;
class QAudioFormat;
class QAudioOutput;
class QIODevice;
class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget *parent = nullptr);
    ~QtWidgetsApplication1();

    void openCamera();
    void addMask(QPixmap& pm, const QString& text);
    void start();

public slots:
    void slotDisplayCameraImage(QImage img);
/*    void slotDisplayAudio(QByteArray data);*/

private:
    Ui::QtWidgetsApplication1Class ui;
    VideoThread *m_videoThread;
    AudioThread* m_audioThread;
    DecodeThread* m_decodeThread;
    PlayThread* m_playThread;
//     QAudioFormat* m_format;
// 	QAudioOutput* audioOutput;
// 	QIODevice* audioDevice;
};
