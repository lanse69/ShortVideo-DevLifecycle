#include "videoaudiomerger.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QUrl>

// 清理文件路径
QString cleanFilePath(const QString &filePath) {
    QString cleaned = filePath;

    // 去除 file:// 前缀
    if (cleaned.startsWith("file://")) {
        cleaned = cleaned.mid(7);  // 去掉前7个字符 "file://"
    }

    return cleaned;
}

VideoAudioMerger::VideoAudioMerger(QObject *parent)
    : QObject(parent)
    , m_videoStartMs(0)
    , m_videoEndMs(0)
    , m_audioStartMs(0)
    , m_audioEndMs(0)
    , m_durationMs(0)
{
    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &VideoAudioMerger::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &VideoAudioMerger::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &VideoAudioMerger::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &VideoAudioMerger::onProcessError);
}

void VideoAudioMerger::setVideoFile(const QString &filePath)
{
    m_videoFile = filePath;
}

void VideoAudioMerger::setVideoTimeRange(qint64 startMs, qint64 endMs)
{
    m_videoStartMs = startMs;
    m_videoEndMs = endMs;
    m_durationMs = endMs - startMs;
}

void VideoAudioMerger::setAudioFile(const QString &filePath)
{
    m_audioFile = filePath;
}

void VideoAudioMerger::setAudioTimeRange(qint64 startMs, qint64 endMs)
{
    m_audioStartMs = startMs;
    m_audioEndMs = endMs;
}

void VideoAudioMerger::setOutputFile(const QString &filePath)
{
    m_outputFile = filePath;
}

bool VideoAudioMerger::validateTimeRanges()
{
    qint64 videoDuration = m_videoEndMs - m_videoStartMs;
    qint64 audioDuration = m_audioEndMs - m_audioStartMs;

    if (videoDuration <= 0 || audioDuration <= 0) {
        emit errorOccurred("时间段无效：时长必须大于0");
        return false;
    }

    // 允许时长有微小差异（100毫秒内）
    if (qAbs(videoDuration - audioDuration) > 100) {
        emit errorOccurred(QString("视频时长(%1ms)和音频时长(%2ms)不匹配")
                               .arg(videoDuration).arg(audioDuration));
        return false;
    }

    m_durationMs = qMin(videoDuration, audioDuration);
    return true;
}

void VideoAudioMerger::merge()
{
    QString cleanVideoFile = m_videoFile;
    QString cleanAudioFile = m_audioFile;

    // 使用 QUrl 清理路径
    QUrl videoUrl(cleanVideoFile);
    if (videoUrl.isLocalFile()) {
        cleanVideoFile = videoUrl.toLocalFile();
    } else if (cleanVideoFile.startsWith("file://")) {
        cleanVideoFile = cleanVideoFile.mid(7);
    }

    QUrl audioUrl(cleanAudioFile);
    if (audioUrl.isLocalFile()) {
        cleanAudioFile = audioUrl.toLocalFile();
    } else if (cleanAudioFile.startsWith("file://")) {
        cleanAudioFile = cleanAudioFile.mid(7);
    }

    qDebug() << "原始视频路径:" << m_videoFile;
    qDebug() << "清理后视频路径:" << cleanVideoFile;
    qDebug() << "原始音频路径:" << m_audioFile;
    qDebug() << "清理后音频路径:" << cleanAudioFile;

    if (!validateTimeRanges()) {
        return;
    }

    if (m_videoFile.isEmpty() || m_audioFile.isEmpty() || m_outputFile.isEmpty()) {
        emit errorOccurred("请先设置视频、音频和输出文件路径");
        return;
    }

    QFileInfo videoInfo(cleanVideoFile);
    QFileInfo audioInfo(cleanAudioFile);

    if (!videoInfo.exists()) {
        emit errorOccurred("视频文件不存在：" + cleanVideoFile +
                           "\n原始路径：" + m_videoFile);
        return;
    }
    if (!audioInfo.exists()) {
        emit errorOccurred("音频文件不存在：" + cleanAudioFile +
                           "\n原始路径：" + m_audioFile);
        return;
    }

    // 确保输出目录存在
    QDir outputDir = QFileInfo(m_outputFile).absoluteDir();
    if (!outputDir.exists()) {
        outputDir.mkpath(".");
    }

    // 构建FFmpeg命令
    QStringList arguments;

    // 输入视频文件和时间段
    arguments << "-ss" << formatTime(m_videoStartMs)
              << "-i" << cleanVideoFile
              << "-t" << formatTime(m_durationMs);

    // 输入音频文件和时间段
    arguments << "-ss" << formatTime(m_audioStartMs)
              << "-i" << cleanAudioFile
              << "-t" << formatTime(m_durationMs);

    // 输出参数
    arguments << "-map" << "0:v:0"  // 使用第一个视频流
              << "-map" << "1:a:0"  // 使用第二个音频流
              << "-c:v" << "copy"   // 视频直接复制（不重新编码）
              << "-c:a" << "aac"    // 音频编码为AAC（确保兼容性）
              << "-shortest"        // 以最短的流结束
              << "-y"               // 覆盖输出文件
              << m_outputFile;

    qDebug() << "FFmpeg命令：ffmpeg" << arguments;

    // 执行FFmpeg命令
    m_process->start("ffmpeg", arguments);

    if (!m_process->waitForStarted(3000)) {  // 添加超时时间
        QString error = QString("无法启动FFmpeg进程，请确保已安装FFmpeg\n")
                        + "命令：ffmpeg " + arguments.join(" ");
        emit errorOccurred(error);
        return;
    }
}

