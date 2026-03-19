#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dataHandler(DataHandler::instance())
{
    dataHandler.loadAll();

    ui->setupUi(this);

    // Setup nice title style I found
    ui->label_title->setStyleSheet(R"(
        QLabel {
            color: #D4AF37; /* Gold for that Nusr-Et look */
            background-color: #1a1a1a;
            font-size: 28px;
            font-weight: bold;
            border: 2px solid #D4AF37;
            padding: 10px;
            border-radius: 8px;
        }
    )");

    // Setup some nice button styles I found online
    QString style = R"(
        QPushButton {
            background-color: #f0f0f0;
            border: 1px solid #ababab;
            border-radius: 4px;
            padding: 5px;
        }

        QPushButton:checked {
            background-color: #4CAF50; /* Snappy Green */
            color: white;
            font-weight: bold;
            border: 2px solid #2e7d32;
        }

        QPushButton:hover {
            background-color: #e0e0e0;
        }
    )";

    this->setStyleSheet(style);

    // Instantiate tabs and add them to the main tab widget
    m_dailyTab = new DailyTab(this);
    m_todoTab = new TodoTab(this);
    ui->tabWidget->addTab(m_dailyTab, "Daily");
    ui->tabWidget->addTab(m_todoTab, "To-Do");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Intercept the close window event so that data can be saved to disc first!
    dataHandler.saveAll();

    // Then continue as usual
    event->accept();
}