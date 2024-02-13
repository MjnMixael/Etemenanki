/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "IgnoreFiles.h"

IgnoreFilesDialog::IgnoreFilesDialog(Etemenanki* m_etemenanki, QWidget* parent) : QDialog(parent), m_etemenanki(m_etemenanki) {
    ui.setupUi(this);

    ui.ignore_files_update_button->setEnabled(false);
    ui.ignore_files_remove_button->setEnabled(false);

    for (int i = 0; i < m_etemenanki->m_ignoredFilesList.size(); i++) {
        addIgnoreFilesItem(m_etemenanki->m_ignoredFilesList[i]);
    }
}

void IgnoreFilesDialog::accept() {
    if (m_etemenanki) {
        m_etemenanki->m_ignoredFilesList.clear();
        for (int i = 0; i < ui.ignore_files_list_widget->count(); ++i) {
            m_etemenanki->m_ignoredFilesList.push_back(ui.ignore_files_list_widget->item(i)->text());
        }
    }

    QDialog::accept();
}

void IgnoreFilesDialog::addIgnoreFilesItem(QString path) {
    if (!Etemenanki::itemExists(ui.ignore_files_list_widget, path)) {
        ui.ignore_files_list_widget->addItem(path);
        ui.ignore_files_line_edit->clear();
    }
}

void IgnoreFilesDialog::resetIgnoreFilesInput() {
    ui.ignore_files_line_edit->clear();
    ui.ignore_files_update_button->setEnabled(false);
    ui.ignore_files_remove_button->setEnabled(false);
    ui.ignore_files_list_widget->clearSelection();
}

void IgnoreFilesDialog::on_ignore_files_add_button_clicked() {
    QString path = ui.ignore_files_line_edit->text();

    if (path.isEmpty()) {
        return;
    }

    addIgnoreFilesItem(path);
}

void IgnoreFilesDialog::on_ignore_files_update_button_clicked() {
    int i = ui.ignore_files_list_widget->currentRow();
    QString path = ui.ignore_files_line_edit->text();

    if (path.isEmpty()) {
        resetIgnoreFilesInput();
        return;
    }

    if (!Etemenanki::itemExists(ui.ignore_files_list_widget, path)) {
        ui.ignore_files_list_widget->item(i)->setText(path);
        resetIgnoreFilesInput();
    }
}

void IgnoreFilesDialog::on_ignore_files_remove_button_clicked() {
    ui.ignore_files_list_widget->takeItem(ui.ignore_files_list_widget->currentRow());
    resetIgnoreFilesInput();
}

void IgnoreFilesDialog::on_ignore_files_clear_button_clicked() {
    ui.ignore_files_list_widget->clear();
    resetIgnoreFilesInput();
}

void IgnoreFilesDialog::on_ignore_files_list_widget_clicked() {
    int i = ui.ignore_files_list_widget->currentRow();
    ui.ignore_files_line_edit->setText(ui.ignore_files_list_widget->item(i)->text());
    ui.ignore_files_update_button->setEnabled(true);
    ui.ignore_files_remove_button->setEnabled(true);
}