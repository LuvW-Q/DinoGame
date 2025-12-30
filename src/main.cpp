#include "gamewindow.h"
#include <QApplication>

/**
 * 应用入口：创建 QApplication 与主窗口并进入事件循环。
 */
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    GameWindow w;
    w.show();
    return QApplication::exec();
}