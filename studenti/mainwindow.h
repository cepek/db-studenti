#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "predmet.h"

class QLabel;
class QPushButton;
class QVBoxLayout;
class QComboBox;
class QSqlQueryModel;
class QTableView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    const QString programVersion {"0.90"};

private:
    void connectDB();
    void createGUI();

    QString predmet {};
    QString datum {};

    PredmetDialog* dialog {};


    // ----------------------------------------------------
    QWidget* prezenceTab();

    void prezenceDisplay();
    void prezenceBody(int body);
    void prezenceClearInfo();
    QVBoxLayout* prezencetInfo();
    QVBoxLayout* prezenceButtons();

    const QSize  imgSize {QSize(300,300)};
    const QColor imgFill {QColor(220,220,220,100)};

    int prezenceID {0};
    QLabel*  prezenceLabel {nullptr};
    QLabel*  prezenceImage {nullptr};
    QPushButton* prezenceLosovani {nullptr};
    QPushButton* prezencePritomen {nullptr};
    QPushButton* prezenceNepritomen {nullptr};

    // ----------------------------------------------------
    QWidget* prednaskaTab();

    void prednaskaDisplay();
    void prednaskaBody(int body);
    void prednaskaClearInfo();
    QVBoxLayout* prednaskaInfo();
    QVBoxLayout* prednaskaButtons();

    int prednaskaID {0};
    QLabel*  prednaskaLabel {nullptr};
    QLabel*  prednaskaImage {nullptr};
    QPushButton* prednaskaLosovani {nullptr};
    QPushButton* prednaskaPlus {nullptr};
    QPushButton* prednaskaNula {nullptr};
    QPushButton* prednaskaNepritomen {nullptr};
    QComboBox*   prednaskaComboBox {nullptr};
    //QLineEdit*   prednaskaFilter {nullptr};
    void prednaskaInitComboBox();
    void prednaskaComboBoxActivated(int id);

    // ----------------------------------------------------
    QWidget* prehledTab();

    QSqlQueryModel* prehledModel {nullptr};
    QTableView* prehledTableView {nullptr};
    QString prehledQuery {};
};

#endif // MAINWINDOW_H
