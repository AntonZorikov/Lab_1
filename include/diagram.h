#ifndef DIAGRAM_H
#define DIAGRAM_H

#include <QApplication>
#include <QWidget>

#include<function_class.h>
#include<info_message.h>
#include<diagram_widget.h>
#include<vector>
#include<unordered_map>

class DiagramWidget;

class Diagram : public QWidget {
public:
    struct Pos{
        int x, y, h, w;
        std::string mes;

        Pos(int x_, int y_, int h_, int w_, std::string mes_);
    };

    std::vector<Function> functions;
    std::vector<InfoMessage> messages;
    std::vector<Function> functionsBase;
    std::vector<InfoMessage> messagesBase;
    std::vector<Pos> pos;
    int  offsetY = 0, minNestingLevel = 0, maxNestingLevel = 0, newWidth = 0, oldWidth = 0, QX = 0;
    bool dragging = false, zoom = false, z = false, zoomBlock = false;
    QPoint lastMousePosition, firstMousePositionZoom, lastMousePositionZoom, lastMousePositionZoomStatic, firstMousePositionZoomStatic;
    double scale = 1, center = width(), coef = 1, oldCoef = 1, offsetX = 0, coefY = 1, offsetMiniChart = 0;
    unsigned __int128 minFunctionWorkTime, maxFunctionWorkTime, minTime, maxTime;
    double minWorkTime_ = 10000000000;
    DiagramWidget *diagramWidget;

public:
    void setFunctions(std::vector<Function> functions_);

    void setMessages(std::vector<InfoMessage> messages_);

    Diagram(std::vector<Function> &functions_);

    Diagram(DiagramWidget *dw);

    Diagram();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
};

#endif // DIAGRAM_H
