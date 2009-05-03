#include <QtGui/QApplication>
#include "itemassistant.h"

using namespace aoia;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ItemAssistant w;
    w.show();
    return a.exec();
}
