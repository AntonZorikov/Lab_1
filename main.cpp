#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QSplitter>
#include <QGridLayout>

#include <QPushButton>
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QTreeWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QScrollArea>
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>
#include <QKeyEvent>
#include <QCheckBox>
#include <QGroupBox>

#include "include/logger.h"
#include "include/diagram_widget.h"
#include "include/main_widget.h"
#include "variant.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <sstream>
#include <set>
#include <unordered_map>

Logger logger;

int main(int argc, char *argv[])
{
    //Zorikov
    //Dryakin

    QApplication a(argc, argv);
    MainWidget w;
    w.move(100, 100);
    w.show();
    //log();

    return a.exec();
}
