#include "monitordialog.h"
#include "ui_monitordialog.h"

MonitorDialog::MonitorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MonitorDialog)
{
    ui->setupUi(this);
    main_layout = new QVBoxLayout;
    main_layout->addWidget(ui->gridLayoutWidget);
    setLayout(main_layout);
}

MonitorDialog::~MonitorDialog()
{
    delete main_layout;
    delete ui;
}

void MonitorDialog::updateWindow(const QPixmap & image11, const QPixmap & image12, const QPixmap & image21, const QPixmap & image22)
{
    ui->lblRoi->setPixmap(
                image11.scaled(
                    QSize(ui->lblRoi->width(),
                          ui->lblRoi->height()),
                    Qt::KeepAspectRatio)
                );

    ui->lblFiltered->setPixmap(
                image12.scaled(
                    QSize(ui->lblFiltered->width(),
                          ui->lblFiltered->height()),
                    Qt::KeepAspectRatio)
                );

    ui->lblContour->setPixmap(
                image21.scaled(
                    QSize(ui->lblContour->width(),
                          ui->lblContour->height()),
                    Qt::KeepAspectRatio)
                );

    ui->lblConvexity->setPixmap(
                image22.scaled(
                    QSize(ui->lblConvexity->width(),
                          ui->lblConvexity->height()),
                    Qt::KeepAspectRatio)
                );
}
