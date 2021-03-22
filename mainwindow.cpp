#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hierarchicalheadermodel.h"
#include "hierarchicalheaderview.h"

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());


    QStandardItemModel *standardModel = new QStandardItemModel(this);

    QStandardItem *head1 = new QStandardItem("head1");
    QStandardItem *head2 = new QStandardItem("head2");
    QStandardItem *head3 = new QStandardItem("head3");

    head1->appendColumn({new QStandardItem("head1 son1")});
    head1->appendColumn({new QStandardItem("head1 son2")});
    head2->appendColumn({new QStandardItem("head2 son1")});
    head3->appendColumn({new QStandardItem("head3 son1")});

    standardModel->appendColumn({head1});
    standardModel->appendColumn({head2});
    standardModel->appendColumn({head3});

    HierarchicalHeaderModel *newModel = new HierarchicalHeaderModel(standardModel);

    QTreeWidget *newTree = new QTreeWidget(this);
    HierarchicalHeaderView *horizontalheader = new HierarchicalHeaderView(Qt::Horizontal, this);
    horizontalheader->setCanSort(true);
    horizontalheader->setModel(newModel);
    newTree->setHeader(horizontalheader);

    horizontalheader->setSectionsClickable(true); // qtreewidget setHeader will reset clickable flag

    HierarchicalHeaderView *verticalheader = new HierarchicalHeaderView(Qt::Vertical, this);
    verticalheader->setCanSort(false);
    verticalheader->setModel(newModel);

    QTableView *table = new QTableView(this);
    table->setVerticalHeader(verticalheader);
    verticalheader->setSectionsClickable(true); // setHeader will reset clickable flag

    mainLayout->addWidget(newTree);
    mainLayout->addWidget(table);

    QPushButton *pushButton = new QPushButton("set VHead TextRole Red");
    connect(pushButton, &QPushButton::clicked, this, [verticalheader](){
        verticalheader->setColor(HierarchicalHeaderView::TextRole, Qt::red);
    });

    mainLayout->addWidget(pushButton);
}

MainWindow::~MainWindow()
{
    delete ui;
}
