#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    controler = new JackController();

    for (int i = 0; i < 5; i++) {
        controler->add_track();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete controler;
}

