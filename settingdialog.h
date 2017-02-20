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

private slots:
    void on_barMinH_valueChanged(int value);

    void on_barMaxH_valueChanged(int value);

    void on_barMinS_valueChanged(int value);

    void on_barMaxS_valueChanged(int value);

    void on_barMaxV_valueChanged(int value);

    void on_barMinV_valueChanged(int value);

    void on_btnReset_released();

private:
    int _max_H = 0;
    int _min_H = 0;
    int _max_S = 0;
    int _min_S = 0;
    int _max_V = 0;
    int _min_V = 0;

    void _setMaxHValue(const int &);
    void _setMinHValue(const int &);
    void _setMaxSValue(const int &);
    void _setMinSValue(const int &);
    void _setMaxVValue(const int &);
    void _setMinVValue(const int &);

    Ui::SettingDialog *ui;
};

#endif // SETTINGDIALOG_H
