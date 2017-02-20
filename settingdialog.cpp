#include "settingdialog.h"
#include "ui_settingdialog.h"

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
    setWindowTitle("Setting");
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::setErode(const bool & val)
{
    if (val)
        ui->chkErode->setCheckState(Qt::Checked);
    else
        ui->chkErode->setCheckState(Qt::Unchecked);
}

void SettingDialog::setDilate(const bool & val)
{
    if (val)
        ui->chkDilate->setCheckState(Qt::Checked);
    else
        ui->chkDilate->setCheckState(Qt::Unchecked);
}

void SettingDialog::setMedianBlur(const bool & val)
{
    if (val)
        ui->chkMedianBlur->setCheckState(Qt::Checked);
    else
        ui->chkMedianBlur->setCheckState(Qt::Unchecked);
}

bool SettingDialog::getErode()
{
    return ui->chkErode->checkState();
}

bool SettingDialog::getDilate()
{
    return ui->chkDilate->checkState();
}

bool SettingDialog::getMedianBlur()
{
    return ui->chkMedianBlur->checkState();
}

void SettingDialog::setMaxH(const int & val)
{
    ui->barMaxH->setValue(val);
    ui->lblMaxH->setText(QString::number(val));
    _setMaxHValue(val);
}

void SettingDialog::setMinH(const int & val)
{
    ui->barMinH->setValue(val);
    ui->lblMinH->setText(QString::number(val));
    _setMinHValue(val);
}

void SettingDialog::setMaxS(const int & val)
{
    ui->barMaxS->setValue(val);
    ui->lblMaxS->setText(QString::number(val));
    _setMaxSValue(val);
}

void SettingDialog::setMinS(const int & val)
{
    ui->barMinS->setValue(val);
    ui->lblMinS->setText(QString::number(val));
    _setMinSValue(val);
}

void SettingDialog::setMaxV(const int & val)
{
    ui->barMaxV->setValue(val);
    ui->lblMaxV->setText(QString::number(val));
    _setMaxVValue(val);

}

void SettingDialog::setMinV(const int & val)
{
    ui->barMinV->setValue(val);
    ui->lblMinV->setText(QString::number(val));
    _setMinVValue(val);
}

void SettingDialog::_setMaxHValue(const int & val)
{
    if (_min_H > val)
    {
        _max_H = _min_H;
        _min_H = val;
    }
    else
        _max_H = val;
}

void SettingDialog::_setMinHValue(const int & val)
{
    if (_max_H < val)
    {
        _min_H = _max_H;
        _max_H = val;
    }
    else
        _min_H = val;
}

void SettingDialog::_setMaxSValue(const int & val)
{
    if (_min_S > val)
    {
        _max_S = _min_S;
        _min_S = val;
    }
    else
        _max_S = val;
}

void SettingDialog::_setMinSValue(const int & val)
{
    if (_max_S < val)
    {
        _min_S = _max_S;
        _max_S = val;
    }
    else
        _min_S = val;
}

void SettingDialog::_setMaxVValue(const int & val)
{
    if (_min_V > val)
    {
        _max_V = _min_V;
        _min_V = val;
    }
    else
        _max_V = val;
}

void SettingDialog::_setMinVValue(const int & val)
{
    if (_max_V < val)
    {
        _min_V = _max_V;
        _max_V = val;
    }
    else
        _min_V = val;
}

int SettingDialog::getMaxH()
{
    return _max_H;
}

int SettingDialog::getMinH()
{
    return _min_H;
}

int SettingDialog::getMaxS()
{
    return _max_S;
}

int SettingDialog::getMinS()
{
    return _min_S;
}

int SettingDialog::getMaxV()
{
    return _max_V;
}

int SettingDialog::getMinV()
{
    return _min_V;
}

void SettingDialog::on_barMinH_valueChanged(int value)
{
    ui->lblMinH->setText(QString::number(value));
    _setMinHValue(value);
}

void SettingDialog::on_barMaxH_valueChanged(int value)
{
    ui->lblMaxH->setText(QString::number(value));
    _setMaxHValue(value);
}

void SettingDialog::on_barMinS_valueChanged(int value)
{
    ui->lblMinS->setText(QString::number(value));
    _setMinSValue(value);
}

void SettingDialog::on_barMaxS_valueChanged(int value)
{

    ui->lblMaxS->setText(QString::number(value));
    _setMaxSValue(value);
}


void SettingDialog::on_barMinV_valueChanged(int value)
{

    ui->lblMinV->setText(QString::number(value));
    _setMinVValue(value);
}

void SettingDialog::on_barMaxV_valueChanged(int value)
{

    ui->lblMaxV->setText(QString::number(value));
    _setMaxVValue(value);
}

void SettingDialog::on_btnReset_released()
{

}
