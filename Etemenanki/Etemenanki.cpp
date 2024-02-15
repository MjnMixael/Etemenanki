/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#include "Etemenanki.h"

bool g_continueProcessing = false;

Etemenanki::Etemenanki(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);
    QString windowString = m_title + " " + m_version + " - " + m_description;
    setWindowTitle(windowString);

    m_settingsFilePath = m_appDataPath + m_settingsFileName;
    m_logFilePath = m_appDataPath + m_logFileName;

    // Set some ui configurations
    ui.offset_line_edit->setValidator(new QIntValidator(0, INT_MAX, this));
    ui.position_string_line_edit->setValidator(new QIntValidator(0, 9, this));
    ui.position_id_line_edit->setValidator(new QIntValidator(0, 9, this));
    ui.regex_table_widget->setColumnWidth(0, m_regexCheckWidth);
    int scrollBarWidth = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    int regexMainWidth = (ui.regex_table_widget->width() - ((m_regexPositionWidth * 2) + m_regexCheckWidth + scrollBarWidth));
    ui.regex_table_widget->setColumnWidth(1, regexMainWidth);
    ui.regex_table_widget->setColumnWidth(2, m_regexPositionWidth);
    ui.regex_table_widget->setColumnWidth(3, m_regexPositionWidth);

    // Load user settings and/or defaults
    loadSettings();

    // Disable buttons
    ui.regex_update_button->setEnabled(false);
    ui.regex_remove_button->setEnabled(false);
    ui.files_update_button->setEnabled(false);
    ui.files_remove_button->setEnabled(false);

    // Maybe disable offset
    toggleOffsetControl(!m_fillInIds);

    // Set the read only checkbox
    ui.read_only_checkbox->setChecked(m_readOnly);

    //Create the processor thread
    m_xstrProcessor = new XstrProcessor(this);
    connect(m_xstrProcessor, &XstrProcessor::update_terminal_text, this, &Etemenanki::update_terminal_output);

    m_xstrProcessor->setLogFilePath(m_logFilePath);

    update_terminal_output("Ready!");
}

void Etemenanki::run_xstr() {
    m_xstrProcessor->run();
}

void Etemenanki::ui_save_settings() {
    saveSettings();
}

void Etemenanki::ui_open_documentation() {
    QDesktopServices::openUrl(m_githubUrl);
}

void Etemenanki::ui_open_preferences() {
    SettingsDialog dialog(this, this);
    dialog.exec();
}

void Etemenanki::ui_open_ignore_files() {
    IgnoreFilesDialog dialog(this, this);
    dialog.exec();
}

void Etemenanki::ui_open_ignore_ids() {
    IgnoreIdsDialog dialog(this, this);
    dialog.exec();
}

void Etemenanki::ui_open_preload_pairs() {
    PreloadPairsDialog dialog(this, this);
    dialog.exec();
}

void Etemenanki::closeEvent(QCloseEvent* event) {
    if (m_xstrProcessor->isRunning()) {
        g_continueProcessing = false;
        resetInterface();
    }

    saveSettings();
}

void Etemenanki::resetInterface() {
    ui.begin_button->setEnabled(false);
    ui.begin_button->setText("Waiting...");

    m_xstrThread->wait();
    delete m_xstrThread;

    ui.begin_button->setText("Run");
    ui.begin_button->setEnabled(true);
    toggleControls(true);
}

void Etemenanki::setComprehensive(bool val) {
    m_comprehensiveScan = val;
}
bool Etemenanki::getComprehensive() {
    return m_comprehensiveScan;
}

void Etemenanki::setFillInIds(bool val) {
    m_fillInIds = val;
}
bool Etemenanki::getFillInIds() {
    return m_fillInIds;
}

void Etemenanki::setSortingType(int val) {
    m_sortingType = val;
}
int Etemenanki::getSortingType() {
    return m_sortingType;
}

void Etemenanki::setHeaderAnnotations(bool val) {
    m_headerAnnotations = val;
}
bool Etemenanki::getHeaderAnnotations() {
    return m_headerAnnotations;
}

