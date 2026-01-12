// fileutils.cpp
#include "fileutils.h"
#include <QDebug>

FileUtils::FileUtils(QObject *parent) : QObject(parent)
{
}

QString FileUtils::createVideoTempDir(const QString &videoPath)
{
    if (videoPath.isEmpty()) {
        return "";
    }

    // 获取原视频目录
    QFileInfo fileInfo(videoPath);
    QString videoDir = fileInfo.absolutePath();

    // 创建临时目录（在原视频目录下）
    QString tempDir = videoDir + "/_merged_videos";
    QDir dir(tempDir);

    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "无法创建临时目录:" << tempDir;
            return "";
        }
    }

    return tempDir;
}

QString FileUtils::generateMergedFileName(const QString &videoPath)
{
    if (videoPath.isEmpty()) {
        return "";
    }

    QFileInfo fileInfo(videoPath);
    QString baseName = fileInfo.baseName();

    // 使用当前日期（格式：yyyyMMdd）
    QString dateStr = QDateTime::currentDateTime().toString("yyyyMMdd");

    // 生成新文件名：原文件名_合并_日期.mp4
    return QString("%1_merged_%2.mp4").arg(baseName).arg(dateStr);
}

QString FileUtils::getMergedVideoPath(const QString &videoPath)
{
    if (videoPath.isEmpty()) {
        return "";
    }

    // 创建临时目录
    QString tempDir = createVideoTempDir(videoPath);
    if (tempDir.isEmpty()) {
        return "";
    }

    // 生成文件名
    QString fileName = generateMergedFileName(videoPath);

    // 返回完整路径
    return tempDir + "/" + fileName;
}

bool FileUtils::fileExists(const QString &filePath)
{
    return QFile::exists(filePath);
}

bool FileUtils::deleteFile(const QString &filePath)
{
    return QFile::remove(filePath);
}

QString FileUtils::getFileDirectory(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.absolutePath();
}

QString FileUtils::getFileName(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName();
}

QString FileUtils::getFileNameWithoutExt(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.completeBaseName();
}
