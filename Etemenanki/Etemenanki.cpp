#include "Etemenanki.h"
#include "xstr.h"

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
    ui.directory_line_edit->setText("C:\\Games\\FreespaceOpen\\FS2\\BtA-2.0.0");
    ui.offset_line_edit->setText("0");
    add_regex_row("XSTR\\s*\\(\\s*\"([^\"] + )\",\\s*(-?\\d+)\\s*\\)", "1", "2");
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

    auto function = std::bind(&Etemenanki::runXSTR, this);
    XSTR_thread = QThread::create(function);
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
    }
}

void Etemenanki::on_files_remove_button_clicked() {
    ui.files_list_widget->takeItem(ui.files_list_widget->currentRow());
    ui.files_line_edit->clear();
    ui.files_update_button->setEnabled(false);
    ui.files_remove_button->setEnabled(false);
    ui.files_list_widget->clearSelection();
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

bool Etemenanki::add_regex_row(QString regex, QString string_pos, QString id_pos, int row) {
    if (regex.isEmpty()) {
        return false;
    }

    if (string_pos.isEmpty()) {
        string_pos = "1";
    }

    if (id_pos.isEmpty()) {
        id_pos = "2";
    }

    if (row < 0) {
        if (!itemExists(ui.regex_table_widget, regex)) {
            ui.regex_table_widget->insertRow(ui.regex_table_widget->rowCount());
            ui.regex_table_widget->setItem(ui.regex_table_widget->rowCount() - 1, 0, new QTableWidgetItem(regex));
            ui.regex_table_widget->setItem(ui.regex_table_widget->rowCount() - 1, 1, new QTableWidgetItem(string_pos));
            ui.regex_table_widget->setItem(ui.regex_table_widget->rowCount() - 1, 2, new QTableWidgetItem(id_pos));
            return true;
        }
    }
    else {
        ui.regex_table_widget->setItem(row, 0, new QTableWidgetItem(regex));
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
    /*if (XSTR_thread->isRunning()) {
        XSTR_thread->terminate(); //Causes an crash -- FIXMEEE
        ui.begin_button->setText("Run");
    }*/

    Valid_extensions.clear();
    Valid_patterns.clear();
    
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
    Input_path = directory.toStdString();
    Offset = offset.toInt();
    Output_filename = output.toStdString();

    for (int i = 0; i < ui.files_list_widget->count(); ++i) {
        std::string ext = ui.files_list_widget->item(i)->text().toStdString();
        Valid_extensions.push_back(ext);
    }

    for (int i = 0; i < ui.regex_table_widget->rowCount(); ++i) {
        std::string pattern = ui.regex_table_widget->item(i, 0)->text().toStdString();
        std::string string_pos = ui.regex_table_widget->item(i, 1)->text().toStdString();
        std::string id_pos = ui.regex_table_widget->item(i, 2)->text().toStdString();

        std::regex reg;
        int pos = std::stoi(string_pos);
        int id = std::stoi(id_pos);
        try {
            reg.assign(pattern);
        }
        catch (const std::regex_error& e) {
            std::cerr << "Regex error: " << e.what() << std::endl;
        }

        regexPattern thisPattern = { reg, pos, id, i, pattern };
        Valid_patterns.push_back(thisPattern);
    }

    XSTR_thread->start();

    //ui.begin_button->setText("Terminate");
    ui.begin_button->setEnabled(false);
    ui.directory_line_edit->setEnabled(false);
    ui.files_line_edit->setEnabled(false);
    ui.files_list_widget->setEnabled(false);
    ui.files_add_button->setEnabled(false);
    ui.files_update_button->setEnabled(false);
    ui.files_remove_button->setEnabled(false);
    ui.regex_line_edit->setEnabled(false);
    ui.position_string_line_edit->setEnabled(false);
    ui.position_id_line_edit->setEnabled(false);
    ui.regex_table_widget->setEnabled(false);
    ui.regex_add_button->setEnabled(false);
    ui.regex_update_button->setEnabled(false);
    ui.regex_remove_button->setEnabled(false);
    ui.offset_line_edit->setEnabled(false);
    ui.replace_radio_button->setEnabled(false);
    ui.output_line_edit->setEnabled(false);
    ui.actionExit->setEnabled(false);
}

void Etemenanki::runXSTR() {
    run(ui.terminal_output);
}

void Etemenanki::set_terminal_text(std::string input) {
    QString text;
    text.fromStdString(input);
    ui.terminal_output->setText(text);
}