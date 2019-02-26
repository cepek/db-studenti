#include "predmet.h"
#include <QVBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>

PredmetDialog::PredmetDialog(QWidget *parent) : QDialog(parent)
{
    comboBox = new QComboBox(this);

    QSqlQuery query {"select predmet from predmety where aktivni=1 order by predmet desc;"};

    int minLenght {1};
    while (query.next()) {
        QString val {query.value(0).toString().simplified()};
        comboBox->addItem(val);
        if (val.length() > minLenght) minLenght = val.length();
    }
    if (comboBox->count() == 0){
       QMessageBox messageBox;
        messageBox.critical(0,tr("Chyba při otevírání databáze"),
                              tr("V databázi nejsou žádné předměty"));
        throw int(4);
    }
    comboBox->setMinimumContentsLength(minLenght);

    setWindowTitle(tr("Výběr předmětu"));
    setModal(true);

    QVBoxLayout* vbox = new QVBoxLayout(this); // this nastavuje novy layout pro Dialog

    QHBoxLayout* comboLayout = new QHBoxLayout;
    comboLayout->addStretch();
    comboLayout->addWidget(comboBox);
    comboLayout->addStretch();
    vbox->addLayout(comboLayout);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, [this](){
        emit predmetAccepted(comboBox->currentText());
        });
    connect(buttonBox, &QDialogButtonBox::rejected, [this](){close();});

    vbox->addWidget(buttonBox);
}
