/*
    ConnectPlayUI.h
        ������սUI
            byʯ����
*/

#pragma once
#include "ui_ConnectPlayUI.h"
#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

class ConnectPlayUI : QDialog {
    Ui::ConnectPlayUI ui;
public:
    ConnectPlayUI();

    void showAndSetup(int mode);
    std::function<bool(const QString addr, int port)> onConfirmListener;
};

