#include "MsgHandler.h"

/* =================================================================== *\
|    Low level message handling tools                                   |
\* =================================================================== */

QVector<Message> Messages;

void MsgHandlerFcn(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    Message MSG;
    MSG.type = type;

    QRegExp rx("css\\{(.*)\\}(.*)");

    if (rx.indexIn(msg)==-1) {

        MSG.css = QString("p");
        MSG.text = msg;

    } else {

        MSG.css = rx.cap(1);
        MSG.text = rx.cap(2).trimmed();

    }

    Messages.push_back(MSG);

}

/* =================================================================== *\
|    MsgHandler Class                                                   |
\* =================================================================== */

MsgHandler::MsgHandler(QTextBrowser* Output) : uiOutput(Output) {

    // Timer
    QTimer *t_Msg = new QTimer();
    connect(t_Msg, SIGNAL(timeout()), this, SLOT(UpdateMessage()));
    t_Msg->start(50);

    // Style
    QFile File("../ZeTracker/Infos.css");
    File.open(QFile::ReadOnly);
    QTextDocument *OutDoc = new QTextDocument;
    OutDoc->setDefaultStyleSheet(File.readAll());
    OutDoc->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    uiOutput->setDocument(OutDoc);

}

MsgHandler::~MsgHandler() {}

void MsgHandler::UpdateMessage() {

    while (Messages.length()) {

        Message MSG = Messages.takeFirst();
        QString S;

        switch (MSG.type) {
        case QtDebugMsg:
            cout << MSG.text.toStdString() << endl;
            break;
        case QtInfoMsg:
            S = "<" + MSG.css + ">" + MSG.text + "</p>" ;
            break;
        case QtWarningMsg:
            S = "<p class='warning'>" + MSG.text + "</p>";
            break;
        case QtCriticalMsg:
            S= "<p class='critical'>" + MSG.text + "</p>";
            break;
        case QtFatalMsg:
            S = "<p class='fatal'>" + MSG.text + "</p>";
            break;
        }

        uiOutput->setHtml(uiOutput->toHtml().append(S));
        uiOutput->verticalScrollBar()->setValue(uiOutput->verticalScrollBar()->maximum());
    }
}
