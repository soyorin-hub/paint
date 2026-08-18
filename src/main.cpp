#include <QApplication>
#include <QFile>
#include "app/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Paint");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("PaintStudio");

    // 设置全局默认字体
    QFont defaultFont = app.font();
    defaultFont.setPointSize(10);
    app.setFont(defaultFont);

    // 加载全局主题样式
    QFile themeFile(":/theme.qss");
    if (themeFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QString::fromUtf8(themeFile.readAll()));
        themeFile.close();
    }

    MainWindow window;
    window.resize(1280, 800);
    window.show();

    return app.exec();
}
