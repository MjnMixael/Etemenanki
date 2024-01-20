#include "Settings.h"

SettingsDialog::SettingsDialog(Etemenanki* etemenanki, QWidget* parent) : QDialog(parent), etemenanki(etemenanki) {
    ui.setupUi(this);

    if (etemenanki) {
        ui.comprehensive_button->setChecked(etemenanki->get_comprehensive());
        ui.fill_in_ids_button->setEnabled(etemenanki->get_comprehensive());
        ui.fill_in_ids_button->setChecked(etemenanki->get_fill_in_ids());
    }
}

void SettingsDialog::accept() {
    if (etemenanki) {
        etemenanki->set_comprehensive(ui.comprehensive_button->isChecked());

        if (ui.comprehensive_button->isChecked()) {
            etemenanki->set_fill_in_ids(ui.fill_in_ids_button->isChecked());
            etemenanki->toggle_offset_control(!ui.fill_in_ids_button->isChecked());
        } else {
            etemenanki->set_fill_in_ids(false);
            etemenanki->toggle_offset_control(true);
        }
    }

    QDialog::accept();
}

void SettingsDialog::on_comprehensive_button_clicked() {
    ui.fill_in_ids_button->setEnabled(ui.comprehensive_button->isChecked());
}