/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "Etemenanki.h"

bool continueProcessing = false;

Etemenanki::Etemenanki(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);
    QString windowString = Title + " " + Version + " - " + Description;
    setWindowTitle(windowString);

    SettingsFileName = AppDataPath + "settings.json";
    LogFileName = AppDataPath + "Etemenanki.log";

    // Set some ui configurations
    ui.offset_line_edit->setValidator(new QIntValidator(0, 9999999, this));
    ui.position_string_line_edit->setValidator(new QIntValidator(0, 9, this));
    ui.position_id_line_edit->setValidator(new QIntValidator(0, 9, this));
    ui.regex_table_widget->setColumnWidth(0, 23);
    ui.regex_table_widget->setColumnWidth(1, 476);
    ui.regex_table_widget->setColumnWidth(2, 40);
    ui.regex_table_widget->setColumnWidth(3, 40);

    // Load user settings and/or defaults
    loadSettings();

    // Disable buttons
    ui.regex_update_button->setEnabled(false);
    ui.regex_remove_button->setEnabled(false);
    ui.files_update_button->setEnabled(false);
    ui.files_remove_button->setEnabled(false);

    //Create the processor thread
    xstrProcessor = new XstrProcessor(this);
    connect(xstrProcessor, &XstrProcessor::updateTerminalText, this, &Etemenanki::updateTerminalOutput);

    xstrProcessor->setLogFilePath(LogFileName);
}

void Etemenanki::runXSTR() {
    xstrProcessor->run();
}

void Etemenanki::uiSaveSettings() {
    saveSettings();
}

void Etemenanki::uiOpenDocumentation() {
    QDesktopServices::openUrl(Github);
}

void Etemenanki::closeEvent(QCloseEvent* event) {
    if (xstrProcessor->isRunning()) {
        continueProcessing = false;
        resetInterface();
    }

    saveSettings();
}

void Etemenanki::resetInterface() {
    ui.begin_button->setEnabled(false);
    ui.begin_button->setText("Waiting...");

    XSTR_thread->wait();
    delete XSTR_thread;

    ui.begin_button->setText("Run");
    ui.begin_button->setEnabled(true);
    toggleControls(true);
}

bool itemExists(QListWidget* listWidget, const QString& textToCheck) {
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        if (item && item->text() == textToCheck) {
            return true;  // Item already exists
        }
    }
    return false;  // Item does not exist
}

void Etemenanki::add_file_extension(QString ext) {
    if (ext.at(0) != ".") {
        ext.prepend(".");
    }

    if (!itemExists(ui.files_list_widget, ext)) {
        ui.files_list_widget->addItem(ext);
    }
}

void Etemenanki::on_files_add_button_clicked() {
    QString ext = ui.files_line_edit->text();

    add_file_extension(ext);

    ui.files_line_edit->clear();
    updateTerminalOutput("New file extension added!");
}

void Etemenanki::on_files_update_button_clicked() {
    int i = ui.files_list_widget->currentRow();
    QString ext = ui.files_line_edit->text();

    if (ext.at(0) != ".") {
        ext.prepend(".");
    }

    if (!itemExists(ui.files_list_widget, ext)) {
        ui.files_list_widget->item(i)->setText(ext);
        ui.files_line_edit->clear();
        ui.files_update_button->setEnabled(false);
        ui.files_remove_button->setEnabled(false);
        ui.files_list_widget->clearSelection();
        updateTerminalOutput("File extension updated!");
    }
}

void Etemenanki::on_files_remove_button_clicked() {
    ui.files_list_widget->takeItem(ui.files_list_widget->currentRow());
    ui.files_line_edit->clear();
    ui.files_update_button->setEnabled(false);
    ui.files_remove_button->setEnabled(false);
    ui.files_list_widget->clearSelection();
    updateTerminalOutput("File extension removed!");
}

