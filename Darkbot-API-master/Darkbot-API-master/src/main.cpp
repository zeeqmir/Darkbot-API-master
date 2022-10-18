#include "browser_window.h"
#include <QApplication>
#include <QWebEngineSettings>
#include <QWebEngineUrlScheme>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BrowserWindow w;

    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);

    w.show();

    return a.exec();
}
