#pragma once

#include <QObject>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

class FileUtils : public QObject
{
    Q_OBJECT

public:
    explicit FileUtils(QObject *parent = nullptr);

    // 在视频目录下创建临时目录
    Q_INVOKABLE QString createVideoTempDir(const QString &videoPath);

    // 生成基于日期的合并文件名
    Q_INVOKABLE QString generateMergedFileName(const QString &videoPath);

    // 获取合并视频的完整路径
    Q_INVOKABLE QString getMergedVideoPath(const QString &videoPath);

    // 检查文件是否存在
    Q_INVOKABLE bool fileExists(const QString &filePath);

    // 删除文件
    Q_INVOKABLE bool deleteFile(const QString &filePath);

    // 获取文件所在目录
    Q_INVOKABLE QString getFileDirectory(const QString &filePath);

    // 获取文件名（不含路径）
    Q_INVOKABLE QString getFileName(const QString &filePath);

    // 获取不带扩展名的文件名
    Q_INVOKABLE QString getFileNameWithoutExt(const QString &filePath);
};
