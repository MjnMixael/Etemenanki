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
#include <QFileDialog>

#include "ui_Etemenanki.h"
#include "Settings.h"
#include "IgnoreFiles.h"
#include "IgnoreIds.h"
#include "PreloadPairs.h"
#include "xstr.h"

class Etemenanki : public QMainWindow
{
    Q_OBJECT

public:
    Etemenanki(QWidget *parent = nullptr);

    // Pass controls from Settings.cpp
    void setComprehensive(bool val);
    bool getComprehensive();
    void setFillInIds(bool val);
    bool getFillInIds();
    void setSortingType(int val);
    int getSortingType();
    void setHeaderAnnotations(bool val);
    bool getHeaderAnnotations();
    void setVerboseAnnotations(bool val);
    bool getVerboseAnnotations();
    void setCaseInsensitive(bool val);
    bool getCaseInsensitive();
    void toggleOffsetControl(bool val);

    // Check if an item exists in a widget
    static bool itemExists(QListWidget* listWidget, const QString& textToCheck);
    static bool itemExists(QTableWidget* tableWidget, const QString& textToCheck);

    // Public vectors
    QVector<QString> m_ignoredFilesList;
    QVector<QString> m_ignoredIdsList;
    QVector<XstrPair> m_preloadedPairs;

    //Public variables
    QString m_appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Etemenanki/";
    QString m_preloadFilename = "xstr_pairs.json";

public slots:
    // Browse buttons
    void on_directory_browse_button_clicked();
    void on_output_directory_browse_button_clicked();
    
    // File extensions widget
    void on_files_add_button_clicked();
    void on_files_update_button_clicked();
    void on_files_remove_button_clicked();
    void on_files_clear_button_clicked();
    void on_files_list_widget_clicked();

    // Regex table widget
    void on_regex_add_button_clicked();
    void on_regex_update_button_clicked();
    void on_regex_remove_button_clicked();
    void on_regex_clear_button_clicked();
    void on_regex_table_widget_clicked();

    // Read only widget
    void on_read_only_checkbox_clicked();

    // Begin button
    void on_begin_button_clicked();

    // Terminal convenience
    void update_terminal_output(const QString& text);

    // Calls the xstr processing thread
    void run_xstr();

    // Menu actions
    void ui_save_settings();
    void ui_open_documentation();
    void ui_open_preferences();
    void ui_open_ignore_files();
    void ui_open_ignore_ids();
    void ui_open_preload_pairs();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::EtemenankiClass ui;

    // Generic UI and application settings
    QString m_title = "Etemenanki";
    QString m_version = "v.1.4.0";
    QString m_description = "A translation tool for FreespaceOpen!";
    QString m_settingsFileName = "settings.json";
    QString m_settingsFilePath;
    QString m_logFileName = "Etemenanki.log";
    QString m_logFilePath;
    QUrl m_githubUrl = "https://github.com/MjnMixael/Etemenanki";
    int m_regexCheckWidth = 23;
    int m_regexPositionWidth = 40;

    // Thread pointers
    XstrProcessor* m_xstrProcessor;
    QThread* m_xstrThread;

    // UI control methods
    void toggleControls(bool val);
    bool addRegexRow(QString pattern, QString string_pos, QString id_pos, bool checked = true, int row = -1);
    bool isRowChecked(int row);
    void addFileExtension(QString ext);
    void loadSettings();
    void saveSettings();
    void resetInterface();

    // Data control methods
    void loadPreloadedPairs();

    // Default user values
    QString m_defaultOutputFile = "tstrings.tbl";
    QString m_defaultOutputDirectory = "";
    QString m_defaultOffset = "0";
    bool m_defaultReplacement = false;
    bool m_comprehensiveScan = false;
    bool m_fillInIds = false;
    int m_sortingType = PARSING_ORDER;
    bool m_headerAnnotations = true;
    bool m_verboseAnnotations = false;
    bool m_readOnly = false;
    bool m_caseInsensitive = false;
    QStringList m_defaultExtensions = { ".tbl", ".tbm", ".fs2", ".fc2" };
    QJsonObject m_defaultRegex() {
        QJsonObject obj;
        obj["regex_string"] = "XSTR\\(\"([^\"]+)\",\\s*(-?\\d+)\\)";
        obj["string_position"] = 1;
        obj["id_position"] = 2;
        return obj;
    }
};