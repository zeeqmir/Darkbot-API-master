#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H


#include <QMainWindow>
#include <QWebEngineView>
#include <QLocalServer>
#include <QLocalSocket>
#include <QWebEnginePage>
#include <QWebEngineProfile>



namespace Ui {
class BrowserWindow;
}

class BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);
    ~BrowserWindow();
    void sendConfirm();


public slots:
    void parseCommand();
    void processNewConnection();
    void checkUrl(QUrl url);

private:
    Ui::BrowserWindow *ui;
    QWebEngineView *browser;
    QWebEnginePage *page;
    QWebEngineProfile *profile;
    QLocalSocket *socket;
    QLocalServer *server ;
    QWidget* eventsReciverWidget;
    bool blockMouse = false;
    bool sendResponse = false;

    bool eventFilter(QObject* pObject, QEvent* pEvent);

    void newConnection();
    void mouseClick(int x, int y);
    void mousePress(int x, int y);
    void mouseRelease(int x, int y);
    void mouseMove(int x, int y);
    void keyClick(int k);
    void refresh();
};

#endif // MAIN_WINDOW_H
