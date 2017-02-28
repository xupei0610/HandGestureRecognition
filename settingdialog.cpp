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

void SettingDialog::on_barMinH_valueChanged(int value)
{
    ui->lblMinH->setText(QString::number(value));
    _min_H = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMaxH_valueChanged(int value)
{
    ui->lblMaxH->setText(QString::number(value));
    _max_H = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMinS_valueChanged(int value)
{
    ui->lblMinS->setText(QString::number(value));
    _min_S = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMaxS_valueChanged(int value)
{
    ui->lblMaxS->setText(QString::number(value));
    _max_S = value;
    _emitColorBoundSignal();
}


void SettingDialog::on_barMinV_valueChanged(int value)
{
    ui->lblMinV->setText(QString::number(value));
    _min_V = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMaxV_valueChanged(int value)
{
    ui->lblMaxV->setText(QString::number(value));
    _max_V = value;
    _emitColorBoundSignal();
}

void SettingDialog::on_barMinHorizon_valueChanged(int value)
{
    ui->lblMinHorizon->setText(QString::number(value) + QString("%"));
    _min_ROI_horizon = value;
    _emitROISignal();
}

void SettingDialog::on_barMaxHorizon_valueChanged(int value)
{
    ui->lblMaxHorizon->setText(QString::number(value) + QString("%"));
    _max_ROI_horizon = value;
    _emitROISignal();
}

void SettingDialog::on_barMinVertical_valueChanged(int value)
{
    ui->lblMinVertical->setText(QString::number(value) + QString("%"));
    _min_ROI_vertical = value;
    _emitROISignal();
}

void SettingDialog::on_barMaxVertical_valueChanged(int value)
{
    ui->lblMaxVertical->setText(QString::number(value) + QString("%"));
    _max_ROI_vertical = value;
    _emitROISignal();
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

std::pair<std::pair<int, int>, std::pair<int, int> > SettingDialog::getROI()
{
    return std::make_pair(
                std::make_pair(_min_ROI_horizon, _max_ROI_horizon),
                std::make_pair(_min_ROI_vertical,_max_ROI_vertical)
                );
}

void SettingDialog::on_chkErode_toggled(bool checked)
{
    emit changeErode(checked);
}

void SettingDialog::on_chkDilate_toggled(bool checked)
{
    emit changeDilate(checked);
}

void SettingDialog::on_chkMedianBlur_toggled(bool checked)
{
    emit changeMedianBlur(checked);
}