void Etemenanki::on_files_list_widget_clicked() {
    int i = ui.files_list_widget->currentRow();
    ui.files_line_edit->setText(ui.files_list_widget->item(i)->text());
    ui.files_update_button->setEnabled(true);
    ui.files_remove_button->setEnabled(true);
}

bool itemExists(QTableWidget* tableWidget, const QString& textToCheck) {
    QList<QTableWidgetItem*> items = tableWidget->findItems(textToCheck, Qt::MatchExactly);

    return !items.isEmpty();  // If items list is not empty, the item exists
}

bool Etemenanki::add_regex_row(QString pattern, QString string_pos, QString id_pos, bool checked, int row) {
    if (pattern.isEmpty()) {
        return false;
    }

    if (string_pos.isEmpty()) {
        string_pos = "1";
    }

    if (id_pos.isEmpty()) {
        id_pos = "2";
    }

    // Try to validate the regex
    std::regex reg;
    try {
        reg.assign(pattern.toStdString());
    }
    catch (const std::regex_error& e) {
        //std::cerr << "Regex error: " << e.what() << std::endl;
        updateTerminalOutput("Error validating regular expression!");
        return false;
    }

    if (row < 0) {
        if (!itemExists(ui.regex_table_widget, pattern)) {
            ui.regex_table_widget->insertRow(ui.regex_table_widget->rowCount());
            ui.regex_table_widget->setItem(ui.regex_table_widget->rowCount() - 1, 1, new QTableWidgetItem(pattern));
            ui.regex_table_widget->setItem(ui.regex_table_widget->rowCount() - 1, 2, new QTableWidgetItem(string_pos));
            ui.regex_table_widget->setItem(ui.regex_table_widget->rowCount() - 1, 3, new QTableWidgetItem(id_pos));

            // Center the numeric columns
            ui.regex_table_widget->item(ui.regex_table_widget->rowCount() - 1, 2)->setTextAlignment(Qt::AlignCenter);
            ui.regex_table_widget->item(ui.regex_table_widget->rowCount() - 1, 3)->setTextAlignment(Qt::AlignCenter);

            // Add a checkbox
            QTableWidgetItem* checkBoxItem = new QTableWidgetItem();
            if (checked) {
                checkBoxItem->setCheckState(Qt::Checked);
            } else {
                checkBoxItem->setCheckState(Qt::Unchecked);
            }
            ui.regex_table_widget->setItem(ui.regex_table_widget->rowCount() - 1, 0, checkBoxItem);
            return true;
        }
    }
    else {
        ui.regex_table_widget->setItem(row, 1, new QTableWidgetItem(pattern));
        ui.regex_table_widget->setItem(row, 2, new QTableWidgetItem(string_pos));
        ui.regex_table_widget->setItem(row, 3, new QTableWidgetItem(id_pos));
        return true;
    }

    return false;
}

void Etemenanki::on_regex_add_button_clicked() {
    QString regex = ui.regex_line_edit->text();
    QString string_pos = ui.position_string_line_edit->text();
    QString id_pos = ui.position_id_line_edit->text();

    if (add_regex_row(regex, string_pos, id_pos)) {
        ui.regex_line_edit->clear();
        ui.position_string_line_edit->clear();
        ui.position_id_line_edit->clear();
        updateTerminalOutput("New regular expression added!");
    }
}

void Etemenanki::on_regex_update_button_clicked() {
    int i = ui.regex_table_widget->currentRow();
    QString regex = ui.regex_line_edit->text();
    QString string_pos = ui.position_string_line_edit->text();
    QString id_pos = ui.position_id_line_edit->text();
    bool checked = isRowChecked(i);

    if (add_regex_row(regex, string_pos, id_pos, checked, i)) {
        ui.regex_line_edit->clear();
        ui.position_string_line_edit->clear();
        ui.position_id_line_edit->clear();
        ui.regex_update_button->setEnabled(false);
        ui.regex_remove_button->setEnabled(false);
        ui.regex_table_widget->clearSelection();
        updateTerminalOutput("Regular expression updated!");
    }
}

