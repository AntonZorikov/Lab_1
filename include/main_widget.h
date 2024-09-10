#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include"function_class.h"
#include"info_message.h"
#include<unordered_map>
#include<vector>
#include<thread>

#include<QWidget>
#include<QListWidget>
#include<QTextEdit>
#include<QAction>
#include<QMenuBar>

class MainWidget : public QWidget {
public slots:
    QString directory;

    void openDirectory();

    void selectAll();

    void unselectAll();


    void createDiagram();

    void readLogs();

private:
    QListWidget *fileList;
    QTextEdit *logText = new QTextEdit();

    QAction *infoCategory = new QAction("&INFO");
    QAction *debugCategory = new QAction("&DEBUG");
    QAction *warningCategory = new QAction("&WARNING");
    QAction *errorCategory = new QAction("&ERROR");
    QAction *varCategory = new QAction("&VARIABLES");
    QAction *functionsCategory = new QAction("&FUNCTIONS");

    std::unordered_map<std::thread::id, std::vector<Function>> threadFunctions;
    std::unordered_map<std::thread::id, std::vector<InfoMessage>> threadMessage;

    QMenuBar *initMenu();

public:
    MainWidget(QWidget *parent = 0);
};

#endif // MAIN_WIDGET_H
