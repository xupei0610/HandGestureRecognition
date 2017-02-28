#ifndef TRACKINGDIALOG_H
#define TRACKINGDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QPixmap>
#include <opencv2/opencv.hpp>

namespace Ui {
class TrackingDialog;
}

class TrackingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrackingDialog(QWidget *parent = 0);
    ~TrackingDialog();


    void updateWindow(const QPixmap & image11,
                      const QPixmap & image12,
                      const QPixmap & image21,
                      const QPixmap & image22);

private:
    Ui::TrackingDialog *ui;
    QVBoxLayout* main_layout; // Outer layout to make the dialog expandable
};

#endif // TRACKINGDIALOG_H
