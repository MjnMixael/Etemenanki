#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Etemenanki.h"
#include "xstr.h"

class Etemenanki : public QMainWindow
{
    Q_OBJECT

public:
    Etemenanki(QWidget *parent = nullptr);

public slots:
    void on_begin_button_clicked();
    void set_terminal_text(std::string input);

private:
    Ui::EtemenankiClass ui;
};