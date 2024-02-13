/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "IgnoreIds.h"

IgnoreIdsDialog::IgnoreIdsDialog(Etemenanki* m_etemenanki, QWidget* parent) : QDialog(parent), m_etemenanki(m_etemenanki) {
    ui.setupUi(this);

    ui.ignore_ids_line_edit->setValidator(new QIntValidator(0, INT_MAX, this));

    ui.ignore_ids_update_button->setEnabled(false);
    ui.ignore_ids_remove_button->setEnabled(false);

    for (int i = 0; i < m_etemenanki->m_ignoredIdsList.size(); i++) {
        addIgnoreIdsItem(m_etemenanki->m_ignoredIdsList[i]);
    }
}

void IgnoreIdsDialog::accept() {
    if (m_etemenanki) {
        m_etemenanki->m_ignoredIdsList.clear();
        for (int i = 0; i < ui.ignore_ids_list_widget->count(); ++i) {
            m_etemenanki->m_ignoredIdsList.push_back(ui.ignore_ids_list_widget->item(i)->text());
        }
    }

    QDialog::accept();
}

void IgnoreIdsDialog::addIgnoreIdsItem(QString id) {
    if (!Etemenanki::itemExists(ui.ignore_ids_list_widget, id)) {
        // Make sure it's a valid positive integer
        if (id > 0) {
            ui.ignore_ids_list_widget->addItem(id);
            ui.ignore_ids_line_edit->clear();
        }
    }
}

void IgnoreIdsDialog::on_ignore_ids_add_button_clicked() {
    QString ext = ui.ignore_ids_line_edit->text();

    addIgnoreIdsItem(ext);
}

void IgnoreIdsDialog::on_ignore_ids_update_button_clicked() {
    int i = ui.ignore_ids_list_widget->currentRow();
    QString path = ui.ignore_ids_line_edit->text();

    if (!Etemenanki::itemExists(ui.ignore_ids_list_widget, path)) {
        ui.ignore_ids_list_widget->item(i)->setText(path);
        ui.ignore_ids_line_edit->clear();
        ui.ignore_ids_update_button->setEnabled(false);
        ui.ignore_ids_remove_button->setEnabled(false);
        ui.ignore_ids_list_widget->clearSelection();
    }
}

void IgnoreIdsDialog::on_ignore_ids_remove_button_clicked() {
    ui.ignore_ids_list_widget->takeItem(ui.ignore_ids_list_widget->currentRow());
    ui.ignore_ids_line_edit->clear();
    ui.ignore_ids_update_button->setEnabled(false);
    ui.ignore_ids_remove_button->setEnabled(false);
    ui.ignore_ids_list_widget->clearSelection();
}

void IgnoreIdsDialog::on_ignore_ids_clear_button_clicked() {
    ui.ignore_ids_list_widget->clear();
    ui.ignore_ids_line_edit->clear();
    ui.ignore_ids_update_button->setEnabled(false);
    ui.ignore_ids_remove_button->setEnabled(false);
    ui.ignore_ids_list_widget->clearSelection();
}

void IgnoreIdsDialog::on_ignore_ids_list_widget_clicked() {
    int i = ui.ignore_ids_list_widget->currentRow();
    ui.ignore_ids_line_edit->setText(ui.ignore_ids_list_widget->item(i)->text());
    ui.ignore_ids_update_button->setEnabled(true);
    ui.ignore_ids_remove_button->setEnabled(true);
}