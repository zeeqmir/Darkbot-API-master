#include "browser_window.h"
#include "ui_browser_window.h"
#include <QMouseEvent>
#include <QTextCodec>
#include <iostream>
#include <sstream>
#include <vector>
#include <regex>
#include <QTimer>
#include <Windows.h>
#include <QWebEngineSettings>

BrowserWindow::BrowserWindow(QWidget *parent) :
    QMainWindow(parent){
    ui = new Ui::BrowserWindow();
    ui->setupUi(this);

    browser = new QWebEngineView(this);
    profile = new QWebEngineProfile();
    page = new QWebEnginePage(profile, browser);

    //
    profile->clearHttpCache();
    profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);

    browser->setPage(page);
    browser->load(QUrl("https://darkorbit.com"));

    ui->gridLayout_2->addWidget(browser);

    /* Remove ugly white edges. */
    ui->centralWidget->layout()->setContentsMargins(0,0,0,0);
    ui->mainToolBar->hide();
    statusBar()->hide();

    /* Set pipe name to be the browser pid*/
    DWORD pid = GetCurrentProcessId();
    char buf[30];
    sprintf(buf, "\\\\.\\pipe\\%lu", pid);

    server = new QLocalServer(this);
    server->listen(buf);


    // Url changed
    connect(browser, SIGNAL(urlChanged(QUrl)), this, SLOT(checkUrl(QUrl)));

    // Client connected
    connect(server, SIGNAL(newConnection()), this, SLOT(processNewConnection()));

    // Data Received
    connect(server,  SIGNAL(&QLocalSocket::readyRead), this, SLOT(&MainWindow::parseCommand));

    // Get browser widget event handler?
    foreach(QObject* obj, browser->children())
    {
        QWidget* wgt = qobject_cast<QWidget*>(obj);
        if (wgt)
        {
            eventsReciverWidget = wgt;
            wgt->installEventFilter(this);
            break;
        }
    }

    Qt::WindowFlags flags = Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint;
    this->setWindowFlags(flags);
}


/* Redirects to game when login is successfull. */
void BrowserWindow::checkUrl(QUrl url){
    std::string url_txt = url.toString().toStdString();
    if(url_txt.find("?action=internalStart") != std::string::npos){
        std::string server;
        std::regex server_pattern("https://(.*?)\.darkorbit.com");
        std::smatch sm;

        std::regex_search(url_txt, sm, server_pattern, std::regex_constants::match_any);
        if(sm.size() > 1){
            QString game_url = QString().fromStdString("https://"+sm[1].str()+".darkorbit.com/indexInternal.es?action=internalMapRevolution");
            browser->load(QUrl(game_url));
        }
    }
}

void BrowserWindow::processNewConnection(){
    socket = new QLocalSocket(this);
    socket = server->nextPendingConnection();
    connect(socket,SIGNAL(readyRead()), this, SLOT(parseCommand()));
}

void BrowserWindow::parseCommand(){
    QLocalSocket* clientSocket = (QLocalSocket*)sender();
    QByteArray recievedData;

    recievedData = clientSocket -> readAll();
    std::string data = recievedData.toStdString();
    std::istringstream iss(data);
    std::vector<std::string>  commands;

    // Split command
    std::string s;
    while (getline(iss, s, '|')) {
        commands.push_back(s);
    }


    sendResponse = true;
    if(commands[0] == "click" && commands.size() == 3){
        mouseClick(std::stoi(commands[1]), std::stoi(commands[2]));
    }

    if(commands[0] == "mousePress" && commands.size() == 3){
        mousePress(std::stoi(commands[1]), std::stoi(commands[2]));
    }

    if(commands[0] == "mouseRelease" && commands.size() == 3){
        mouseRelease(std::stoi(commands[1]), std::stoi(commands[2]));
    }

    if(commands[0] == "mouseMove" && commands.size() == 3){
        mouseMove(std::stoi(commands[1]), std::stoi(commands[2]));
    }

    if(commands[0] == "keyClick" && commands.size() == 2){
        keyClick(std::stoi(commands[1]));
    }
    if(commands[0] == "refresh"){
        refresh();
    }
}

bool BrowserWindow::eventFilter(QObject* pObject, QEvent* pEvent){
    /* Filter mouse move events */
    if (blockMouse && pEvent->type() == QEvent::MouseMove){
        blockMouse = true;
        return true;
    }

    return false;

}

void BrowserWindow::mouseClick(int x, int y){
    QPoint clickPos = QPoint(x, y);
    QMouseEvent *press = new QMouseEvent(QEvent::MouseButtonPress,
                                             clickPos,
                                             Qt::LeftButton,
                                             Qt::MouseButton::NoButton,
                                             Qt::NoModifier);
     QCoreApplication::postEvent(eventsReciverWidget, press);
     // Some delay
     Sleep(1);
     QMouseEvent *release = new QMouseEvent(QEvent::MouseButtonRelease,
                                             clickPos,
                                             Qt::LeftButton,
                                             Qt::MouseButton::NoButton,
                                             Qt::NoModifier);
     QCoreApplication::postEvent(eventsReciverWidget, release);
     // Delay and send packet to waiting dll
     // should be better implemented
     Sleep(50);
     socket->write("alo", 4);
}

void BrowserWindow::mousePress(int x, int y){
    QMouseEvent * event1 = new QMouseEvent((QEvent::MouseButtonPress), QPoint(x, y),
        Qt::LeftButton,
        Qt::LeftButton,
        Qt::NoModifier   );

    qApp->postEvent(eventsReciverWidget, event1);
    blockMouse = true;

}

void BrowserWindow::mouseRelease(int x, int y){
    QMouseEvent * event2 = new QMouseEvent((QEvent::MouseButtonRelease), QPoint(x,y),
        Qt::LeftButton,
        Qt::LeftButton,
        Qt::NoModifier);

    qApp->postEvent(eventsReciverWidget, event2);
    blockMouse = false;

}

void BrowserWindow::mouseMove(int x, int y){
    blockMouse = false;
    QMouseEvent * event2 = new QMouseEvent((QEvent::MouseMove), QPoint(x,y),
        Qt::NoButton,
        Qt::NoButton,
        Qt::NoModifier);

    qApp->postEvent(eventsReciverWidget, event2);
}

void BrowserWindow::keyClick(int k){
    QWidget *_eventsReciverWidget = eventsReciverWidget;
    QKeyEvent *press = new QKeyEvent(QEvent::KeyPress,
                                             k,
                                             Qt::NoModifier);
     QCoreApplication::postEvent(eventsReciverWidget, press);
     // Some delay
     QTimer::singleShot(50, [_eventsReciverWidget, k]() {
         QKeyEvent *release = new QKeyEvent(QEvent::KeyRelease,
                                                 k,
                                                 Qt::NoModifier);
         QCoreApplication::postEvent(_eventsReciverWidget, release);
     });

}

void BrowserWindow::refresh(){
    browser->reload();
}

BrowserWindow::~BrowserWindow()
{
    delete ui;
}
