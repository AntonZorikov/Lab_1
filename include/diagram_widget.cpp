#include "diagram_widget.h"
#include "diagram.h"
#include <sstream>
#include <iostream>

#include <QResizeEvent>
#include <QGridLayout>
#include <QSplitter>
#include <QMenuBar>
#include <QAction>
#include <QMenu>

void DiagramWidget::createChart(){
    QList<QListWidgetItem *> selected = fileList->selectedItems();
    if(selected.empty()){
        return;
    }
    //        for(QListWidgetItem* item : selected){
    //            files->append(item->text());
    //        }
    QString str = selected[0]->text();
    diagram->setFunctions(threadFunctions[stringThreadIdMap[selected[0]->text()]]);
    diagram->setMessages(threadMessages[stringThreadIdMap[selected[0]->text()]]);

    diagram->offsetY = 0;
//    diagram->minNestingLevel = 0;
//    diagram->maxNestingLevel = 0;
    diagram->newWidth = 0;
    diagram->oldWidth = 0;
    diagram->QX = 0;
    diagram->dragging = false;
    diagram->zoom = false;
    diagram->z = false;
    diagram->zoomBlock = false;
    diagram->scale = 1;
    diagram->center = width();
    diagram->coef = 1;
    diagram->oldCoef = 1;
    diagram->offsetX = 0;
    diagram->coefY = 1;
    diagram->minWorkTime_ = 10000000000;
    diagram->offsetMiniChart = 0;

    diagram->firstMousePositionZoom = QPoint(0, 0);
    diagram->lastMousePositionZoom = QPoint(1000, 0);
    diagram->lastMousePosition = QPoint(0, 0);


}

void DiagramWidget::resizeEvent(QResizeEvent *event){
//    int newWidth = event->size().width();
//    int oldWidth = event->oldSize().width();

//    if(newWidth > oldWidth && newWidth > 0 && oldWidth > 0){
////        std::cout << " ^ " << std::endl;
//        unsigned __int128 minTime = diagram->functions[0].startTime;
//        unsigned __int128 maxTime = diagram->functions[0].endTime - diagram->functions[0].startTime;
//        double Q_ = ((double)width() / (double)(maxTime)) * diagram->coef;


//        Function func = diagram->functions[0];
////        coef /= (double) width() / (width() + (newWidth - oldWidth));
//        double Q_o = (((double)diagram->width() + 1) / (double)(maxTime)) * diagram->coef;
//        double coefo = diagram->oldCoef * (((double)diagram->width() + 1) / (diagram->lastMousePositionZoom.x() - diagram->firstMousePositionZoom.x()));

//        int xo = (func.startTime - minTime) * Q_o - (diagram->offsetX * coefo);
//        int xn = (func.startTime - minTime) * Q_ - (diagram->offsetX * diagram->coef);
////        std::cout << std::fixed << xn - xo << " " <<  ((xn - xo) * (newWidth - oldWidth)) << std::endl;
//        diagram->QX = ((xn - xo) * (newWidth - oldWidth));
////        offsetX -= ((xn - xo) * (newWidth - oldWidth));
//        newWidth = 0;
//        oldWidth = 0;
//    }
//    if(newWidth < oldWidth && newWidth > 0 && oldWidth > 0){
////        std::cout << " ^ " << std::endl;
//        unsigned __int128 minTime = diagram->functions[0].startTime;
//        unsigned __int128 maxTime = diagram->functions[0].endTime - diagram->functions[0].startTime;
//        double Q_ = ((double)width() / (double)(maxTime)) * diagram->coef;


//        Function func = diagram->functions[0];
////        coef /= (double) width() / (width() + (newWidth - oldWidth));
//        double Q_o = (((double)diagram->width() - 1) / (double)(maxTime)) * diagram->coef;
//        double coefo = diagram->oldCoef * (((double)diagram->width() - 1) / (diagram->lastMousePositionZoom.x() - diagram->firstMousePositionZoom.x()));

//        int xo = (func.startTime - minTime) * Q_o - (diagram->offsetX * coefo);
//        int xn = (func.startTime - minTime) * Q_ - (diagram->offsetX * diagram->coef);
////        std::cout << std::fixed << xn - xo << " " <<  ((xn - xo) * (newWidth - oldWidth)) << std::endl;
//        diagram->QX = ((xo - xn) * (newWidth - oldWidth));
////        offsetX -= ((xn - xo) * (newWidth - oldWidth));
//        newWidth = 0;
//        oldWidth = 0;
//    }
////    diagram->oldWidth = oldWidth;
////    diagram->newWidth = newWidth;

    QWidget::resizeEvent(event);
}

DiagramWidget::DiagramWidget(std::unordered_map<std::thread::id, std::vector<Function> > &threadFunctions_, std::unordered_map<std::thread::id, std::vector<InfoMessage>> &threadMessages_, QWidget *parent) : QWidget(parent){
    fileList = new QListWidget();
    QGridLayout *layout = new QGridLayout(this);
    QSplitter *spliterH = new QSplitter(Qt::Horizontal);

    QAction *createDiagram = new QAction("&Create diagram");
    connect(createDiagram, &QAction::triggered, this, &DiagramWidget::createChart);

    QMenu *settings = new QMenu("&Settings");
    displayAll = new QAction("&Display all time periods");
    displayInfo = new QAction("&Display all info messages");
    displayInfoPartially = new QAction("&Display info messages partially");

    QActionGroup *group1 = new QActionGroup(this);
    group1->setExclusive(true);

    displayInfo->setActionGroup(group1);
    displayInfoPartially->setActionGroup(group1);

    settings->addAction(displayAll);
    settings->addSeparator();
    settings->addAction(displayInfo);
    settings->addAction(displayInfoPartially);
    displayAll->setCheckable(true);
    displayInfo->setCheckable(true);
    displayInfoPartially->setCheckable(true);

    QMenuBar *menuBar = new QMenuBar(this);
    menuBar->addAction(createDiagram);
    menuBar->addMenu(settings);
    layout->setMenuBar(menuBar);


    threadFunctions = threadFunctions_;
    threadMessages = threadMessages_;
    diagram = new Diagram(this);

    QStringList files;

    for(const auto &pair : threadFunctions_){
        std::stringstream ss;
        ss << pair.first;
        files << QString::fromStdString(ss.str());
        stringThreadIdMap[QString::fromStdString(ss.str())] = pair.first;
    }

    std::sort(files.begin(), files.end());
    fileList->addItems(files);
    fileList->setSelectionMode(QAbstractItemView::SingleSelection);

    //        spliterV->addWidget(displayAllCheckBox);

    QList<int> size;
    size << 100 << 400;

    fileList->setFixedWidth(200);

    spliterH->addWidget(fileList);
    spliterH->addWidget(diagram);
    spliterH->setSizes(size);

    layout->addWidget(spliterH);
    layout->setSpacing(0);
    layout->setMargin(0);

    setLayout(layout);
    setFixedSize(1366, 768);
}
