#include "main_widget.h"

#include "diagram_widget.h"
#include "logger.h"
#include <iostream>

#include <QFileDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QString>
#include <QDir>

void MainWidget::openDirectory(){
    QString dir = QFileDialog::getExistingDirectory(nullptr, "Выберите директорию", "", QFileDialog::ShowDirsOnly);

    QDir qdir(dir);
    QStringList files = qdir.entryList(QDir::Files);

    std::sort(files.begin(), files.end());
    fileList->clear();
    fileList->addItems(files);

    directory = dir;
}

void MainWidget::selectAll(){
    for(int i = 0; i < fileList->count(); i++){
        fileList->item(i)->setSelected(true);
    }
}

void MainWidget::unselectAll(){
    for(int i = 0; i < fileList->count(); i++){
        fileList->item(i)->setSelected(false);
    }
}

void MainWidget::createDiagram() {
    DiagramWidget *diagram = new DiagramWidget(threadFunctions, threadMessage);
    diagram->move(100, 100);
    diagram->show();
}

void MainWidget::readLogs(){
    threadFunctions.clear();

    QString text = "";
    if(directory.isEmpty()){
        text = "Directory not selected";
        logText->setText(text);
        return;
    }

    QStringList *files = new QStringList();
    //std::cout << files.size() << " " << std::endl;

    QList<QListWidgetItem*> selected = fileList->selectedItems();
    for(QListWidgetItem* item : selected){
        files->append(item->text());
    }

    //std::map<std::thread::id, std::map<std::string, double>> threadData;

    std::unordered_map<std::string, unsigned __int128> workingFunctions;
    std::unordered_map<std::thread::id, int> nesting;

    if(files->empty()){
        logText->setText(text);
        return;
    }

    int st = 0, en = 0;

    std::sort(files->begin(), files->end());
    //        auto sta = std::chrono::high_resolution_clock::now();
    int item = 0;
    for(auto fp : *(files)){
        item++;
        std::cout << 100.0 / files->size() * item << std::endl;
        //            auto sta = std::chrono::high_resolution_clock::now();

        std::unordered_map<int, std::string> dict;
        std::vector<Log> recs;

        try{
            Logger::readLog(directory.toUtf8().constData(), fp, dict, recs);
        }
        catch(std::exception e){
            logText->setText("Invalid file format");
            return;
        }

        text += "=============== LOG FILE ================ \n";
        text += ("                  " + fp + "\n").toUtf8().constData();

        text += "============== DICTIONARY ============== \n";
        for(const auto &pair : dict){
            //std::cout << pair.first << " " << pair.second << std::endl;
            text += QString::number(pair.first) + " | " + QString::fromStdString(pair.second) + '\n';
        }

        text += "============== LOGS ============== \n";
        for(const auto &log : recs){

            unsigned __int128 rem_ns;
            std::tm tm = Logger::ns_to_tm(log.timestamp, rem_ns);
            std::string form_time = Logger::format_tm(tm, rem_ns);

//            std::cout << log.valueType << std::endl;

            if(log.valueType == 0){
                if((log.logType == 0 && infoCategory->isChecked()) || (log.logType == 1 && warningCategory->isChecked()) ||
                        (log.logType == 2 && debugCategory->isChecked()) || (log.logType == 3 && errorCategory->isChecked())){
                    std::stringstream ss;
                    ss << log.threadId;
                    text += QString::fromStdString(form_time) + "   " + QString::fromStdString(Logger::logLevelToString((LogLevel) log.logType)) + "   " + QString::fromStdString(dict[log.messageCode]) +  "   "  + QString::fromStdString(ss.str()) + '\n';
                    if(!nesting[log.threadId]){
                        nesting[log.threadId] = 0;
                    }
                    InfoMessage message(log.logType, log.timestamp, dict[log.messageCode], nesting[log.threadId], log.threadId);
                    threadMessage[log.threadId].push_back(message);
                }
            }
            else if(log.valueType == 1 && varCategory->isChecked()){
                std::stringstream ss;
                ss << log.threadId;
                text += QString::fromStdString(form_time) + "   " + QString::number(mpark::get<int>(log.value)) + '\n';
            }
            else if(log.valueType == 2 && varCategory->isChecked()){
                std::stringstream ss;
                ss << log.threadId;
                text += QString::fromStdString(form_time) + "   " + QString::number(mpark::get<float>(log.value)) + '\n';
            }
            else if(log.valueType == 3 && varCategory->isChecked()){
                std::stringstream ss;
                ss << log.threadId;
                text += QString::fromStdString(form_time) + "   " + QString::number(mpark::get<long>(log.value)) + '\n';
            }
            if(log.valueType == 4 && functionsCategory->isChecked()){
                std::stringstream ss;
                ss << log.threadId;
                text += QString::fromStdString(form_time) + "   " + QString::fromStdString(Logger::functionStateToString((FunctionState) log.logType)) + "   " + QString::fromStdString(dict[log.messageCode]) + "   " + QString::fromStdString(ss.str()) + '\n';
            }

            if(log.valueType == 4){
                if(!nesting[log.threadId]){
                    nesting[log.threadId] = 0;
                }

                if(!nesting[log.threadId]){
                    nesting[log.threadId] = 0;
                }

                if(log.logType == START_FUNCTION){
                    st++;
                    nesting[log.threadId]++;
                }

                std::string functionName = dict[log.messageCode] + std::to_string(nesting[log.threadId]);

                if(workingFunctions[functionName]){
                    Function func(dict[log.messageCode], nesting[log.threadId], workingFunctions[functionName], log.timestamp);
                    threadFunctions[log.threadId].push_back(func);
                    workingFunctions.erase(functionName);
                }
                else{
                    workingFunctions[functionName] = log.timestamp;
                }

                if(log.logType == END_FUNCTION){
                    en++;
                    nesting[log.threadId]--;
                }
            }



        }
        //            auto end = std::chrono::high_resolution_clock::now();
        //            std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - sta).count() << std::endl;
    }
    //        auto end = std::chrono::high_resolution_clock::now();
    //        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - sta).count() << std::endl;

    logText->setText(text);
}

