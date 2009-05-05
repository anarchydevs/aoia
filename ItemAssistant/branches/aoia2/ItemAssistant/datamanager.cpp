#include "datamanager.h"

namespace aoia {

    DataManager::DataManager(QObject *parent)
        : QObject(parent)
    {

    }

    DataManager::~DataManager()
    {

    }


    bool DataManager::init(QString dbfile)
    {
        return false;
    }


    void DataManager::insertItem(unsigned int keylow, unsigned int keyhigh, unsigned short ql, 
        unsigned short flags, unsigned short stack, unsigned int parent, unsigned short slot, 
        unsigned int children, unsigned int owner)
    {

    }

}   // namespace
