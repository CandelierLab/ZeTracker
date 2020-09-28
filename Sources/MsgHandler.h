#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include <QtMessageHandler>
#include <QTextBrowser>
#include <QScrollBar>
#include <QFile>
#include <QFontDatabase>
#include <QTimer>
#include <QString>
#include <QVector>
#include <QRegExp>
#include <iostream>

#define TITLE_1 "css{p class='title1'}"
#define TITLE_2 "css{p class='title2'}"
#define THREAD "css{p class='thread'}"

using namespace std;

/* =================================================================== *\
|    Low level message handling tools                                   |
\* =================================================================== */

struct Message {
    QtMsgType type;
    QString text;
    QString css;
};

extern QVector<Message> Messages;

void MsgHandlerFcn(QtMsgType type, const QMessageLogContext &context, const QString &msg);

/* =================================================================== *\
|    MsgHandler Class                                                   |
\* =================================================================== */

class MsgHandler : public QObject {

    Q_OBJECT

public:

    // Constructor and destructor
    MsgHandler(QTextBrowser*);
    ~MsgHandler();

    QTextBrowser* uiOutput;

public slots:

    void UpdateMessage();

private:

};


#endif // MSGHANDLER_H