void Etemenanki::setVerboseAnnotations(bool val) {
    m_verboseAnnotations = val;
}
bool Etemenanki::getVerboseAnnotations() {
    return m_verboseAnnotations;
}

void Etemenanki::setCaseInsensitive(bool val) {
    m_caseInsensitive = val;
}
bool Etemenanki::getCaseInsensitive() {
    return m_caseInsensitive;
}

void Etemenanki::toggleOffsetControl(bool val) {
    ui.offset_line_edit->setEnabled(val);
}

QString Etemenanki::getCurrentDirectory() {
    return ui.directory_line_edit->text();
}

void Etemenanki::on_directory_browse_button_clicked() {
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
        ui.directory_line_edit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty()) {
        ui.directory_line_edit->setText(directory);
    }
}

void Etemenanki::on_output_directory_browse_button_clicked() {
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
        ui.output_directory_line_edit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty()) {
        ui.output_directory_line_edit->setText(directory);
    }
}

bool Etemenanki::itemExists(QListWidget* listWidget, const QString& textToCheck) {
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        if (item && item->text() == textToCheck) {
            return true;  // Item already exists
        }
    }
    return false;  // Item does not exist
}

bool Etemenanki::itemExists(QTableWidget* tableWidget, const QString& textToCheck) {
    QList<QTableWidgetItem*> items = tableWidget->findItems(textToCheck, Qt::MatchExactly);

    return !items.isEmpty();  // If items list is not empty, the item exists
}

void Etemenanki::addFileExtension(QString ext) {
    if (ext.at(0) != ".") {
        ext.prepend(".");
    }

    if (!itemExists(ui.files_list_widget, ext)) {
        ui.files_list_widget->addItem(ext);
        ui.files_line_edit->clear();
        update_terminal_output("New file extension added!");
    }
}

void Etemenanki::resetFileExtensionInput() {
    ui.files_line_edit->clear();
    ui.files_update_button->setEnabled(false);
    ui.files_remove_button->setEnabled(false);
    ui.files_list_widget->clearSelection();
}

void Etemenanki::on_files_add_button_clicked() {
    QString ext = ui.files_line_edit->text();

    if (ext.isEmpty()) {
        return;
    }

    addFileExtension(ext);
}

void Etemenanki::on_files_update_button_clicked() {
    int i = ui.files_list_widget->currentRow();
    QString ext = ui.files_line_edit->text();

    if (ext.isEmpty()) {
        resetFileExtensionInput();
        return;
    }

    if (ext.at(0) != ".") {
        ext.prepend(".");
    }

    if (!itemExists(ui.files_list_widget, ext)) {
        ui.files_list_widget->item(i)->setText(ext);
        resetFileExtensionInput();
        update_terminal_output("File extension updated!");
    }
}

void Etemenanki::on_files_remove_button_clicked() {
    ui.files_list_widget->takeItem(ui.files_list_widget->currentRow());
    resetFileExtensionInput();
    update_terminal_output("File extension removed!");
}

void Etemenanki::on_files_clear_button_clicked() {
    ui.files_list_widget->clear();
    resetFileExtensionInput();
    update_terminal_output("File extensions cleared!");
}

void Etemenanki::on_files_list_widget_clicked() {
    int i = ui.files_list_widget->currentRow();
    ui.files_line_edit->setText(ui.files_list_widget->item(i)->text());
    ui.files_update_button->setEnabled(true);
    ui.files_remove_button->setEnabled(true);
}

