#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = 0);
    ~SettingDialog();

    void setMinH(const int &);
    void setMaxH(const int &);
    void setMaxS(const int &);
    void setMinS(const int &);
    void setMaxV(const int &);
    void setMinV(const int &);
    void setMaxROIHorizon(const int &);
    void setMinROIHorizon(const int &);
    void setMaxROIVertical(const int &);
    void setMinROIVertical(const int &);
    void setErode(const bool &);
    void setDilate(const bool &);
    void setMedianBlur(const bool &);

    int getMaxH();
    int getMinH();
    int getMaxS();
    int getMinS();
    int getMaxV();
    int getMinV();
    bool getErode();
    bool getDilate();
    bool getMedianBlur();
    std::pair<std::pair<int, int>, std::pair<int, int> > getROI();

private slots:
    void on_barMinH_valueChanged(int value);
    void on_barMaxH_valueChanged(int value);
    void on_barMinS_valueChanged(int value);
    void on_barMaxS_valueChanged(int value);
    void on_barMaxV_valueChanged(int value);
    void on_barMinV_valueChanged(int value);
    void on_chkErode_toggled(bool checked);
    void on_chkDilate_toggled(bool checked);
    void on_chkMedianBlur_toggled(bool checked);
    void on_barMinHorizon_valueChanged(int value);
    void on_barMaxVertical_valueChanged(int value);
    void on_barMaxHorizon_valueChanged(int value);
    void on_barMinVertical_valueChanged(int value);
    void on_btnReset_clicked();

signals:
    void resetSetting();
    void changeColorLowerBound(const int & H, const int & S, const int & V);
    void changeColorUpperBound(const int & H, const int & S, const int & V);
    void changeROI(const int & start_x_percent,
                   const int & start_y_percent,
                   const int & end_x_percent,
                   const int & end_y_percent);
    void changeErode(bool);
    void changeDilate(bool);
    void changeMedianBlur(bool);

private:
    int _max_H = 0;
    int _min_H = 0;
    int _max_S = 0;
    int _min_S = 0;
    int _max_V = 0;
    int _min_V = 0;
    int _min_ROI_horizon = 0;
    int _max_ROI_horizon = 0;
    int _min_ROI_vertical= 0;
    int _max_ROI_vertical= 0;

    void _emitColorBoundSignal();
    void _emitROISignal();

    Ui::SettingDialog *ui;
};

#endif // SETTINGDIALOG_H
