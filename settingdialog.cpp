#include "settingdialog.h"
#include "ui_settingdialog.h"

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::setMaxH(const int & val)
{
    ui->barMaxH->setValue(val);
}

void SettingDialog::setMinH(const int & val)
{
    ui->barMinH->setValue(val);
}

void SettingDialog::setMaxS(const int & val)
{
    ui->barMaxS->setValue(val);
}

void SettingDialog::setMinS(const int & val)
{
    ui->barMinS->setValue(val);
}

void SettingDialog::setMaxV(const int & val)
{
    ui->barMaxV->setValue(val);
}

void SettingDialog::setMinV(const int & val)
{
    ui->barMinV->setValue(val);
}

void SettingDialog::setMaxROIHorizon(const int & val)
{
    ui->barMaxHorizon->setValue(val);
}

void SettingDialog::setMinROIHorizon(const int & val)
{
    ui->barMinHorizon->setValue(val);
}

void SettingDialog::setMaxROIVertical(const int & val)
{
    ui->barMaxVertical->setValue(val);
}

void SettingDialog::setMinROIVertical(const int & val)
{
    ui->barMinVertical->setValue(val);
}

void SettingDialog::setDetectionArea(const int & val)
{
    ui->boxDetectionArea->setValue(val);
}

void SettingDialog::setSensitivity(const int & val)
{
    ui->boxSensitivity->setValue(val);
}

int SettingDialog::getSensitivity()
{
    return ui->boxSensitivity->value();
}

void SettingDialog::setSamplingFPS(const int & val)
{
    ui->boxFPS->setValue(val);
}

void SettingDialog::setActionInterval(const int & val)
{
    ui->boxInterval->setValue(val);
}

void SettingDialog::_emitColorBoundSignal()
{
    emit changeColorUpperBound(_min_H > _max_H ? _min_H : _max_H,
                               _min_S > _max_S ? _min_S : _max_S,
                               _min_V > _max_V ? _min_V : _max_V);
    emit changeColorLowerBound(_min_H < _max_H ? _min_H : _max_H,
                               _min_S < _max_S ? _min_S : _max_S,
                               _min_V < _max_V ? _min_V : _max_V);
}

void SettingDialog::_emitROISignal()
{
    emit changeROI(_min_ROI_horizon  < _max_ROI_horizon  ? _min_ROI_horizon  : _max_ROI_horizon,
                   _min_ROI_vertical < _max_ROI_vertical ? _min_ROI_vertical : _max_ROI_vertical,
                   _min_ROI_horizon  > _max_ROI_horizon  ? _min_ROI_horizon  : _max_ROI_horizon,
                   _min_ROI_vertical > _max_ROI_vertical ? _min_ROI_vertical : _max_ROI_vertical);
}

void SettingDialog::on_btnReset_clicked()
{
    emit resetSetting();
}

void SettingDialog::on_barMinH_valueChanged(const int &value)
{
    ui->lblMinH->setText(QString::number(value));
    _min_H = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMaxH_valueChanged(const int &value)
{
    ui->lblMaxH->setText(QString::number(value));
    _max_H = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMinS_valueChanged(const int &value)
{
    ui->lblMinS->setText(QString::number(value));
    _min_S = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMaxS_valueChanged(const int &value)
{
    ui->lblMaxS->setText(QString::number(value));
    _max_S = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMinV_valueChanged(const int &value)
{
    ui->lblMinV->setText(QString::number(value));
    _min_V = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMaxV_valueChanged(const int &value)
{
    ui->lblMaxV->setText(QString::number(value));
    _max_V = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMinHorizon_valueChanged(const int &value)
{
    ui->lblMinHorizon->setText(QString::number(value) + QString("%"));
    _min_ROI_horizon = value;
    _emitROISignal();
}

void SettingDialog::on_barMaxHorizon_valueChanged(const int &value)
{
    ui->lblMaxHorizon->setText(QString::number(value) + QString("%"));
    _max_ROI_horizon = value;
    _emitROISignal();
}

void SettingDialog::on_barMinVertical_valueChanged(const int &value)
{
    ui->lblMinVertical->setText(QString::number(value) + QString("%"));
    _min_ROI_vertical = value;
    _emitROISignal();
}

void SettingDialog::on_barMaxVertical_valueChanged(const int &value)
{
    ui->lblMaxVertical->setText(QString::number(value) + QString("%"));
    _max_ROI_vertical = value;
    _emitROISignal();
}

void SettingDialog::on_boxDetectionArea_valueChanged(const int &arg1)
{
    emit changeDetectionArea(arg1);
}

void SettingDialog::on_boxInterval_valueChanged(const int & arg1)
{
    emit changeActionInterval(arg1);
}

void SettingDialog::on_boxSensitivity_valueChanged(const int & arg1)
{
    emit changeSensitivity(arg1);
}

void SettingDialog::on_boxFPS_valueChanged(const int &arg1)
{
    emit changeSamplingFPS(arg1);
}
