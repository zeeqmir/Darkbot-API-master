#ifndef BROWSER_H
#define BROWSER_H

#include <QObject>
#include <QWidget>
#include <QWebEngineView>

class Browser : public QWebEngineView
{
    Q_OBJECT
public:
    explicit Browser(QWidget *parent = nullptr);
	virtual ~Browser();

signals:

public slots:
};

#endif // BROWSER_H
