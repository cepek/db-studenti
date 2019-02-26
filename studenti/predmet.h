#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QDialogButtonBox;
class QComboBox;

class PredmetDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PredmetDialog(QWidget *parent = 0);


signals:
    void predmetAccepted(QString);
public slots:

private:
    QDialogButtonBox* buttonBox {};
    QComboBox* comboBox {};
};

#endif // DIALOG_H
