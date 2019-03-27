#include "serwer.h"
#include <QApplication>
#include <QTimer>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);\
    a.setSetuidAllowed(true);
    QApplication::setSetuidAllowed(true);
    serwer w;
        QProcess process_check;
        process_check.start("/bin/bash");
        process_check.waitForStarted();
        process_check.write("(ps aux | grep pracownia-serwer | grep -v grep | wc -l) && exit || exit\n");
        process_check.waitForFinished();
        process_check.waitForReadyRead();
        QString out = process_check.readAllStandardOutput();
        process_check.close();
        out.replace("\n","");
        //qInfo()<<out;
        if(out.toInt() > 1)
        {
            //qInfo()<<"skrypcik true";
            w.create_file();
            return 0;
        }
        else
        {
        w.ui_manager();
        w.create_file();
        }
    return a.exec();
}

