#include "diagram.h"
#include "diagram_widget.h"
#include "logger.h"
#include <cmath>
#include <iostream>

#include<QMouseEvent>
#include<QToolTip>
#include<QPainter>
#include<QAction>

Diagram::Pos::Pos(int x_, int y_, int h_, int w_, std::string mes_){
    this->x = x_;
    this->y = y_;
    this->h = h_;
    this->w = w_;
    this->mes = mes_;
}

void Diagram::setFunctions(std::vector<Function> functions_){
    this->functions = functions_;
    this->functionsBase = functions_;

    lastMousePositionZoomStatic.setX(width());

    minNestingLevel = functions[0].nestingLevel;
    maxNestingLevel = functions[0].nestingLevel;

    unsigned __int128 minStartTime = std::numeric_limits<unsigned __int128>::max(), maxEndTime = 0;

    for(const Function &func : functions){
        if(func.nestingLevel < minNestingLevel)
            minNestingLevel = func.nestingLevel;

        if(func.nestingLevel > maxNestingLevel)
            maxNestingLevel = func.nestingLevel;

        if(func.startTime < minStartTime)
            minStartTime = func.startTime;

        if(func.endTime > maxEndTime)
            maxEndTime = func.endTime;
    }

    Function func("Thread run", 0, minStartTime, maxEndTime);
    functions.push_back(func);
    functionsBase.push_back(func);

    update();
}

void Diagram::setMessages(std::vector<InfoMessage> messages_){
    this->messages = messages_;
    this->messagesBase = messages_;

    update();
}

Diagram::Diagram(std::vector<Function> &functions_){
    for(Function func : functions_)
        this->functions.push_back(func);

    for(Function func : functions_)
        this->functionsBase.push_back(func);
}

Diagram::Diagram(){}

Diagram::Diagram(DiagramWidget *dw){
    diagramWidget = dw;
}

void Diagram::mouseMoveEvent(QMouseEvent *event){
    if(zoom){
        lastMousePositionZoom = event->pos();

        repaint();
    }
    if(dragging){
        if(lastMousePosition.x() <= 0){
            lastMousePosition.setX(event->x());
        }

        int dx = (event->x() - lastMousePosition.x()) / scale;

        offsetMiniChart += dx;
//        std::cout << offsetMiniChart << " " << event->x() << " " << lastMousePosition.x() << std::endl;

        lastMousePosition = event->pos();

        if(lastMousePositionZoom.x() + offsetMiniChart > width()){
            offsetMiniChart = width() - lastMousePositionZoom.x();
        }
        if(firstMousePositionZoom.x() + offsetMiniChart < 0){
            offsetMiniChart = -firstMousePositionZoom.x();
        }

        update();
    }
}

void Diagram::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton){
        if(event->pos().y() < height() - 50){
            zoom = true;
            lastMousePositionZoom = event->pos();
    //        lastMousePositionZoomStatic = lastMousePositionZoom;
            firstMousePositionZoom = event->pos();
        }
        else{
            dragging = true;
        }
    }
    if(event->button() == Qt::RightButton){
        bool f = false;
        QPointF scaledPos = QPointF(event->x(), event->y()) / scale;
        for(Pos p : pos){
            if(scaledPos.x() >= p.x && scaledPos.x() <= p.x + p.w && scaledPos.y() >= p.y && scaledPos.y() <= p.y + p.h){
                QToolTip::showText(event->globalPos(), QString::fromStdString(p.mes));
                f = true;
            }
        }
        if(!f){
            QToolTip::hideText();
        }
    }
}

