#include "Etemenanki.h"
#include "xstr.h"

Etemenanki::Etemenanki(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

void Etemenanki::on_begin_button_clicked() {
    run(ui.terminal_output);
}

void Etemenanki::set_terminal_text(std::string input) {
    QString text;
    text.fromStdString(input);
    ui.terminal_output->setText(text);
}