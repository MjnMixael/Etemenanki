#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Etemenanki.h"
#include "xstr.h"

class Etemenanki : public QMainWindow
{
    Q_OBJECT

public:
    Etemenanki(QWidget *parent = nullptr);
    QThread* XSTR_thread;

public slots:
    void on_files_add_button_clicked();
    void on_files_update_button_clicked();
    void on_files_remove_button_clicked();
    void on_files_list_widget_clicked();

    void on_regex_add_button_clicked();
    void on_regex_update_button_clicked();
    void on_regex_remove_button_clicked();
    void on_regex_table_widget_clicked();

    void on_begin_button_clicked();

    void updateTerminalOutput(const QString& text);

    void runXSTR();

private:
    Ui::EtemenankiClass ui;
    XstrProcessor* xstrProcessor;
    QThread* processor;

    void toggleControls(bool val);
    bool add_regex_row(QString pattern, QString string_pos, QString id_pos, int row = -1);
};