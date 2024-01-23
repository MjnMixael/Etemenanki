/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

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
    void on_sorting_combo_box_currentIndexChanged(int val);
private:
    Ui::SettingsDialog ui;
    Etemenanki* m_etemenanki;
};
