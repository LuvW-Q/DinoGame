#include "gamewindow.h"
#include <QApplication>

/**
 * 应用入口：创建 QApplication 与主窗口并进入事件循环。
 * @param argc 参数数量（Qt 传入）。
 * @param argv 参数数组（Qt 传入）。
 */
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    GameWindow w;
    w.show();
    return QApplication::exec();
}