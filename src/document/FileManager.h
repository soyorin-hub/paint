#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>

class Document;
class QGraphicsScene;

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);

    // 保存/加载 .vdraw 文件
    bool saveDocument(const QString &path, Document *document);
    Document *loadDocument(const QString &path);

    // 导出
    bool exportSvg(const QString &path, QGraphicsScene *scene);
    bool exportPng(const QString &path, QGraphicsScene *scene);

    // 最近文件
    QStringList recentFiles() const { return m_recentFiles; }
    void addRecentFile(const QString &path);

private:
    QStringList m_recentFiles;
    static const int MAX_RECENT_FILES = 5;
};

#endif // FILEMANAGER_H
