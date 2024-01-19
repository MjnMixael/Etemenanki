#pragma once

#include <QDialog>
#include "ui_Settings.h"

#include "Etemenanki.h"

class Etemenanki;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(Etemenanki* etemenanki, QWidget* parent = nullptr);
public slots:
    void accept() override;
    void on_comprehensive_button_clicked();
private:
    Ui::SettingsDialog ui;
    Etemenanki* etemenanki;
};
