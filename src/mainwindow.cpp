#include "mainwindow.h"



MainWindow::MainWindow(Map * m) : map(m)
{
    line_labels = new QVBoxLayout;
    graph = &(m->g);

    zoomin = new QPushButton("&Zoom In");
    zoomout = new QPushButton("&Zoom Out");
    zoomdefault = new QPushButton("&Default Zoom");
    changetime = new QPushButton("&Change Time");
    settime = new QPushButton("&Set Time");
    pauseresumetime = new QPushButton("&Pause/Resume time");
    settime->hide();
    faster = new QPushButton("&Faster");
    slower = new QPushButton("&Slower");

}

void MainWindow::createScene()
{
    scene = new QGraphicsScene();
    scene->setSceneRect(0,0,600,600);
    scene->setBackgroundBrush(QBrush(Qt::white));
}


void MainWindow::createStreetMap()
{
    sm = new StreetMap(graph, graph->cs, map->stations);
    scene->addItem(sm);
}

void MainWindow::createSystemClock()
{
    sys_clock = new QTimer(this);
    time = new QTime(0,0,0);
    time_edit = new QTimeEdit();
    time_edit->hide();
    time_edit->setTime(*time);
    clock_label = new QLabel(time->toString("hh:mm:ss"));
    QObject::connect(sys_clock, SIGNAL(timeout()), this, SLOT(updateClock()));
}


void MainWindow::createLines()
{
    for (auto line : graph->line_pts) {
        std::cout<<line.first<<std::endl;
        LineObject * lineObject = new LineObject(graph, map->lines,line.first, line.second, time);
        struct timetable_s connection1 = {.start = (0 + 0 + 2)};
        struct timetable_s connection2 = {.start = (1*60*60 + 0*60 + 2)};
        struct timetable_s connection3 = {.start = (1*60*60 + 10*60 + 50)};
        
        std::vector<timetable_s> t = {connection1,connection2,connection3};        
        lineObject->createVehicles();

        line_labels->addWidget(lineObject->label);
        
        scene->addItem(lineObject->route);
        for (auto v : lineObject->vehicles){
            scene->addItem(v);
        }

        QObject::connect(lineObject->label, SIGNAL(clicked()),lineObject->route,SLOT(showRoute()));
        QObject::connect(sys_clock,SIGNAL(timeout()),lineObject, SLOT(startVehicle()));
        QObject::connect(this, SIGNAL(timeChanged(float)), lineObject, SLOT(timeChanged(float)));
        QObject::connect(this, SIGNAL(stopAnimation()), lineObject, SLOT(stopAnimation()));
        QObject::connect(this, SIGNAL(resumeAnimation()), lineObject, SLOT(resumeAnimation()));
        QObject::connect(sm, SIGNAL(updateRoute(float)),lineObject, SLOT(timeChanged(float)));
    }
}


void MainWindow::finish() 
{

    sys_clock->start(1000);
    

    view = new QGraphicsView(scene);
    view->show();

    QObject::connect(zoomin, SIGNAL(clicked()), this, SLOT(zoomIn()));
    QObject::connect(zoomout, SIGNAL(clicked()), this, SLOT(zoomOut()));
    QObject::connect(zoomdefault, SIGNAL(clicked()), this, SLOT(defaultZoom()));
    QObject::connect(changetime, SIGNAL(clicked()), time_edit, SLOT(show()));
    QObject::connect(changetime, SIGNAL(clicked()), settime, SLOT(show()));
    QObject::connect(settime, SIGNAL(clicked()), time_edit, SLOT(hide()));
    QObject::connect(settime, SIGNAL(clicked()), this, SLOT(setTime()));
    QObject::connect(settime, SIGNAL(clicked()), settime, SLOT(hide()));
    QObject::connect(pauseresumetime, SIGNAL(clicked()), this, SLOT(toggleClock()));
    QObject::connect(faster, SIGNAL(clicked()), this, SLOT(fasterClock()));
    QObject::connect(slower, SIGNAL(clicked()), this, SLOT(slowerClock()));


    mainLayout = new QGridLayout;

    mainLayout->addWidget(view,0,0,1,7);
    mainLayout->addLayout(this->line_labels, 0, 7);
    mainLayout->addWidget(clock_label, 1,0);
    mainLayout->addWidget(slower,1,1);
    mainLayout->addWidget(faster,1,2);
    mainLayout->addWidget(pauseresumetime,1,3);
    mainLayout->addWidget(changetime,1,4);
    mainLayout->addWidget(time_edit,2,0);
    mainLayout->addWidget(settime,2,1);  
    mainLayout->addWidget(zoomout,1,5);
    mainLayout->addWidget(zoomdefault,1,6);
    mainLayout->addWidget(zoomin,1,7);
    
    QWidget *widget = new QWidget;
    widget->setLayout(mainLayout);

    setCentralWidget(widget);


    setWindowTitle(tr("ICP -Traffic simulator"));
    setUnifiedTitleAndToolBarOnMac(true);
}


void MainWindow::updateClock()
{
    *time = time->addSecs(1);
    clock_label->setText(time->toString("hh:mm:ss"));
    emit timeChanged(1.f);
}

void MainWindow::zoomIn()
{
    view->scale(1.1,1.1);
}
void MainWindow::zoomOut()
{
    view->scale(0.9,0.9);
}

void MainWindow::setTime()
{
    *time = time_edit->time();
    clock_label->setText(time->toString("hh:mm:ss"));

    emit timeChanged(1.f);
}

void MainWindow::defaultZoom()
{
    view->resetMatrix();
}

void MainWindow::toggleClock()
{
    if (sys_clock->isActive()){
        sys_clock->stop();
        clock_state = false;
        emit stopAnimation();
    }else { 
        sys_clock->start();
        clock_state = true;
        emit resumeAnimation();
    }
}

void MainWindow::fasterClock()
{
    if (sys_clock->interval()>10){
        sys_clock->setInterval(sys_clock->interval()*0.5);
        emit timeChanged(0.5);
    }
}

void MainWindow::slowerClock()
{
    if(sys_clock->interval()<10000){       
        std::cout<<sys_clock->interval()<<std::endl;
        sys_clock->setInterval(sys_clock->interval()*2);
        emit timeChanged(2);
    }
}