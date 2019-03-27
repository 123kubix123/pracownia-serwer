#include "panel_browser.h"
#include "ui_panel_browser.h"
#include <QtWebEngineWidgets>

Panel_Browser::Panel_Browser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Panel_Browser)
{
    ui->setupUi(this);
}

Panel_Browser::~Panel_Browser()
{
    delete ui;
}


void Panel_Browser::load(QString Address, QString Port)
{
    ui->Browser->load(QUrl("http://"+Address+":"+Port+"/"));
}
