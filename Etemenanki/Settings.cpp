/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "Settings.h"

SettingsDialog::SettingsDialog(Etemenanki* m_etemenanki, QWidget* parent) : QDialog(parent), m_etemenanki(m_etemenanki) {
    ui.setupUi(this);

    if (m_etemenanki) {
        ui.comprehensive_button->setChecked(m_etemenanki->getComprehensive());
        ui.fill_in_ids_button->setEnabled(m_etemenanki->getComprehensive());
        ui.fill_in_ids_button->setChecked(m_etemenanki->getFillInIds());
    }
}

void SettingsDialog::accept() {
    if (m_etemenanki) {
        m_etemenanki->setComprehensive(ui.comprehensive_button->isChecked());

        if (ui.comprehensive_button->isChecked()) {
            m_etemenanki->setFillInIds(ui.fill_in_ids_button->isChecked());
            m_etemenanki->toggleOffsetControl(!ui.fill_in_ids_button->isChecked());
        } else {
            m_etemenanki->setFillInIds(false);
            m_etemenanki->toggleOffsetControl(true);
        }
    }

    QDialog::accept();
}

void SettingsDialog::on_comprehensive_button_clicked() {
    ui.fill_in_ids_button->setEnabled(ui.comprehensive_button->isChecked());
}