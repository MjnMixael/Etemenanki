/*
 * Copyright (C) Mike Nelson
 *
 * All source code herein is subject to the GPL license included.
 *
*/

#pragma once

#include <QtWidgets/QMainWindow>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDesktopServices>

#include "ui_Etemenanki.h"
#include "Settings.h"
#include "xstr.h"

class Etemenanki : public QMainWindow
{
    Q_OBJECT

public:
    Etemenanki(QWidget *parent = nullptr);
    QThread* XSTR_thread;

    void set_comprehensive(bool val);
    bool get_comprehensive();

    void set_fill_in_ids(bool val);
    bool get_fill_in_ids();

    void toggle_offset_control(bool val);

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

    // Menu actions
    void uiSaveSettings();
    void uiOpenDocumentation();
    void uiOpenPreferences();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QString Title = "Etemenanki";
    QString Version = "v.1.2.0";
    QString Description = "A translation tool for FreespaceOpen!";
    Ui::EtemenankiClass ui;
    XstrProcessor* xstrProcessor;
    QThread* processor;
    QString AppDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Etemenanki/";
    QString SettingsFileName;
    QString LogFileName;
    QUrl Github = "https://github.com/MjnMixael/Etemenanki";

    int regex_check_w = 23;
    int regex_position_w = 40;

    void toggleControls(bool val);
    bool add_regex_row(QString pattern, QString string_pos, QString id_pos, bool checked = true, int row = -1);
    bool isRowChecked(int row);
    void add_file_extension(QString ext);
    void loadSettings();
    void saveSettings();

    void resetInterface();

    // Default values
    QString defaultOutputFile = "tstrings.tbl";
    QString defaultOutputDirectory = "";
    QString defaultOffset = "0";
    bool defaultReplacement = false;
    bool comprehensiveScan = false;
    bool fillInIds = false;
    QStringList defaultExtensions = { ".tbl", ".tbm", ".fs2", ".fc2" };
    QJsonObject defaultRegex() {
        QJsonObject obj;
        obj["regex_string"] = "XSTR\\(\"([^\"]+)\",\\s*(-?\\d+)\\)";
        obj["string_position"] = 1;
        obj["id_position"] = 2;
        return obj;
    }
};