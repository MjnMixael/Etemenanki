/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "Etemenanki.h"
#include <QSharedMemory>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSharedMemory sharedMemory;
    sharedMemory.setKey("EtemenankiRunning");

    if (!sharedMemory.create(1)) {
        // Another instance is already running so end here
        return 1;
    }

    Etemenanki w;
    w.show();
    return a.exec();
}
