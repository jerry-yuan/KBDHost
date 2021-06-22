#ifndef ABSTRACTSUBAPPLICATION_H
#define ABSTRACTSUBAPPLICATION_H

#include <QObject>
#include <QMenu>

class AbstractSubApplication {
public:
    virtual void regActions(QMenu* contextMenu)=0;
    virtual void keyboardAttached(){};
    virtual void keyboardDettached(){};
};

#endif // ABSTRACTSUBAPPLICATION_H