void Etemenanki::on_regex_remove_button_clicked() {
    ui.regex_table_widget->removeRow(ui.regex_table_widget->currentRow());
    ui.regex_line_edit->clear();
    ui.position_string_line_edit->clear();
    ui.position_id_line_edit->clear();
    ui.regex_update_button->setEnabled(false);
    ui.regex_remove_button->setEnabled(false);
    ui.regex_table_widget->clearSelection();
    updateTerminalOutput("Regular expression removed!");
}

void Etemenanki::on_regex_table_widget_clicked() {
    int i = ui.regex_table_widget->currentRow();
    if (i >= 0) {
        ui.regex_line_edit->setText(ui.regex_table_widget->item(i, 1)->text());
        ui.position_string_line_edit->setText(ui.regex_table_widget->item(i, 2)->text());
        ui.position_id_line_edit->setText(ui.regex_table_widget->item(i, 3)->text());
        ui.regex_update_button->setEnabled(true);
        ui.regex_remove_button->setEnabled(true);
    }
}

bool Etemenanki::isRowChecked(int row) {
    QTableWidgetItem* item = ui.regex_table_widget->item(row, 0);
    if (item) {
        return item->checkState() == Qt::Checked;
    }
    return false;
}

void Etemenanki::on_begin_button_clicked() {
    if (xstrProcessor->isRunning()) {
        continueProcessing = false;
        resetInterface();
        return;
    }

    xstrProcessor->clearVectors();
    
    // Get values from UI
    QString directory = ui.directory_line_edit->text();
    QString outputFile = ui.output_file_line_edit->text();
    QString outputDirectory = ui.output_directory_line_edit->text();
    QString offset = ui.offset_line_edit->text();
    bool replace = ui.replace_radio_button->isChecked();

    // Validate values
    if (directory.isEmpty()) {
        ui.terminal_output->setText("No directory provided!");
        return;
    }

    if (outputFile.isEmpty()) {
        ui.terminal_output->setText("Output not provided. Using default!");
        outputFile = "tstrings.tbl";
        return;
    }

    if (offset.isEmpty()) {
        ui.terminal_output->setText("Offset not provided. Setting to 0!");
        offset = "1";
        return;
    }
    
    // Set internal values
    xstrProcessor->setInputPath(directory.toStdString());
    xstrProcessor->setOutputFilepath(outputDirectory.toStdString());
    xstrProcessor->setOutputFilename(outputFile.toStdString());
    xstrProcessor->setOffset(offset.toInt());

    for (int i = 0; i < ui.files_list_widget->count(); ++i) {
        std::string ext = ui.files_list_widget->item(i)->text().toStdString();
        xstrProcessor->addFileExtension(ext);
    }

    for (int i = 0; i < ui.regex_table_widget->rowCount(); ++i) {
        if (isRowChecked(i)) {
            std::string pattern = ui.regex_table_widget->item(i, 1)->text().toStdString();
            std::string string_pos = ui.regex_table_widget->item(i, 2)->text().toStdString();
            std::string id_pos = ui.regex_table_widget->item(i, 3)->text().toStdString();

            int pos = std::stoi(string_pos);
            int id = std::stoi(id_pos);

            xstrProcessor->addRegexPattern(pattern, pos, id, i);
        }
    }

    saveSettings();

    qDebug() << "Processing in thread:" << QThread::currentThreadId();

    auto function = std::bind(&Etemenanki::runXSTR, this);
    XSTR_thread = QThread::create(function);

    connect(XSTR_thread, &QThread::finished, this, [this]() {
        continueProcessing = false;
        resetInterface();
        });

    XSTR_thread->start();

    ui.begin_button->setText("Terminate");
    
    toggleControls(false);
}

