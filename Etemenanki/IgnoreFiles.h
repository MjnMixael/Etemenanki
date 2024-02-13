/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#pragma once

#include <QDialog>
#include "ui_IgnoreFiles.h"

#include "Etemenanki.h"

class Etemenanki;

class IgnoreFilesDialog : public QDialog {
    Q_OBJECT
public:
    explicit IgnoreFilesDialog(Etemenanki* etemenanki, QWidget* parent = nullptr);
public slots:
    void accept() override;

    // Ignored Files extensions widget
    void on_ignore_files_add_button_clicked();
    void on_ignore_files_update_button_clicked();
    void on_ignore_files_remove_button_clicked();
    void on_ignore_files_clear_button_clicked();
    void on_ignore_files_list_widget_clicked();
private:
    Ui::IgnoreFilesDialog ui;
    Etemenanki* m_etemenanki;

    // UI Controls
    void addIgnoreFilesItem(QString path);
};