void Diagram::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton && dragging){
        dragging = false;
        lastMousePosition.setX(0);

        lastMousePositionZoom.setX(lastMousePositionZoom.x() + offsetMiniChart);
        firstMousePositionZoom.setX(firstMousePositionZoom.x() + offsetMiniChart);

        functions.clear();
        for(auto func : functionsBase){
            functions.push_back(func);
        }

        std::sort(functions.begin(), functions.end(), [=](const Function &a, const Function &b){
            return a.startTime < b.startTime;
        });

        unsigned __int128 minTime = functions[0].startTime;
        maxTime = functions[0].endTime - functions[0].startTime;
        double Q_ = ((double)width() / (double)(maxTime)) * (coef * ((double) width() / (lastMousePositionZoom.x() - firstMousePositionZoom.x())));

        minFunctionWorkTime = functions[0].endTime - functions[0].startTime;

        for(const Function &func : functions){
            if(func.endTime - func.startTime < minFunctionWorkTime)
                minFunctionWorkTime = func.endTime - func.startTime;
        }

        int count = 0;
        for(Function func : functions){
            qint64 x1 = (func.startTime - minTime) * Q_ - (offsetX * coef);
            qint64 x2 = (func.startTime - minTime) * Q_ - (offsetX * coef) + (func.endTime - func.startTime) * Q_;
            if((((x1 <= width() && x1 >= 0) || (x2 >= 0 && x2 <= width()) || (x1 <= 0 && x2 >= width())))){
                count++;
            }
        }

        if((width() / Q_) < 1000 || functions.size() - count <= 1){
            return;
        }

        if((width() / Q_) < 1000 || lastMousePositionZoom.x() == firstMousePositionZoom.x() || functions.size() - count <= 1){
            z = false;
            return;
        }

        if(firstMousePositionZoom.x() < lastMousePositionZoom.x() && firstMousePositionZoom.y() < lastMousePositionZoom.y()){
            offsetX += offsetMiniChart;
            std::cout << (firstMousePositionZoom.x()) / coef << " " << offsetMiniChart << std::endl;
        }

        maxTime = functions[0].endTime - functions[0].startTime;
        minTime = functions[0].startTime;

        Q_ = (((double)width()) / ((double)(maxTime)));

        double st = firstMousePositionZoom.x() / (Q_ * coef) + minTime;
        double en = lastMousePositionZoom.x() / (Q_ * coef) + minTime;

        Q_ = ((double)width() / (double)(maxTime)) * coef;

        functions.erase(std::remove_if(functions.begin(), functions.end(), [st, en, minTime, Q_, this](const Function func) {
            qint64 x1 = (func.startTime - minTime) * Q_ - (offsetX * coef);
            qint64 x2 = (func.startTime - minTime) * Q_ - (offsetX * coef) + (func.endTime - func.startTime) * Q_;
            return !(((x1 <= width() && x1 >= 0) || (x2 >= 0 && x2 <= width()) || (x1 <= 0 && x2 >= width())));
        }), functions.end());

        offsetMiniChart = 0;

        repaint();
    }
    else if(event->button() == Qt::LeftButton && !dragging){
        if(zoomBlock){
            return;
        }

        lastMousePositionZoomStatic = lastMousePositionZoom;
        firstMousePositionZoomStatic = firstMousePositionZoom;

        zoom = false;
        z = true;
        repaint();

        std::sort(functions.begin(), functions.end(), [=](const Function &a, const Function &b){
            return a.startTime < b.startTime;
        });

        unsigned __int128 minTime = functions[0].startTime;
        maxTime = functions[0].endTime - functions[0].startTime;
        double Q_ = ((double)width() / (double)(maxTime)) * (coef * ((double) width() / (lastMousePositionZoom.x() - firstMousePositionZoom.x())));

        minFunctionWorkTime = functions[0].endTime - functions[0].startTime;

        for(const Function &func : functions){
            if(func.endTime - func.startTime < minFunctionWorkTime)
                minFunctionWorkTime = func.endTime - func.startTime;
        }

        int count = 0;
        for(Function func : functions){
            qint64 x1 = (func.startTime - minTime) * Q_ - (offsetX * coef);
            qint64 x2 = (func.startTime - minTime) * Q_ - (offsetX * coef) + (func.endTime - func.startTime) * Q_;
            if((((x1 <= width() && x1 >= 0) || (x2 >= 0 && x2 <= width()) || (x1 <= 0 && x2 >= width())))){
                count++;
            }
        }

        if((width() / Q_) < 1000 || functions.size() - count <= 1){
            return;
        }

        if((width() / Q_) < 1000 || lastMousePositionZoom.x() == firstMousePositionZoom.x() || functions.size() - count <= 1){
            z = false;
            return;
        }

        if(firstMousePositionZoom.x() < lastMousePositionZoom.x() && firstMousePositionZoom.y() < lastMousePositionZoom.y()){
            offsetX += (firstMousePositionZoom.x() + offsetMiniChart) / coef;
            center = lastMousePositionZoom.x() - firstMousePositionZoom.x();
        }

        maxTime = functions[0].endTime - functions[0].startTime;
        minTime = functions[0].startTime;

        Q_ = (((double)width()) / ((double)(maxTime)));

        double st = firstMousePositionZoom.x() / (Q_ * coef) + minTime;
        double en = lastMousePositionZoom.x() / (Q_ * coef) + minTime;

        oldCoef = coef;
        coef *= (double) width() / (lastMousePositionZoom.x() - firstMousePositionZoom.x());
        z = false;
        Q_ = ((double)width() / (double)(maxTime)) * coef;

        functions.erase(std::remove_if(functions.begin(), functions.end(), [st, en, minTime, Q_, this](const Function func) {
            qint64 x1 = ((func.startTime - minTime) * Q_ - (offsetX * coef));
            qint64 x2 = ((func.startTime - minTime) * Q_ - (offsetX * coef)) + ((func.endTime - func.startTime) * Q_);
            return x1 > width() || x2 < 0;
        }), functions.end());

        repaint();
    }
}