void Etemenanki::toggleControls(bool val) {
    ui.directory_line_edit->setEnabled(val);
    ui.files_line_edit->setEnabled(val);
    ui.files_list_widget->setEnabled(val);
    ui.files_add_button->setEnabled(val);
    ui.files_update_button->setEnabled(val);
    ui.files_remove_button->setEnabled(val);
    ui.regex_line_edit->setEnabled(val);
    ui.position_string_line_edit->setEnabled(val);
    ui.position_id_line_edit->setEnabled(val);
    ui.regex_table_widget->setEnabled(val);
    ui.regex_add_button->setEnabled(val);
    ui.regex_update_button->setEnabled(val);
    ui.regex_remove_button->setEnabled(val);
    ui.offset_line_edit->setEnabled(val);
    ui.replace_radio_button->setEnabled(val);
    ui.output_directory_line_edit->setEnabled(val);
    ui.output_file_line_edit->setEnabled(val);
}

void Etemenanki::updateTerminalOutput(const QString& text) {
    ui.terminal_output->setText(text);
}

void Etemenanki::loadSettings() {
    QDir().mkpath(QFileInfo(SettingsFileName).absolutePath()); // Ensure the directory exists
    QFile file(SettingsFileName);
    
    QJsonObject settings = {};
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc(QJsonDocument::fromJson(file.readAll()));
        settings = doc.object();
    }  
    
    ui.output_file_line_edit->setText(settings.value("outputFile").toString(defaultOutputFile));
    ui.output_directory_line_edit->setText(settings.value("outputDirectory").toString(defaultOutputDirectory));
    ui.directory_line_edit->setText(settings["directory"].toString());
    ui.offset_line_edit->setText(settings.value("offset").toString(defaultOffset));
    ui.replace_radio_button->setChecked(settings.value("replaceValues").toBool(defaultReplacement));

    ui.files_list_widget->clear(); // Clear existing items before loading
    QJsonArray extensionsArray;
    if (settings.contains("file_extensions")) {
        extensionsArray = settings["file_extensions"].toArray();
    }
    else {
        for (const QString& ext : defaultExtensions) {
            extensionsArray.append(ext);
        }
    }

    for (const QJsonValue& value : extensionsArray) {
        add_file_extension(value.toString());
    }

    QJsonArray regexArray;
    if (settings.contains("regex_rules")) {
        regexArray = settings["regex_rules"].toArray();
    }
    else {
        regexArray.append(defaultRegex());
    }

    foreach(const QJsonValue & value, regexArray) {
        QJsonObject regexItem = value.toObject();
        add_regex_row(regexItem["regex_string"].toString(), QString::number(regexItem["string_position"].toInt()), QString::number(regexItem["id_position"].toInt()), regexItem["checked"].toBool());
    }

    file.close();
}

void Etemenanki::saveSettings() {
    QFile file(SettingsFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        updateTerminalOutput("Saving failed. Couldn't open save file...");
        return;
    }

    QJsonObject settings;
    settings["outputFile"] = ui.output_file_line_edit->text();
    settings["outputDirectory"] = ui.output_directory_line_edit->text();
    settings["directory"] = ui.directory_line_edit->text();
    settings["offset"] = ui.offset_line_edit->text();
    settings["replaceValues"] = ui.replace_radio_button->isChecked();

    QJsonArray extensionsArray;
    for (int i = 0; i < ui.files_list_widget->count(); ++i) {
        extensionsArray.append(ui.files_list_widget->item(i)->text());
    }
    settings["file_extensions"] = extensionsArray;

    QJsonArray regexArray;
    for (int i = 0; i < ui.regex_table_widget->rowCount(); ++i) {
        QJsonObject regexItem;
        regexItem["regex_string"] = ui.regex_table_widget->item(i, 1)->text();
        regexItem["string_position"] = ui.regex_table_widget->item(i, 2)->text().toInt();
        regexItem["id_position"] = ui.regex_table_widget->item(i, 3)->text().toInt();
        regexItem["checked"] = isRowChecked(i);
        regexArray.append(regexItem);
    }
    settings["regex_rules"] = regexArray;

    QJsonDocument doc(settings);
    file.write(doc.toJson());
    file.close();
    updateTerminalOutput("Settings saved!");
}