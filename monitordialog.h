#ifndef TRACKINGDIALOG_H
#define TRACKINGDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QPixmap>
#include <opencv2/opencv.hpp>

namespace Ui {
class MonitorDialog;
}

class MonitorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MonitorDialog(QWidget *parent = 0);
    ~MonitorDialog();


    void updateWindow(const QPixmap & image11,
                      const QPixmap & image12,
                      const QPixmap & image21,
                      const QPixmap & image22);

private:
    Ui::MonitorDialog *ui;
    QVBoxLayout* main_layout; // Outer layout to make the dialog expandable
};

#endif // TRACKINGDIALOG_H