bool Etemenanki::addRegexRow(QString pattern, QString string_pos, QString id_pos, bool checked, int row) {
    if (pattern.isEmpty()) {
        return false;
    }

    if (string_pos.isEmpty()) {
        string_pos = "1";
    }

    if (id_pos.isEmpty()) {
        id_pos = "2";
    }

    if (string_pos.toInt() < 0) {
        return false;
    }

    if (id_pos.toInt() < 0) {
        return false;
    }

    // Try to validate the regex
    std::regex reg;
    try {
        reg.assign(pattern.toStdString());
    }
    catch (const std::regex_error& e) {
        //std::cerr << "Regex error: " << e.what() << std::endl;
        update_terminal_output("Error validating regular expression!");
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

void Etemenanki::resetRegexInput() {
    ui.regex_line_edit->clear();
    ui.position_string_line_edit->clear();
    ui.position_id_line_edit->clear();
    ui.regex_update_button->setEnabled(false);
    ui.regex_remove_button->setEnabled(false);
    ui.regex_table_widget->clearSelection();
}

void Etemenanki::on_regex_add_button_clicked() {
    QString regex = ui.regex_line_edit->text();
    QString string_pos = ui.position_string_line_edit->text();
    QString id_pos = ui.position_id_line_edit->text();

    if (addRegexRow(regex, string_pos, id_pos)) {
        ui.regex_line_edit->clear();
        ui.position_string_line_edit->clear();
        ui.position_id_line_edit->clear();
        update_terminal_output("New regular expression added!");
    }
}

void Etemenanki::on_regex_update_button_clicked() {
    int i = ui.regex_table_widget->currentRow();
    QString regex = ui.regex_line_edit->text();
    QString string_pos = ui.position_string_line_edit->text();
    QString id_pos = ui.position_id_line_edit->text();
    bool checked = isRowChecked(i);

    if (regex.isEmpty() || string_pos.isEmpty() || id_pos.isEmpty()) {
        resetRegexInput();
        return;
    }

    if (addRegexRow(regex, string_pos, id_pos, checked, i)) {
        resetRegexInput();
        update_terminal_output("Regular expression updated!");
    }
}

void Etemenanki::on_regex_remove_button_clicked() {
    ui.regex_table_widget->removeRow(ui.regex_table_widget->currentRow());
    resetRegexInput();
    update_terminal_output("Regular expression removed!");
}

void Etemenanki::on_regex_clear_button_clicked() {
    ui.regex_table_widget->clear();
    ui.regex_table_widget->setRowCount(0);
    resetRegexInput();
    update_terminal_output("Regular expressions cleared!");
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

void Etemenanki::on_read_only_checkbox_clicked() {
    m_readOnly = ui.read_only_checkbox->isChecked();
}

void Etemenanki::loadPreloadedPairs() {
    QString filepath = m_appDataPath + m_preloadFilename;
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

    QJsonArray preloadPairsArray;
    if (settings.contains("preload_pairs_list")) {
        preloadPairsArray = settings["preload_pairs_list"].toArray();
    } else {
        preloadPairsArray = {};
    }

    m_preloadedPairs.clear();
    foreach(const QJsonValue & value, preloadPairsArray) {
        QJsonObject preloadItem = value.toObject();

        XstrPair pair;
        pair.text = preloadItem["text"].toString().toStdString();
        pair.id = preloadItem["id"].toString().toInt();
        pair.files.push_back("Preloaded Pair");
        pair.locked = true;

        if (pair.id < 0) {
            continue;
        }

        m_preloadedPairs.push_back(pair);
    }

    waitDialog.accept();
}

void Etemenanki::on_begin_button_clicked() {
    if (m_xstrProcessor->isRunning()) {
        ui.begin_button->setEnabled(false); // Prevent double clicks
        g_continueProcessing = false;
        update_terminal_output("Terminate requested! Please wait...");
        return;
    }

    loadPreloadedPairs();

    m_xstrProcessor->clearVectors();
    
    // Get values from UI
    QString directory = ui.directory_line_edit->text();
    QString outputFile = ui.output_file_line_edit->text();
    QString outputDirectory = ui.output_directory_line_edit->text();
    QString offset = ui.offset_line_edit->text();
    bool replace = ui.replace_checkbox->isChecked();

    // Validate values
    if (directory.isEmpty()) {
        update_terminal_output("No directory provided!");
        return;
    }

    if (outputFile.isEmpty()) {
        update_terminal_output("Output not provided. Using default!");
        outputFile = "tstrings.tbl";
        return;
    }

    if (offset.isEmpty()) {
        update_terminal_output("Offset not provided. Setting to 0!");
        offset = "1";
        return;
    }
    
    // Set internal values
    m_xstrProcessor->setInputPath(directory.toStdString());
    m_xstrProcessor->setOutputFilepath(outputDirectory.toStdString());
    m_xstrProcessor->setOutputFilename(outputFile.toStdString());
    m_xstrProcessor->setOffset(offset.toInt());
    m_xstrProcessor->setReplaceExisting(replace);
    m_xstrProcessor->setComprehensiveScan(m_comprehensiveScan);
    m_xstrProcessor->setFillEmptyIds(m_fillInIds);
    m_xstrProcessor->setSortingType(m_sortingType);
    m_xstrProcessor->setHeadersToggle(m_headerAnnotations);
    m_xstrProcessor->setAnnotationsToggle(m_verboseAnnotations);
    m_xstrProcessor->setReadOnlyToggle(m_readOnly);
    m_xstrProcessor->setCaseInsensitiveToggle(m_caseInsensitive);

    for (auto pair : m_preloadedPairs) {
        if (!m_xstrProcessor->addPreloadPair(pair)) {
            QString msg = "Pair ID '" + QString::number(pair.id) + "' is duplicated. Skipping!";
            QMessageBox::information(
                this,
                "Duplicate ID!",
                msg
            );
        }
    }

    for (int i = 0; i < ui.files_list_widget->count(); ++i) {
        std::string ext = ui.files_list_widget->item(i)->text().toStdString();
        m_xstrProcessor->addFileExtension(ext);
    }

    for (int i = 0; i < ui.regex_table_widget->rowCount(); ++i) {
        if (isRowChecked(i)) {
            std::string pattern = ui.regex_table_widget->item(i, 1)->text().toStdString();
            std::string stringPos = ui.regex_table_widget->item(i, 2)->text().toStdString();
            std::string idPos = ui.regex_table_widget->item(i, 3)->text().toStdString();

            int pos = std::stoi(stringPos);
            int id = std::stoi(idPos);

            m_xstrProcessor->addRegexPattern(pattern, pos, id, i);
        }
    }

    for (int i = 0; i < m_ignoredFilesList.size(); i++) {
        std::string path = m_ignoredFilesList[i].toStdString();
        m_xstrProcessor->addIgnoredFile(path);
    }

    for (int i = 0; i < m_ignoredIdsList.size(); i++) {
        int id = m_ignoredIdsList[i].toInt();
        m_xstrProcessor->addIgnoredId(id);
    }

    saveSettings();

    if (m_xstrProcessor->getNumFileExtensions() <= 0) {
        update_terminal_output("No file extensions provided!");
        return;
    }

    if (m_xstrProcessor->getNumRegexPatterns() <= 0) {
        update_terminal_output("No active regex patterns provided!");
        return;
    }

    qDebug() << "Processing in thread:" << QThread::currentThreadId();

    auto function = std::bind(&Etemenanki::run_xstr, this);
    m_xstrThread = QThread::create(function);

    connect(m_xstrThread, &QThread::finished, this, [this]() {
        g_continueProcessing = false;
        resetInterface();
        });

    m_xstrThread->start();

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
    if (!m_fillInIds) {
        ui.offset_line_edit->setEnabled(val);
    }
    ui.replace_checkbox->setEnabled(val);
    ui.output_directory_line_edit->setEnabled(val);
    ui.output_file_line_edit->setEnabled(val);
    ui.actionPreferences->setEnabled(val);
    ui.read_only_checkbox->setEnabled(val);
    ui.actionIgnore_Files->setEnabled(val);
}

void Etemenanki::update_terminal_output(const QString& text) {
    ui.terminal_output->setText(text);
}

void Etemenanki::loadSettings() {
    QDir().mkpath(QFileInfo(m_settingsFilePath).absolutePath()); // Ensure the directory exists
    QFile file(m_settingsFilePath);
    
    QJsonObject settings = {};
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc(QJsonDocument::fromJson(file.readAll()));
        settings = doc.object();
    }  
    
    ui.output_file_line_edit->setText(settings.value("outputFile").toString(m_defaultOutputFile));
    ui.output_directory_line_edit->setText(settings.value("outputDirectory").toString(m_defaultOutputDirectory));
    ui.directory_line_edit->setText(settings["directory"].toString());
    ui.offset_line_edit->setText(settings.value("offset").toString(m_defaultOffset));
    ui.replace_checkbox->setChecked(settings.value("replaceValues").toBool(m_defaultReplacement));
    m_comprehensiveScan = settings.value("comprehensive").toBool(m_comprehensiveScan);
    m_fillInIds = settings.value("fillInIds").toBool(m_fillInIds);
    m_sortingType = settings.value("sortingType").toInt(m_sortingType);
    m_headerAnnotations = settings.value("headerAnnotations").toBool(m_headerAnnotations);
    m_verboseAnnotations = settings.value("verboseAnnotations").toBool(m_verboseAnnotations);
    m_readOnly = settings.value("readOnlyMode").toBool(m_readOnly);
    m_caseInsensitive = settings.value("caseInsensitive").toBool(m_caseInsensitive);

    ui.files_list_widget->clear(); // Clear existing items before loading
    QJsonArray extensionsArray;
    if (settings.contains("file_extensions")) {
        extensionsArray = settings["file_extensions"].toArray();
    } else {
        for (const QString& ext : m_defaultExtensions) {
            extensionsArray.append(ext);
        }
    }

    for (const QJsonValue& value : extensionsArray) {
        addFileExtension(value.toString());
    }

    ui.regex_table_widget->clear();
    QJsonArray regexArray;
    if (settings.contains("regex_rules")) {
        regexArray = settings["regex_rules"].toArray();
    } else {
        regexArray.append(m_defaultRegex());
    }

    foreach (const QJsonValue& value, regexArray) {
        QJsonObject regexItem = value.toObject();
        addRegexRow(regexItem["regex_string"].toString(), QString::number(regexItem["string_position"].toInt()), QString::number(regexItem["id_position"].toInt()), regexItem["checked"].toBool());
    }

    m_ignoredFilesList.clear();
    QJsonArray ignoredFilesArray;
    if (settings.contains("ignored_files_list")) {
        ignoredFilesArray = settings["ignored_files_list"].toArray();
    } else {
        ignoredFilesArray = {};
    }

    for (const QJsonValue& value : ignoredFilesArray) {
        m_ignoredFilesList.push_back(value.toString());
    }

    m_ignoredIdsList.clear();
    QJsonArray ignoredIdsArray;
    if (settings.contains("ignored_ids_list")) {
        ignoredIdsArray = settings["ignored_ids_list"].toArray();
    } else {
        ignoredIdsArray = {};
    }

    for (const QJsonValue& value : ignoredIdsArray) {
        if (value.toString().toInt() > 0) {
            m_ignoredIdsList.push_back(value.toString());
        }
    }

    file.close();
}

void Etemenanki::saveSettings() {
    QFile file(m_settingsFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        update_terminal_output("Saving failed. Couldn't open save file...");
        return;
    }

    QJsonObject settings;
    settings["outputFile"] = ui.output_file_line_edit->text();
    settings["outputDirectory"] = ui.output_directory_line_edit->text();
    settings["directory"] = ui.directory_line_edit->text();
    settings["offset"] = ui.offset_line_edit->text();
    settings["replaceValues"] = ui.replace_checkbox->isChecked();
    settings["comprehensive"] = m_comprehensiveScan;
    settings["fillInIds"] = m_fillInIds;
    settings["sortingType"] = m_sortingType;
    settings["headerAnnotations"] = m_headerAnnotations;
    settings["verboseAnnotations"] = m_verboseAnnotations;
    settings["readOnlyMode"] = m_readOnly;
    settings["caseInsensitive"] = m_caseInsensitive;

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

    QJsonArray ignoredFilesArray;
    for (int i = 0; i < m_ignoredFilesList.size(); i++) {
        ignoredFilesArray.append(m_ignoredFilesList[i]);
    }
    settings["ignored_files_list"] = ignoredFilesArray;

    QJsonArray ignoredIdsArray;
    for (int i = 0; i < m_ignoredIdsList.size(); i++) {
        ignoredIdsArray.append(m_ignoredIdsList[i]);
    }
    settings["ignored_ids_list"] = ignoredIdsArray;

    QJsonDocument doc(settings);
    file.write(doc.toJson());
    file.close();
    update_terminal_output("Settings saved!");
}