#ifndef DIAGRAM_WIDGET_H
#define DIAGRAM_WIDGET_H

#include<unordered_map>
#include<function_class.h>
#include<info_message.h>
#include<diagram.h>
#include<thread>
#include<QListWidget>

class Diagram;

class DiagramWidget : public QWidget {
private:
    std::unordered_map<std::thread::id, std::vector<Function>> threadFunctions;
    std::unordered_map<std::thread::id, std::vector<InfoMessage>> threadMessages;
    std::unordered_map<QString, std::thread::id> stringThreadIdMap;
    QListWidget *fileList;
    Diagram *diagram;

    void createChart();

public:
    QAction *displayAll, *displayInfo, *displayInfoPartially;

    void resizeEvent(QResizeEvent *event) override;

    DiagramWidget(std::unordered_map<std::thread::id, std::vector<Function>> &threadFunctions_, std::unordered_map<std::thread::id, std::vector<InfoMessage>> &threadMessages_, QWidget *parent = 0);
};

#endif // DIAGRAM_WIDGET_H
