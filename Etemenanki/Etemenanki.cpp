#include "Etemenanki.h"
#include "xstr.h"

#include <regex>

bool continueProcessing = false;

Etemenanki::Etemenanki(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    ui.offset_line_edit->setValidator(new QIntValidator(0, 9999999, this));
    ui.position_string_line_edit->setValidator(new QIntValidator(0, 9, this));
    ui.position_id_line_edit->setValidator(new QIntValidator(0, 9, this));
    ui.regex_table_widget->setColumnWidth(0, 500);
    ui.regex_table_widget->setColumnWidth(1, (ui.regex_table_widget->width() - 500 - 2)/2);
    ui.regex_table_widget->setColumnWidth(2, (ui.regex_table_widget->width() - 500 - 2) / 2);

    // Default values
    ui.output_line_edit->setText("tstrings.tbl");
    ui.directory_line_edit->setText("C:\\Games\\FreespaceOpen\\FS2\\BtA-2.0.0\\bta_core\\data\\config");
    ui.offset_line_edit->setText("0");
    add_regex_row("XSTR\\(\"([^\"]+)\",\\s*(-?\\d+)\\)", "1", "2");
    add_regex_row("([a-zA-Z0-9_]+)\\s*=\\s*\\{\\s*\"([^\"]+)\",\\s*(-?\\d+)\\}", "2", "3");
    add_regex_row("utils\\.xstr\\(\\{\\s*\"([^\"]+)\",\\s*(-?\\d+)\\}\\)", "1","2");
    add_regex_row("\\+Val:\\s*([a-zA-Z0-9_]+)\\s*\\(\\s*\"([^\"]+)\",\\s*(-?\\d+)\\)", "2","3");
    add_regex_row("\\[\"([^\"]+)\",\\s*(-?\\d+)\\]", "1","2");
    ui.files_list_widget->addItem(".tbl");
    ui.files_list_widget->addItem(".tbm");
    ui.files_list_widget->addItem(".fs2");
    ui.files_list_widget->addItem(".fc2");
    ui.files_list_widget->addItem(".lua");
    ui.files_list_widget->addItem(".cfg");

    // Disable buttons
    ui.regex_update_button->setEnabled(false);
    ui.regex_remove_button->setEnabled(false);
    ui.files_update_button->setEnabled(false);
    ui.files_remove_button->setEnabled(false);

    xstrProcessor = new XstrProcessor(this);
    connect(xstrProcessor, &XstrProcessor::updateTerminalText, this, &Etemenanki::updateTerminalOutput);
}

void Etemenanki::runXSTR() {
    xstrProcessor->run();
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

void Etemenanki::on_files_add_button_clicked() {
    QString ext = ui.files_line_edit->text();

    if (ext.at(0) != ".") {
        ext.prepend(".");
    }

    if (!itemExists(ui.files_list_widget, ext)) {
        ui.files_list_widget->addItem(ext);
    }

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

bool Etemenanki::add_regex_row(QString pattern, QString string_pos, QString id_pos, int row) {
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
            ui.regex_table_widget->setItem(ui.regex_table_widget->rowCount() - 1, 0, new QTableWidgetItem(pattern));
            ui.regex_table_widget->setItem(ui.regex_table_widget->rowCount() - 1, 1, new QTableWidgetItem(string_pos));
            ui.regex_table_widget->setItem(ui.regex_table_widget->rowCount() - 1, 2, new QTableWidgetItem(id_pos));
            return true;
        }
    }
    else {
        ui.regex_table_widget->setItem(row, 0, new QTableWidgetItem(pattern));
        ui.regex_table_widget->setItem(row, 1, new QTableWidgetItem(string_pos));
        ui.regex_table_widget->setItem(row, 2, new QTableWidgetItem(id_pos));
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

    if (add_regex_row(regex, string_pos, id_pos, i)) {
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
    ui.regex_line_edit->setText(ui.regex_table_widget->item(i, 0)->text());
    ui.position_string_line_edit->setText(ui.regex_table_widget->item(i, 1)->text());
    ui.position_id_line_edit->setText(ui.regex_table_widget->item(i, 2)->text());
    ui.regex_update_button->setEnabled(true);
    ui.regex_remove_button->setEnabled(true);
}

void Etemenanki::on_begin_button_clicked() {
    if (xstrProcessor->isRunning()) {
        continueProcessing = false;

        ui.begin_button->setEnabled(false);
        ui.begin_button->setText("Waiting...");

        XSTR_thread->wait();
        delete XSTR_thread;

        ui.begin_button->setText("Run");
        ui.begin_button->setEnabled(true);
        toggleControls(true);
        return;
    }

    xstrProcessor->clearVectors();
    
    // Get values from UI
    QString directory = ui.directory_line_edit->text();
    QString output = ui.output_line_edit->text();
    QString offset = ui.offset_line_edit->text();
    bool replace = ui.replace_radio_button->isChecked();

    // Validate values
    if (directory.isEmpty()) {
        ui.terminal_output->setText("No directory provided!");
        return;
    }

    if (output.isEmpty()) {
        ui.terminal_output->setText("Output not provided. Using default!");
        output = "tstrings.tbl";
        return;
    }

    if (offset.isEmpty()) {
        ui.terminal_output->setText("Offset not provided. Setting to 0!");
        offset = "1";
        return;
    }
    
    // Set internal values
    xstrProcessor->setInputPath(directory.toStdString());
    xstrProcessor->setOutputFilename(output.toStdString());
    xstrProcessor->setOffset(offset.toInt());

    for (int i = 0; i < ui.files_list_widget->count(); ++i) {
        std::string ext = ui.files_list_widget->item(i)->text().toStdString();
        xstrProcessor->addFileExtension(ext);
    }

    for (int i = 0; i < ui.regex_table_widget->rowCount(); ++i) {
        std::string pattern = ui.regex_table_widget->item(i, 0)->text().toStdString();
        std::string string_pos = ui.regex_table_widget->item(i, 1)->text().toStdString();
        std::string id_pos = ui.regex_table_widget->item(i, 2)->text().toStdString();

        int pos = std::stoi(string_pos);
        int id = std::stoi(id_pos);

        xstrProcessor->addRegexPattern(pattern, pos, id, i);
    }
    qDebug() << "Processing in thread:" << QThread::currentThreadId();

    auto function = std::bind(&Etemenanki::runXSTR, this);
    XSTR_thread = QThread::create(function);

    connect(XSTR_thread, &QThread::finished, this, [this]() {
        ui.actionExit->setEnabled(true);
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
    ui.output_line_edit->setEnabled(val);
    ui.actionExit->setEnabled(val);
}

void Etemenanki::updateTerminalOutput(const QString& text) {
    ui.terminal_output->setText(text);
}