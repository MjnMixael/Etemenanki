/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "IgnoreFiles.h"

PreloadPairsDialog::PreloadPairsDialog(Etemenanki* m_etemenanki, QWidget* parent) : QDialog(parent), m_etemenanki(m_etemenanki) {
    ui.setupUi(this);

    ui.preload_id_line_edit->setValidator(new QIntValidator(0, INT_MAX, this));

    ui.preload_update_button->setEnabled(false);
    ui.preload_remove_button->setEnabled(false);

    int scrollBarWidth = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    int regexMainWidth = (ui.preload_table_widget->width() - m_preloadIdWidth - scrollBarWidth);
    ui.preload_table_widget->setColumnWidth(0, regexMainWidth);
    ui.preload_table_widget->setColumnWidth(1, m_preloadIdWidth);

    loadPreloadedPairs();
}

void PreloadPairsDialog::accept() {
    savePreloadedPairs();
    QDialog::accept();
}

void PreloadPairsDialog::on_preload_add_button_clicked() {
    QString text = ui.preload_text_line_edit->text();
    QString id = ui.preload_id_line_edit->text();

    if (text.isEmpty() || id.isEmpty()) {
        return;
    }

    if (addNewPair(text, id)) {
        ui.preload_text_line_edit->clear();
        ui.preload_id_line_edit->clear();
    }
}

void PreloadPairsDialog::on_preload_update_button_clicked() {
    int i = ui.preload_table_widget->currentRow();
    QString text = ui.preload_text_line_edit->text();
    QString id = ui.preload_id_line_edit->text();

    if (addNewPair(text, id, i)) {
        ui.preload_text_line_edit->clear();
        ui.preload_id_line_edit->clear();
        ui.preload_update_button->setEnabled(false);
        ui.preload_remove_button->setEnabled(false);
        ui.preload_table_widget->clearSelection();
    }
}

void PreloadPairsDialog::on_preload_remove_button_clicked() {
    ui.preload_table_widget->removeRow(ui.preload_table_widget->currentRow());
    ui.preload_text_line_edit->clear();
    ui.preload_id_line_edit->clear();
    ui.preload_update_button->setEnabled(false);
    ui.preload_remove_button->setEnabled(false);
    ui.preload_table_widget->clearSelection();
}

void PreloadPairsDialog::on_preload_clear_button_clicked() {
    ui.preload_table_widget->clear();
    ui.preload_table_widget->setRowCount(0);
    ui.preload_text_line_edit->clear();
    ui.preload_id_line_edit->clear();
    ui.preload_update_button->setEnabled(false);
    ui.preload_remove_button->setEnabled(false);
    ui.preload_table_widget->clearSelection();
}

void PreloadPairsDialog::on_preload_table_widget_clicked() {
    int i = ui.preload_table_widget->currentRow();
    if (i >= 0) {
        ui.preload_text_line_edit->setText(ui.preload_table_widget->item(i, 0)->text());
        ui.preload_id_line_edit->setText(ui.preload_table_widget->item(i, 1)->text());
        ui.preload_update_button->setEnabled(true);
        ui.preload_remove_button->setEnabled(true);
    }
}

bool PreloadPairsDialog::itemExists(QTableWidget* tableWidget, const QString& textToCheck, const QString& idToCheck) {
    QList<QTableWidgetItem*> items = tableWidget->findItems(textToCheck, Qt::MatchExactly);

    for (auto item : items) {
        int row = item->row();
        QString id = tableWidget->item(row, 1)->text();

        if (id == idToCheck) {
            return true;
        }
    }

    return false;
}

bool PreloadPairsDialog::addNewPair(QString text, QString id, int row) {
    if (text.isEmpty()) {
        return false;
    }

    if (id.isEmpty()) {
        return false;
    }

    if (row < 0) {
        if (!itemExists(ui.preload_table_widget,text, id)) {
            ui.preload_table_widget->insertRow(ui.preload_table_widget->rowCount());
            ui.preload_table_widget->setItem(ui.preload_table_widget->rowCount() - 1, 0, new QTableWidgetItem(text));
            ui.preload_table_widget->setItem(ui.preload_table_widget->rowCount() - 1, 1, new QTableWidgetItem(id));

            // Center the numeric column
            ui.preload_table_widget->item(ui.preload_table_widget->rowCount() - 1, 1)->setTextAlignment(Qt::AlignCenter);

            return true;
        }
    } else {
        ui.preload_table_widget->setItem(row, 0, new QTableWidgetItem(text));
        ui.preload_table_widget->setItem(row, 1, new QTableWidgetItem(id));
        return true;
    }

    return false;
}

void PreloadPairsDialog::loadPreloadedPairs() {
    QString filepath = m_etemenanki->m_appDataPath + m_etemenanki->m_preloadFilename;
    QDir().mkpath(QFileInfo(filepath).absolutePath()); // Ensure the directory exists
    QFile file(filepath);

    QJsonObject settings = {};
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc(QJsonDocument::fromJson(file.readAll()));
        settings = doc.object();
    }

    ui.preload_table_widget->clear();
    ui.preload_table_widget->setRowCount(0);
    QJsonArray preloadPairsArray;
    if (settings.contains("preload_pairs_list")) {
        preloadPairsArray = settings["preload_pairs_list"].toArray();
    } else {
        preloadPairsArray = {};
    }

    foreach (const QJsonValue & value, preloadPairsArray) {
        QJsonObject preloadItem = value.toObject();
        addNewPair(preloadItem["text"].toString(), preloadItem["id"].toString());
    }
}

void PreloadPairsDialog::savePreloadedPairs() {
    QString filepath = m_etemenanki->m_appDataPath + m_etemenanki->m_preloadFilename;
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_etemenanki->update_terminal_output("Unable to create Preload Pair json file!");
        return;
    }

    QJsonObject preloads;

    QJsonArray preloadPairsArray;
    for (int i = 0; i < ui.preload_table_widget->rowCount(); ++i) {
        QJsonObject preloadItem;
        preloadItem["text"] = ui.preload_table_widget->item(i, 0)->text();
        preloadItem["id"] = ui.preload_table_widget->item(i, 1)->text();
        preloadPairsArray.append(preloadItem);
    }
    preloads["preload_pairs_list"] = preloadPairsArray;

    QJsonDocument doc(preloads);
    file.write(doc.toJson());
    file.close();
}