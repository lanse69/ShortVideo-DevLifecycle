#pragma once

#include <QObject>
#include <QString>
#include <QProcess>
#include <QTime>

class VideoAudioMerger : public QObject
{
    Q_OBJECT

public:
    explicit VideoAudioMerger(QObject *parent = nullptr);

    // 添加 Q_INVOKABLE 修饰符！！！
    Q_INVOKABLE void setVideoFile(const QString &filePath);
    Q_INVOKABLE void setVideoTimeRange(qint64 startMs, qint64 endMs);
    Q_INVOKABLE void setAudioFile(const QString &filePath);
    Q_INVOKABLE void setAudioTimeRange(qint64 startMs, qint64 endMs);
    Q_INVOKABLE void setOutputFile(const QString &filePath);
    Q_INVOKABLE bool validateTimeRanges();
    Q_INVOKABLE void merge();

    // 获取时长（毫秒）
    qint64 getDuration() const { return m_durationMs; }

signals:
    void progressChanged(int percent);
    void mergeFinished(bool success, const QString &outputPath);
    void errorOccurred(const QString &error);

private slots:
    void onProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    QString m_videoFile;
    QString m_audioFile;
    QString m_outputFile;
    qint64 m_videoStartMs;
    qint64 m_videoEndMs;
    qint64 m_audioStartMs;
    qint64 m_audioEndMs;
    qint64 m_durationMs;

    QProcess *m_process;

    // 转换时间格式：毫秒 -> HH:MM:SS.mmm
    QString formatTime(qint64 ms);
    // 获取视频时长
    qint64 getVideoDuration(const QString &filePath);
    qint64 getAudioDuration(const QString &filePath);
};

