#include <QApplication>
#include <QThreadPool>
#include <QDir>
#ifdef Q_OS_UNIX
#include <sys/resource.h>
#endif
#include "maindialog.h"

int main(int argc, char** argv)
{
#ifdef Q_OS_UNIX
    rlimit r;
    if (getrlimit(RLIMIT_NOFILE, &r) >= 0)
    {
        cout << "rlimit before:" << r.rlim_cur << '\t' << r.rlim_max << endl;
        r.rlim_cur = 1024;
        setrlimit(RLIMIT_NOFILE, &r);
        cout << "rlimit after:" << r.rlim_cur << '\t' << r.rlim_max << endl;
    }
    if (getrlimit(RLIMIT_STACK, &r) >= 0)
    {
        cout << "rlimit before:" << r.rlim_cur << '\t' << r.rlim_max << endl;
        r.rlim_cur = 1024;
        setrlimit(RLIMIT_NOFILE, &r);
        cout << "rlimit after:" << r.rlim_cur << '\t' << r.rlim_max << endl;
    }
#endif
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    QApplication app(argc, argv);
    srand((unsigned int)time(0));
#ifdef SEEK_TO_SAME_POS
    DecodeThread::setSeekPos(rand());
#endif
#if defined(Q_OS_MAC) && defined(FIX_MAC109_QT51)
    QDir appDir(app.applicationDirPath());
    appDir.cdUp();appDir.cdUp();appDir.cdUp();
    QDir::setCurrent(appDir.absolutePath());
#endif
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    QApplication::setStyle("plastique");
#endif
    QThreadPool::globalInstance()->setMaxThreadCount(1024);
    MainDialog d;
    d.show();
    return app.exec();
}
