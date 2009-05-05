#include <QtGui/QApplication>
#include "itemassistant.h"
#include <ItemAssistantCore/Logger.h>
#include "Version.h"


using namespace aoia;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Logger::instance().init("ItemAssistant.log", g_versionNumber);

    ItemAssistant w;
    w.show();
    
    //QObject::connect(a, SIGNAL(QApplication::aboutToQuit()), &Logger::instance(), SLOT(Logger::destroy()));

    int retval = a.exec();

    Logger::instance().destroy();

    return retval;
}