void Diagram::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_W:
        offsetY += 10 / scale;
        break;
    case Qt::Key_S:
        offsetY -= 10 / scale;
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
    update();
}

void Diagram::wheelEvent(QWheelEvent *event){
    if(event->angleDelta().y() > 0){
        coefY *= 1.1;
        offsetY *= 1.1;
    }
    else{
        coefY /= 1.1;
        offsetY /= 1.1;
    }
    update();
}

void Diagram::paintEvent(QPaintEvent *event){
    if(functions.empty()){
        return;
    }

    maxTime = functions[functions.size() - 1].endTime - functions[0].startTime;
    unsigned __int128 minTime = functions[0].startTime;

    //        if((func.endTime > lastMousePositionZoom.x() / Q_ && func.startTime < lastMousePositionZoom.x() / Q_) || (func.startTime < firstMousePositionZoom.x() / Q_ && func.endTime > firstMousePositionZoom.x() / Q_))
    double Q_ = (((double)width()) / ((double)(maxTime)));

    std::sort(functions.begin(), functions.end(), [=](const Function &a, const Function &b){
        return a.startTime < b.startTime;
    });

    setFocus();

    minTime = functions[0].startTime;
    maxTime = functions[0].endTime - functions[0].startTime;
    //        if(!zoom)
    //            maxTime = en - st > 0 ? en - st : maxTime;

    minFunctionWorkTime = functions[0].endTime - functions[0].startTime;
    maxFunctionWorkTime = functions[0].endTime - functions[0].startTime;

    for(const Function &func : functions){
        if(func.endTime - func.startTime < minFunctionWorkTime)
            minFunctionWorkTime = func.endTime - func.startTime;

        if(func.endTime - func.startTime > maxFunctionWorkTime)
            maxFunctionWorkTime = func.endTime - func.startTime;

        if(func.startTime < minTime)
            minTime = func.startTime;
    }

    for(const Function &func : functions){
        unsigned __int128 time = func.endTime - func.startTime;
        double workTime = static_cast<double>(static_cast<double>(static_cast<int>(time - minFunctionWorkTime)) / static_cast<double>(static_cast<int>(maxFunctionWorkTime - minFunctionWorkTime)));
        if(workTime < minWorkTime_ && workTime > 0){
            minWorkTime_ = workTime;
        }
    }


    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(scale, scale);
    painter.setBrush(Qt::green);

    unsigned __int128 minStartTime = std::numeric_limits<unsigned __int128>::max(), maxEndTime = 0;

    for(const Function &func : functions){
        if(func.startTime < minStartTime)
            minStartTime = func.startTime;

        if(func.endTime > maxEndTime)
            maxEndTime = func.endTime;
    }

    double nestingQ = 200 / ((maxNestingLevel - minNestingLevel) + 3);
    //    Q_ = ((double)width() / (double)(maxEndTime - minStartTime)) * coef;
    Q_ = ((double)width() / (double)(maxTime)) * coef;
    pos.clear();
    int item = 0;
    int HEIGHT = std::ceil(600.0 / maxNestingLevel) * coefY > 7 ? std::ceil(600.0 / maxNestingLevel) * coefY : 7;

    std::unordered_map<int, std::vector<std::pair<unsigned __int128, unsigned __int128>>> visibleFunctions;

    for(const Function &func : functions){
        qint64 x1 = ((func.startTime - minTime) * Q_ - (offsetX * coef));
        qint64 x2 = ((func.startTime - minTime) * Q_ - (offsetX * coef)) + (func.endTime - func.startTime) * Q_;
        //            std::cout << (func.endTime - func.startTime) * Q_ << std::endl;
//        std::cout << x1 << " " << x2 << " " << std::fixed << coef << std::endl;
//        std::cout << " _ " << func.nestingLevel << " " << x1 << " " << x2 << " " << std::numeric_limits<qint64>::max() << std::endl;
        if(func.nestingLevel > 0 && ((x1 <= width() && x1 >= 0) || (x2 >= 0 && x2 <= width()) || (x1 <= 0 && x2 >= width())) && (diagramWidget->displayAll->isChecked() || (func.endTime - func.startTime) * Q_ > 3)){
            item++;

            //            std::cout << "Paint Event: " << (100.0 / functions.size()) * item << "%" << std::endl;
//            std::cout << (func.startTime - minTime) * Q_ - (offsetX * coef) << " " << HEIGHT * func.nestingLevel + offsetY << " " << (func.endTime - func.startTime) * Q_ << std::endl;

            unsigned __int128 time = func.endTime - func.startTime;

            visibleFunctions[func.nestingLevel].push_back(std::make_pair(func.startTime, func.endTime));

            //                QColor color(100 + func.startTime % 70, 30 + nestingQ * std::abs(func.nestingLevel), 30 + nestingQ * std::abs(func.nestingLevel));
            QColor color(100 + func.startTime % 70, 255.0 / maxFunctionWorkTime * (func.endTime - func.startTime), 30 + nestingQ * std::abs(func.nestingLevel));
            painter.setBrush(color);

            unsigned __int128 rem_ns;
            std::tm tm = Logger::ns_to_tm(func.startTime, rem_ns);
            std::string st_time = Logger::format_tm(tm, rem_ns);

            tm = Logger::ns_to_tm(func.endTime, rem_ns);
            std::string end_time = Logger::format_tm(tm, rem_ns);


            //            painter.drawRect((func.startTime - minTime) * Q_ - (offsetX * coef), HEIGHT * func.nestingLevel + offsetY, diagramWidget->displayAll->isChecked() ? std::ceil((func.endTime - func.startTime) * Q_) : (func.endTime - func.startTime) * Q_, HEIGHT);


            qint64 x = ((func.startTime - minTime) * Q_ - (offsetX * coef));
            qint64 w = diagramWidget->displayAll->isChecked() ? std::ceil((func.endTime - func.startTime) * Q_) : (func.endTime - func.startTime) * Q_;

//            std::cout << x << " " << w << " " << std::numeric_limits<int>::max() << std::endl;

            if(x < 0){
                w += x;
                x = 0;
            }
            if(x + w > 10000){
                w = 10000;
            }

            Pos pos_(x, HEIGHT * (func.nestingLevel - 1) + offsetY + 50, HEIGHT,  w, func.name + "\n" + "Start time: " + st_time + "ns" + "\n" +
                     "End time:   " + end_time + "ns" + "\n" +
                     "Time:          " + Logger::unsigned_int128_ToString(time) + "ns" + '\n' + "NL: " + std::to_string(func.nestingLevel));
            //                                                                                                                                   "Width: " + std::to_string((func.endTime - func.startTime) * Q_) + " " + logger.unsigned_int128_ToString(minTime) + " " + logger.unsigned_int128_ToString(func.startTime) + " " + logger.unsigned_int128_ToString(func.endTime) + " "  + std::to_string((func.startTime - minTime) * Q_));
            pos.push_back(pos_);
//            std::cout << x << " ^ " << w << " " << std::numeric_limits<int>::max() << " " << func.nestingLevel << std::endl;
//            std::cout << re.x() << " " << re.y() << " " << re.width() << " " << re.height() << std::endl;

//            if(((func.startTime - minTime) * Q_ - (offsetX * coef)) < std::numeric_limits<int>::min())

                painter.drawRect(x, HEIGHT * (func.nestingLevel - 1) + offsetY + 50, w, HEIGHT);
//            }
//            else{
//////                QRect rect(0, HEIGHT * func.nestingLevel + offsetY, ((func.startTime - minTime) * Q_ - (offsetX * coef)) + (func.endTime - func.startTime) * Q_, HEIGHT);
//////                std::cout << rect.x() << " " << rect.width() << " " << functions.size() << std::endl;
////////                std::cout << (func.startTime - minTime) * Q_ - (offsetX * coef) << std::endl;
//                unsigned __int128 w = ((func.startTime - minTime) * Q_ - (offsetX * coef)) + (func.endTime - func.startTime) * Q_;
//                if(w > 10000){
//                    w = 10000;
//                }
//                painter.drawRect(0, HEIGHT * (func.nestingLevel - 1) + offsetY + 50, w, HEIGHT);
//            }

            QFontMetrics fm(painter.font());
            int textWidth = fm.width(QString::fromStdString(func.name));

            if(textWidth < (func.endTime - func.startTime) * Q_ && fm.ascent() < HEIGHT){
                painter.setPen(Qt::black);

                if(x1 < 0 && x2 < width() && x2 > 0 && textWidth < x2 && x2 / 2 + textWidth < x2)
                    painter.drawText(x2 / 2, HEIGHT * (func.nestingLevel - 1) + offsetY + 50 + fm.ascent() + HEIGHT / 2 - fm.height() / 2, QString::fromStdString(func.name));
                else if(x1 > 0 && x1 < width() && x2 > width() && x1 + ((width() - x1) / 2) + textWidth < width())
                    painter.drawText(x1 + ((width() - x1) / 2), HEIGHT * (func.nestingLevel - 1) + offsetY + 50 + fm.ascent() + HEIGHT / 2 - fm.height() / 2, QString::fromStdString(func.name));
                else if(x1 < 0 && x2 > width())
                    painter.drawText(width() / 2, HEIGHT * (func.nestingLevel - 1) + offsetY + 50 + fm.ascent() + HEIGHT / 2 - fm.height() / 2, QString::fromStdString(func.name));
                else if(x1 < width() && x1 > 0 && x2 < width() && x2 > 0 && ((func.startTime - minTime) * Q_ - (offsetX * coef)) + ((func.endTime - func.startTime) * Q_) / 2 - textWidth > x1)
                    painter.drawText(((func.startTime - minTime) * Q_ - (offsetX * coef)) + ((func.endTime - func.startTime) * Q_) / 2 - textWidth, HEIGHT * (func.nestingLevel - 1) + offsetY + 50 + fm.ascent() + HEIGHT / 2 - fm.height() / 2, QString::fromStdString(func.name));

            }
        }
    }

    QPen pen(Qt::black, 2);

    painter.setBrush(Qt::NoBrush);
    painter.setPen(pen);

    if(width() > 150){
        painter.drawLine(2, 0, 2, 25);
        painter.drawLine(width() - 3, 0, width() - 3, 25);
        painter.drawLine(2, 25 / 2, width() - 3, 25 / 2);

        painter.drawText(width() / 2 - 30, 25 / 2 + 50 - 30, QString::fromStdString(Logger::unsigned_int128_ToString((width() / Q_)) + "ns"));
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(pen);

    painter.drawLine(0, height() - 50, width(), height() - 50);

    painter.setPen(Qt::NoPen);

    HEIGHT = std::ceil(50 / maxNestingLevel);

    std::sort(functionsBase.begin(), functionsBase.end(), [=](const Function &a, const Function &b){
        return a.startTime < b.startTime;
    });

    setFocus();

    maxTime = functionsBase[0].endTime - functionsBase[0].startTime;

    for(const Function &func : functionsBase){
        if(func.startTime < minStartTime)
            minStartTime = func.startTime;

        if(func.endTime > maxEndTime)
            maxEndTime = func.endTime;
    }

    double Q = ((double)width() / (double)(maxEndTime - minStartTime));

    for(const Function &func : functionsBase){
        qint64 x = ((func.startTime - minTime) * Q);
        qint64 w = diagramWidget->displayAll->isChecked() ? std::ceil((func.endTime - func.startTime) * Q) : (func.endTime - func.startTime) * Q;

        QColor color(100 + func.startTime % 70, 255.0 / maxFunctionWorkTime * (func.endTime - func.startTime), 30 + nestingQ * std::abs(func.nestingLevel));
        painter.setBrush(color);

        if(x < 0){
            w += x;
            x = 0;
        }
        if(x + w > 10000){
            w = 10000;
        }

        painter.drawRect(x, HEIGHT * (func.nestingLevel - 1) + height() - 50, w, HEIGHT);
    }

    painter.setPen(Qt::black);

    double st = firstMousePositionZoom.x() / (Q_ * coef) + minTime;
    double en = lastMousePositionZoom.x() / (Q_ * coef) + minTime;

    QColor color(0, 0, 0, 30);
    painter.setBrush(color);

    painter.drawRect(offsetX + offsetMiniChart, height() - 50, (offsetX + (lastMousePositionZoomStatic.x() - firstMousePositionZoomStatic.x()) / oldCoef) - offsetX, 50);
//    painter.drawLine(offsetX, height() - 50, offsetX, height());
//    painter.drawLine(offsetX + (lastMousePositionZoomStatic.x() - firstMousePositionZoomStatic.x()) / oldCoef, height() - 50, offsetX + (lastMousePositionZoomStatic.x() - firstMousePositionZoom.x()) / oldCoef, height());

    std::unordered_map<int, int> logLevelPosX;
    for(const InfoMessage &message : messages){
//        int x1 = ((message.startTime - minTime) * Q_ - (offsetX * coef));
//        //            std::cout << (func.endTime - func.startTime) * Q_ << std::endl;
////        std::cout << x1 << " " << x2 << " " << std::fixed << coef << std::endl;
//        if(message.nestingLevel > 0 && ((x1 <= width() && x1 >= 0) || (x2 >= 0 && x2 <= width()) || (x1 <= 0 && x2 >= width())) && (diagramWidget->displayAll->isChecked() || (message.endTime - message.startTime) * Q_ > 1)){

        bool draw = false;
        for(auto &pair : visibleFunctions[message.nestingLevel]){
            if(message.timestamp > pair.first && message.timestamp < pair.second)
                draw = true;
        }

        int h = HEIGHT / 2 > 20 ? 20 : HEIGHT / 2;
        int interval = ((message.timestamp - minTime) * Q_ - (offsetX * coef)) - logLevelPosX[message.nestingLevel];
        if((diagramWidget->displayInfo->isChecked() || (diagramWidget->displayInfoPartially->isChecked() && interval > 7)) && h > 0 && draw){


            if(message.logLevel == INFO)
                painter.setBrush(Qt::white);
            else if(message.logLevel == WARNING)
                painter.setBrush(Qt::yellow);
            else if(message.logLevel == DEBUG)
                painter.setBrush(Qt::cyan);
            else if(message.logLevel == ERROR)
                painter.setBrush(Qt::red);

            painter.drawEllipse(((message.timestamp - minTime) * Q_ - (offsetX * coef)), HEIGHT * (message.nestingLevel - 1) + offsetY + 50 + h / 2, h, h);
            unsigned __int128 rem_ns;
            std::tm tm = Logger::ns_to_tm(message.timestamp , rem_ns);
            std::string time = Logger::format_tm(tm, rem_ns);
            Pos pos_(((message.timestamp - minTime) * Q_ - (offsetX * coef)), HEIGHT * (message.nestingLevel - 1) + offsetY + 50 + h / 2, h, h, "Time: " + time + "ns" + "\n" +
                                                                                                                                             Logger::logLevelToString((LogLevel) message.logLevel) + "\n" +
                                                                                                                                             message.message);
            logLevelPosX[message.nestingLevel] = ((message.timestamp - minTime) * Q_ - (offsetX * coef));
            pos.push_back(pos_);
        }
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::black);

    if(zoom && !zoomBlock){
        painter.drawLine(firstMousePositionZoom.x(), 0, firstMousePositionZoom.x(), height() - 50);
        painter.drawLine(lastMousePositionZoom.x(), 0, lastMousePositionZoom.x(), height() - 50);
        //            std::cout << std::fixed << firstMousePositionZoom.x() / Q_ + minTime << " " << lastMousePositionZoom.x() / Q_ << " " << lastMousePositionZoom.x() / Q_ - firstMousePositionZoom.x() / Q_ << std::endl;
    }
//    std::cout << item << std::endl;
}
