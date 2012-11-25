#include <QApplication>
#include <QThreadPool>
#include <sys/resource.h>
#include "maindialog.h"

int main(int argc, char** argv)
{
    rlimit r;
    if (getrlimit(RLIMIT_NOFILE, &r) >= 0)
    {
        cout << "rlimit before:" << r.rlim_cur << '\t' << r.rlim_max << endl;
        r.rlim_cur = 1024;
        setrlimit(RLIMIT_NOFILE, &r);
        cout << "rlimit after:" << r.rlim_cur << '\t' << r.rlim_max << endl;
    }
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    QApplication app(argc, argv);
    srand((unsigned int)time(0));
    QThreadPool::globalInstance()->setMaxThreadCount(1024);
    MainDialog d;
    d.show();
    return app.exec();
}
