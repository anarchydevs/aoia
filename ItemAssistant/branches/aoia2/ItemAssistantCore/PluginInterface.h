#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QWidget>


namespace aoia {

    struct MessageHandlerInterface
    {
        virtual ~MessageHandlerInterface() {}
        virtual void handleMessage() = 0;
    };


    struct GuiPluginInterface
    {
        virtual ~GuiPluginInterface() {}

        virtual QString getLabel() const = 0;
        virtual QWidget* getMainWidget(QWidget* parent) = 0;
        virtual QWidget* getSidebarWidget(QWidget* parent) = 0;
        virtual void activate() = 0;
        virtual void deactivate() = 0;
    };


    class PluginInterface
    {
    public:
        virtual ~PluginInterface() {}

        virtual void initPlugin() = 0;
    };

}

Q_DECLARE_INTERFACE(aoia::PluginInterface, "net.frellu.ia.PluginInterface/1.0");
Q_DECLARE_INTERFACE(aoia::MessageHandlerInterface, "net.frellu.ia.MessageHandlerInterface/1.0");
Q_DECLARE_INTERFACE(aoia::GuiPluginInterface, "net.frellu.ia.GuiPluginInterface/1.0");

#endif // PLUGININTERFACE_H
