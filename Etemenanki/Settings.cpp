/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "Settings.h"

SettingsDialog::SettingsDialog(Etemenanki* m_etemenanki, QWidget* parent) : QDialog(parent), m_etemenanki(m_etemenanki) {
    ui.setupUi(this);

    std::string g_sortingNames[Next_available_order] = { 
        "Parsing Order", 
        "XSTR ID Order", 
        "Filename Order" 
    };
    
    for (int i = 0; i < Next_available_order; i++) {
        ui.sorting_combo_box->addItem(QString::fromStdString(g_sortingNames[i]));
    }

    if (m_etemenanki) {
        ui.comprehensive_button->setChecked(m_etemenanki->getComprehensive());

        ui.fill_in_ids_button->setEnabled(m_etemenanki->getComprehensive());
        ui.fill_in_ids_button->setChecked(m_etemenanki->getFillInIds());

        ui.sorting_combo_box->setEnabled(m_etemenanki->getComprehensive());
        ui.sorting_combo_box->setCurrentIndex(m_etemenanki->getSortingType());
    }
}

void SettingsDialog::accept() {
    if (m_etemenanki) {
        m_etemenanki->setComprehensive(ui.comprehensive_button->isChecked());

        if (ui.comprehensive_button->isChecked()) {
            m_etemenanki->setFillInIds(ui.fill_in_ids_button->isChecked());
            m_etemenanki->toggleOffsetControl(!ui.fill_in_ids_button->isChecked());
            m_etemenanki->setSortingType(ui.sorting_combo_box->currentIndex());
        } else {
            m_etemenanki->setFillInIds(false);
            m_etemenanki->toggleOffsetControl(true);
            m_etemenanki->setSortingType(PARSING_ORDER);
        }
    }

    QDialog::accept();
}

void SettingsDialog::on_comprehensive_button_clicked() {
    ui.fill_in_ids_button->setEnabled(ui.comprehensive_button->isChecked());
    ui.sorting_combo_box->setEnabled(ui.comprehensive_button->isChecked());
    if (!ui.sorting_combo_box->isEnabled()) {
        ui.sorting_combo_box->setCurrentIndex(PARSING_ORDER);
    }
}