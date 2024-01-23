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

        ui.sorting_combo_box->setCurrentIndex(m_etemenanki->getSortingType());

        ui.header_comments_button->setChecked(m_etemenanki->getHeaderAnnotations());
        ui.verbose_comments_button->setChecked(m_etemenanki->getVerboseAnnotations());
    }
}

void SettingsDialog::accept() {
    if (m_etemenanki) {
        m_etemenanki->setComprehensive(ui.comprehensive_button->isChecked());
        m_etemenanki->setSortingType(ui.sorting_combo_box->currentIndex());
        m_etemenanki->setHeaderAnnotations(ui.header_comments_button->isChecked());
        m_etemenanki->setVerboseAnnotations(ui.verbose_comments_button->isChecked());

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

void SettingsDialog::on_sorting_combo_box_currentIndexChanged(int val) {
    if (val == XSTR_ID_ORDER) {
        ui.header_comments_button->setChecked(false);
        ui.header_comments_button->setEnabled(false);
    } else {
        ui.header_comments_button->setEnabled(true);
        ui.header_comments_button->setChecked(true);
    }
}