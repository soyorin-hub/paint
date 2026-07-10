#include <QApplication>
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

    MainWindow window;
    window.resize(1280, 800);
    window.show();

    return app.exec();
}
