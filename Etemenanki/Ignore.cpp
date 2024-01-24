/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "Ignore.h"

IgnoreDialog::IgnoreDialog(Etemenanki* m_etemenanki, QWidget* parent) : QDialog(parent), m_etemenanki(m_etemenanki) {
    ui.setupUi(this);

    for (int i = 0; i < m_etemenanki->m_ignoredList.size(); i++) {
        addIgnoreItem(m_etemenanki->m_ignoredList[i]);
    }
}

void IgnoreDialog::accept() {
    if (m_etemenanki) {
        m_etemenanki->m_ignoredList.clear();
        for (int i = 0; i < ui.ignore_list_widget->count(); ++i) {
            m_etemenanki->m_ignoredList.push_back(ui.ignore_list_widget->item(i)->text());
        }
    }

    QDialog::accept();
}

void IgnoreDialog::addIgnoreItem(QString path) {
    if (!Etemenanki::itemExists(ui.ignore_list_widget, path)) {
        ui.ignore_list_widget->addItem(path);
        ui.ignore_line_edit->clear();
    }
}

void IgnoreDialog::on_ignore_add_button_clicked() {
    QString ext = ui.ignore_line_edit->text();

    addIgnoreItem(ext);
}

void IgnoreDialog::on_ignore_update_button_clicked() {
    int i = ui.ignore_list_widget->currentRow();
    QString path = ui.ignore_line_edit->text();

    if (!Etemenanki::itemExists(ui.ignore_list_widget, path)) {
        ui.ignore_list_widget->item(i)->setText(path);
        ui.ignore_line_edit->clear();
        ui.ignore_update_button->setEnabled(false);
        ui.ignore_remove_button->setEnabled(false);
        ui.ignore_list_widget->clearSelection();
    }
}

void IgnoreDialog::on_ignore_remove_button_clicked() {
    ui.ignore_list_widget->takeItem(ui.ignore_list_widget->currentRow());
    ui.ignore_line_edit->clear();
    ui.ignore_update_button->setEnabled(false);
    ui.ignore_remove_button->setEnabled(false);
    ui.ignore_list_widget->clearSelection();
}

void IgnoreDialog::on_ignore_list_widget_clicked() {
    int i = ui.ignore_list_widget->currentRow();
    ui.ignore_line_edit->setText(ui.ignore_list_widget->item(i)->text());
    ui.ignore_update_button->setEnabled(true);
    ui.ignore_remove_button->setEnabled(true);
}