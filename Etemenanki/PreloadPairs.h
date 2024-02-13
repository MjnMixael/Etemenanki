/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#pragma once

#include <QDialog>
#include "ui_PreloadPairs.h"

#include "Etemenanki.h"

class Etemenanki;

class PreloadPairsDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreloadPairsDialog(Etemenanki* etemenanki, QWidget* parent = nullptr);
public slots:
    void accept() override;

    // Item Verification
    static bool itemExists(QTableWidget* tableWidget, const QString& textToCheck, const QString& idToCheck);

    // Preload Pairs extensions widget
    void on_preload_add_button_clicked();
    void on_preload_update_button_clicked();
    void on_preload_remove_button_clicked();
    void on_preload_clear_button_clicked();
    void on_preload_table_widget_clicked();
private:
    Ui::PreloadPairsDialog ui;
    Etemenanki* m_etemenanki;

    int m_preloadIdWidth = 100;

    // UI Controls
    bool pairExists(QString text, QString id);
    bool addNewPair(QString text, QString id, int row = -1);

    void loadPreloadedPairs();
    void savePreloadedPairs();

    void resetPreloadPairsInput();

};