QMenuBar *MainWidget::initMenu(){
    QMenu *selectCategories = new QMenu("&Select Categories");

    QAction *openDirectory = new QAction("&Open Directory");
    QAction *selectAll = new QAction("Select All");
    QAction *unselectAll = new QAction("Unselect All");
    QAction *read = new QAction("Read");

    connect(openDirectory, &QAction::triggered, this, &MainWidget::openDirectory);
    connect(selectAll, &QAction::triggered, this, &MainWidget::selectAll);
    connect(unselectAll, &QAction::triggered, this, &MainWidget::unselectAll);
    connect(read, &QAction::triggered, this, &MainWidget::readLogs);

    infoCategory->setCheckable(true);
    debugCategory->setCheckable(true);
    warningCategory->setCheckable(true);
    errorCategory->setCheckable(true);
    varCategory->setCheckable(true);
    functionsCategory->setCheckable(true);

    infoCategory->setChecked(true);
    debugCategory->setChecked(true);
    warningCategory->setChecked(true);
    errorCategory->setChecked(true);
    varCategory->setChecked(true);
    functionsCategory->setChecked(true);

    selectCategories->addAction(infoCategory);
    selectCategories->addAction(debugCategory);
    selectCategories->addAction(warningCategory);
    selectCategories->addAction(errorCategory);
    selectCategories->addAction(varCategory);
    selectCategories->addAction(functionsCategory);

    QMenuBar *menuBar = new QMenuBar(this);
    menuBar->addAction(openDirectory);
    menuBar->addMenu(selectCategories);
    menuBar->addAction(selectAll);
    menuBar->addAction(unselectAll);
    menuBar->addAction(read);
    menuBar->show();

    return menuBar;
}

MainWidget::MainWidget(QWidget *parent) : QWidget(parent){
    fileList = new QListWidget();
    fileList->setSelectionMode(QAbstractItemView::MultiSelection);

    QMenuBar *menuBar = initMenu();

    QGridLayout *layout = new QGridLayout(this);
    QSplitter *spliterV = new QSplitter(Qt::Vertical);
    QSplitter *spliterH = new QSplitter(Qt::Horizontal);
    QPushButton *button = new QPushButton("Create Chart");
    button->setFixedHeight(50);

    QGridLayout *layout_ = new QGridLayout(this);

    QScrollArea *scrollArea = new QScrollArea();
    logText->setReadOnly(true);
    scrollArea->setWidget(logText);
    scrollArea->setWidgetResizable(true);

    layout_->addWidget(fileList, 0, 0);
    layout_->addWidget(button, 0, 1);

    spliterV->addWidget(fileList);
    spliterV->addWidget(button);

    QList<int> size;
    size << 100 << 400;

    spliterH->addWidget(spliterV);
    spliterH->addWidget(scrollArea);
    spliterH->setSizes(size);

    layout->addWidget(spliterH);
    layout->setMenuBar(menuBar);
    layout->setSpacing(0);
    layout->setMargin(0);

    connect(button, &QPushButton::clicked, this, &MainWidget::createDiagram);

    setLayout(layout);
    resize(900, 400);
}