QString VideoAudioMerger::formatTime(qint64 ms)
{
    int hours = ms / 3600000;
    int minutes = (ms % 3600000) / 60000;
    int seconds = (ms % 60000) / 1000;
    int milliseconds = ms % 1000;

    return QString("%1:%2:%3.%4")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(milliseconds, 3, 10, QChar('0'));
}

void VideoAudioMerger::onProcessOutput()
{
    QString output = m_process->readAllStandardError();

    // 解析FFmpeg输出进度
    static QRegularExpression timeRegex(R"(time=(\d{2}):(\d{2}):(\d{2})\.(\d{2}))");
    QRegularExpressionMatch match = timeRegex.match(output);

    if (match.hasMatch()) {
        int hours = match.captured(1).toInt();
        int minutes = match.captured(2).toInt();
        int seconds = match.captured(3).toInt();
        int hundredths = match.captured(4).toInt();

        qint64 currentMs = (hours * 3600 + minutes * 60 + seconds) * 1000 + hundredths * 10;

        if (m_durationMs > 0) {
            int percent = qMin(99, static_cast<int>(currentMs * 100 / m_durationMs));
            emit progressChanged(percent);
        }
    }

    if (!output.trimmed().isEmpty()) {
        qDebug() << "FFmpeg:" << output.trimmed();
    }
}

void VideoAudioMerger::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        emit progressChanged(100);
        emit mergeFinished(true, m_outputFile);
    } else {
        QString error = m_process->readAllStandardError();
        if (error.isEmpty()) {
            error = "FFmpeg进程异常退出";
        }
        emit errorOccurred(error);
        emit mergeFinished(false, "");
    }
}

void VideoAudioMerger::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "FFmpeg启动失败，请确保已安装";
        break;
    case QProcess::Crashed:
        errorMsg = "FFmpeg进程崩溃";
        break;
    case QProcess::Timedout:
        errorMsg = "FFmpeg进程超时";
        break;
    case QProcess::WriteError:
        errorMsg = "写入FFmpeg进程失败";
        break;
    case QProcess::ReadError:
        errorMsg = "读取FFmpeg进程输出失败";
        break;
    default:
        errorMsg = "FFmpeg进程未知错误";
    }

    emit errorOccurred(errorMsg);
    emit mergeFinished(false, "");
}

qint64 VideoAudioMerger::getVideoDuration(const QString &filePath)
{
    QProcess process;
    QStringList args;
    args << "-v" << "error"
         << "-show_entries" << "format=duration"
         << "-of" << "default=noprint_wrappers=1:nokey=1"
         << filePath;

    process.start("ffprobe", args);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    bool ok;
    double duration = output.trimmed().toDouble(&ok);

    if (ok) {
        return static_cast<qint64>(duration * 1000);
    }

    return -1;
}

qint64 VideoAudioMerger::getAudioDuration(const QString &filePath)
{
    return getVideoDuration(filePath);
}
