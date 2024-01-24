/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#pragma once

#include <QDialog>
#include "ui_Ignore.h"

#include "Etemenanki.h"

class Etemenanki;

class IgnoreDialog : public QDialog {
    Q_OBJECT
public:
    explicit IgnoreDialog(Etemenanki* etemenanki, QWidget* parent = nullptr);
public slots:
    void accept() override;

    // Ignored Files extensions widget
    void on_ignore_add_button_clicked();
    void on_ignore_update_button_clicked();
    void on_ignore_remove_button_clicked();
    void on_ignore_list_widget_clicked();
private:
    Ui::IgnoreDialog ui;
    Etemenanki* m_etemenanki;

    // UI Controls
    void addIgnoreItem(QString path);
};
