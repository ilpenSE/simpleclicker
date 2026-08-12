#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logger.hpp"
#include "presets.hpp"

extern Logger *lg;
extern PresetManager *presets;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent) , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  lg->info("Hello from MainWindow!");
}

MainWindow::~MainWindow()
{
  delete ui;
}
