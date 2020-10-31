#include "ConnectPlayUI.h"
#include <QtWidgets/QAction>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#include <QRegexp>

ConnectPlayUI::ConnectPlayUI() {
    ui.setupUi(this);
    setWindowFlags(windowFlags() & (~(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint)));

    connect(ui.btnCancel, &QPushButton::clicked, [&](int) {
        close();
    });

    ui.leIP->setText("127.0.0.1");
    ui.lePort->setText("23333");

    connect(ui.btnOK, &QPushButton::clicked, [&](int) {
        auto addr = ui.leIP->text();
        auto port = ui.lePort->text().toInt();

        QRegExp reg("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
        if (ui.leIP->isEnabled() && !reg.exactMatch(addr)) {
            QMessageBox::warning(this, u8"提示", u8"ip地址格式有误");
            return;
        }

        if (port >= 65536) {
            QMessageBox::warning(this, u8"提示", u8"端口号范围[0, 65535]");
            return;
        }

        if (onConfirmListener) {
            ui.btnOK->setDisabled(true);
            ui.btnCancel->setDisabled(true);
            if (onConfirmListener(addr, port))
                close();
        }
    });
}

void ConnectPlayUI::showAndSetup(int mode) {
    setModal(true);
    show();
    ui.btnCancel->setDisabled(false);
    ui.btnOK->setDisabled(false);
    if (mode == 0) {
        ui.leIP->setDisabled(false);
        ui.lePort->setDisabled(false);
    }
    else {
        ui.leIP->clear();
        ui.leIP->setDisabled(true);
        ui.lePort->setDisabled(false);
    }
}