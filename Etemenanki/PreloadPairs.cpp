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

void PreloadPairsDialog::resetPreloadPairsInput() {
    ui.preload_text_line_edit->clear();
    ui.preload_id_line_edit->clear();
    ui.preload_update_button->setEnabled(false);
    ui.preload_remove_button->setEnabled(false);
    ui.preload_table_widget->clearSelection();
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

    if (text.isEmpty() || id.isEmpty()) {
        resetPreloadPairsInput();
        return;
    }

    if (addNewPair(text, id, i)) {
        resetPreloadPairsInput();
    }
}

void PreloadPairsDialog::on_preload_remove_button_clicked() {
    ui.preload_table_widget->removeRow(ui.preload_table_widget->currentRow());
    resetPreloadPairsInput();
}

void PreloadPairsDialog::on_preload_clear_button_clicked() {
    ui.preload_table_widget->clear();
    ui.preload_table_widget->setRowCount(0);
    resetPreloadPairsInput();
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

void PreloadPairsDialog::on_preload_import_button_clicked() {
    QString dir = QDir::homePath();
    if (m_etemenanki) {
        QString current = m_etemenanki->getCurrentDirectory();
        if (current.length() > 0) {
            dir = current;
        }
    }

    QString filename = QFileDialog::getOpenFileName(this, tr("Select Tstrings.tbl"),
        dir,
        tr("Table Files (*.tbl)")
    );
    if (!filename.isEmpty()) {
        parseTstringsTable(filename.toStdString());
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

    if (id.toInt() < 0) {
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

    // Show a dialog while we process
    QMessageBox waitDialog;
    waitDialog.setWindowTitle("Processing");
    waitDialog.setText("Please wait, parsing xstr_pairs.json file...");
    waitDialog.setStandardButtons(0); // No buttons
    waitDialog.show();

    QApplication::processEvents();

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

    waitDialog.accept();
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

void PreloadPairsDialog::parseTstringsTable(const std::string& filename) {
    std::ifstream file(filename);

    // Check and skip BOM if present
    char bom[3];
    file.read(bom, 3);
    if (!(bom[0] == (char)0xEF && bom[1] == (char)0xBB && bom[2] == (char)0xBF)) {
        // No BOM, seek back to the beginning of the file
        file.seekg(0);
    }

    std::string line, processedLine;
    bool inDefaultSection = false;

    // Clear the table
    on_preload_clear_button_clicked();

    // Show a dialog while we process
    QMessageBox waitDialog;
    waitDialog.setWindowTitle("Processing");
    waitDialog.setText("Please wait, parsing file...");
    waitDialog.setStandardButtons(0); // No buttons
    waitDialog.show();

    QApplication::processEvents();

    while (std::getline(file, line)) {
        // Skip comments, empty lines, and non-default sections
        if (line.empty() || line.substr(0, 2) == ";;" || (line[0] == '#' && line != "#default" && !inDefaultSection)) {
            continue;
        }
        if (line == "#default") {
            inDefaultSection = true;
            continue;
        } else if (line == "#end") {
            inDefaultSection = false;
            continue;
        }
        if (!inDefaultSection) continue;

        // Extract the integer before the comma
        std::istringstream iss(line);
        int number;
        char comma;
        if (!(iss >> number >> comma)) continue; // Skip if no number and comma

        // Skip to the first quote
        iss.ignore(std::numeric_limits<std::streamsize>::max(), '"');

        std::string accumulatedText;
        // Start capturing text immediately after the first quote within the same line
        std::getline(iss, accumulatedText);

        // If the accumulated text does not end with a quote, it means the string spans multiple lines
        if (!accumulatedText.empty() && accumulatedText.back() != '"') {
            // Remove the potential partial quote at the end if it exists
            if (accumulatedText.back() == '"') {
                accumulatedText.pop_back();
            }

            // Read and accumulate the rest of the text until the closing quote is found
            do {
                std::string nextLine;
                if (std::getline(file, nextLine)) {
                    // Check if the next line contains the closing quote
                    size_t quotePos = nextLine.find('"');
                    if (quotePos != std::string::npos) {
                        // Append up to the closing quote and stop
                        accumulatedText += "\n" + nextLine.substr(0, quotePos);
                        break;
                    } else {
                        // If no closing quote, append the whole line and continue
                        accumulatedText += "\n" + nextLine;
                    }
                }
            } while (!file.eof());
        } else if (!accumulatedText.empty()) {
            // If the string ends with a quote, remove it
            accumulatedText.pop_back();
        }

        addNewPair(QString::fromStdString(accumulatedText), QString::number(number));
    }

    waitDialog.accept();
}