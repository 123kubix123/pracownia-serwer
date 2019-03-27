#ifndef PANEL_BROWSER_H
#define PANEL_BROWSER_H

#include <QWidget>

namespace Ui {
class Panel_Browser;
}

class Panel_Browser : public QWidget
{
    Q_OBJECT

public:
    explicit Panel_Browser(QWidget *parent = 0);
    ~Panel_Browser();
    void load(QString Address, QString Port);

private:
    Ui::Panel_Browser *ui;
};

#endif // PANEL_BROWSER_H
