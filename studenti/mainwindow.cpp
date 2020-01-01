#include "mainwindow.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTabWidget>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QByteArray>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSqlQueryModel>
#include <QTableView>
#include <QDebug>

#include "predmet.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto time_point = std::chrono::system_clock::now();
    std::time_t now = std::chrono::system_clock::to_time_t(time_point);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now),"%Y-%m-%d");
    datum = ss.str().c_str();
    // qDebug() << "datum   =" << datum;

    connectDB();    
    createGUI();

    dialog = new PredmetDialog(this);
    dialog->show();

    connect(dialog, &PredmetDialog::predmetAccepted, [this](QString s){
        predmet = s;
        show();
        dialog->close();
        prednaskaInitComboBox();

        QString sql { R"sql(
        select student, prijmeni, jmeno, soucet,
               case when soucet > 0 then soucet
                    else null
               end as body
        from (
           select *
           from studenti join zapisy using (student)
           where predmet = '%1'
        ) as zapsani_studenti
        left join
        (
          select student, predmet, sum(body) as soucet
          from hodnoceni
          where predmet = '%1'
          group by student, predmet
        ) as soucty_bodu_z_hodnoceni
        using (student)
        order by prijmeni asc, jmeno asc )sql" };

        prehledQuery = sql.arg(predmet).simplified();
        });
}

MainWindow::~MainWindow()
{
}

void MainWindow::connectDB()
{
    QSqlDatabase db {};
    // podmineny preklad pro databaze postgresql nebo sqlite3
#if 1
    { // ssh tunel: (auto)ssh -L 65432:localhost:5432 geo102.fsv.cvut.cz
      //                      -o ServerAliveInterval=60
      // pripojeni: psql -h localhost -p 65432 -U cepek cepek_studenti
      // heslo    : ALTER ROLE cepek with encrypted password '...';
      // pripojeni k databazi postgresql
        db = QSqlDatabase::addDatabase("QPSQL");
        db.setDatabaseName("cepek_studenti");
        db.setHostName("localhost");
        db.setPassword("qwerty");
        db.setPort(65432);
        db.open();
        if (!db.isOpen()) {
            QSqlError error = db.lastError();
            qDebug() << error.text();
            QMessageBox messageBox;
            messageBox.critical(0,tr("Chyba při otevírání databáze"),error.text());
            throw int(1);
        }
    }
#else
    // pripojeni k databazi sqlite3
    // hledam soubor studenti.db v beznem adresari, nadrazenem a "vedlejsim"
    QString sqliteDbFileName {};
    for (auto path : QStringList({"", "../", "../studenti/"}))
    {
        QString file = path + "studenti.db";
        QFileInfo fileInfo {file};
        if (fileInfo.exists() && fileInfo.isFile()) {
            sqliteDbFileName = file;
            break;
        }
    }

    if (sqliteDbFileName.isEmpty()) {
        QMessageBox messageBox;
        messageBox.critical(0,tr("Chyba při otevírání databáze"),
                            tr("Soubor \"studenti.db\" nenalezen"));
        throw int(2);
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(sqliteDbFileName);
    db.open();
    // qDebug() << db.isOpen() << db.lastError();
#endif

    QString missing {};
    QStringList dbTables = db.tables();
    for (auto t : QStringList({"studenti", "predmety", "zapisy", "hodnoceni"})) {
        if (!dbTables.contains(t,Qt::CaseInsensitive)) {
            missing += (missing.isEmpty()?"    ":"\n    ") +t;
        }
    }
    if (!missing.isEmpty()) {
        missing = tr("Databáze \"%1\" neobsahuje tabulky\n").arg(db.databaseName()) + missing;
        QMessageBox messageBox;
        messageBox.critical(0,tr("Chyba při otevírání databáze"),missing);
        throw int(3);
    }

    statusBar()->showMessage(tr("Databáze připojena"),10000);
}

void MainWindow::createGUI()
{
    setContentsMargins(5,5,5,5);

    QAction* action {nullptr};
    QMenu* menuFile = new QMenu(tr("&Soubor"), this);

    action = menuFile->addAction(tr("&Konec"));
    connect(action, &QAction::triggered, [this](){close();});

    menuBar()->addMenu(menuFile);

    QTabWidget* tabWidget = new QTabWidget(this);
    tabWidget->addTab(prezenceTab(), tr("Prezence"));
    tabWidget->addTab(prednaskaTab(), tr("Přednáška"));
    tabWidget->addTab(prehledTab(), tr("Přehled"));

    connect(tabWidget, &QTabWidget::currentChanged,
            [this](int i){ if (i == 2) prehledModel->setQuery(prehledQuery); });

    setCentralWidget(tabWidget);
}

// --------------------------------------------------------

QWidget* MainWindow::prezenceTab()
{
    QWidget* tab = new QDialog;
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addItem(prezenceButtons());
    hbox->addItem(prezencetInfo());
    tab->setLayout(hbox);
    return tab;
}

QVBoxLayout* MainWindow::prezencetInfo()
{
    QVBoxLayout* prez = new QVBoxLayout;
    prezenceLabel = new QLabel(tr("jméno"));
    prezenceImage = new QLabel(tr("obrázek"));
    prezenceClearInfo();
    prez->addWidget(prezenceLabel);
    prez->addWidget(prezenceImage);
    prez->addStretch();
    return prez;
}

QVBoxLayout* MainWindow::prezenceButtons()
{
    QVBoxLayout* buttons = new QVBoxLayout;
    QGridLayout* grid = new QGridLayout;
    prezenceLosovani = new QPushButton(tr("losování"));

    prezencePritomen = new  QPushButton(tr("+"));
    QPalette paletteP = prezencePritomen->palette();
    paletteP.setColor(QPalette::Button, QColor(Qt::green));
    prezencePritomen->setPalette(paletteP);

    prezenceNepritomen = new QPushButton(tr("-"));
    QPalette paletteN = prezenceNepritomen->palette();
    paletteN.setColor(QPalette::Button, QColor(Qt::red));
    prezenceNepritomen->setPalette(paletteN);

    grid->addWidget(prezenceLosovani,   0, 0);
    grid->addWidget(prezencePritomen,   0, 1);
    grid->addWidget(prezenceNepritomen, 1, 1);
    buttons->addLayout(grid);
    buttons->addStretch();

    connect(prezenceLosovani,  &QPushButton::clicked, [this](){prezenceDisplay();});
    connect(prezencePritomen,  &QPushButton::clicked, [this](){prezenceBody(+1);});
    connect(prezenceNepritomen,&QPushButton::clicked, [this](){prezenceBody(-1);});

    return buttons;
}

void MainWindow::prezenceDisplay()
{
    QSqlQuery query {"select s.student, jmeno, prijmeni, img_base64 "
                     "from   studenti as s "
                     "join   zapisy as z "
                     "       on s.student = z.student "
                     "       and predmet = '" + predmet + "'"
                     "where s.studuje = 1 and s.student not in ("
                     "   select student "
                     "   from hodnoceni "
                     "   where datum='" + datum + "' and predmet = '" + predmet + "'"
                     ") "
                     "order by random();"};
    // qDebug() << query.lastError();
    if (query.next()) {
        prezenceID = query.value(0).toInt();
        QString jmeno    {query.value(1).toString()};
        QString prijmeni {query.value(2).toString()};
        prezenceLabel->setText(jmeno + " " + prijmeni);

        QByteArray img = query.value(3).toByteArray();
        QPixmap pixmap;
        pixmap.loadFromData(QByteArray::fromBase64(img));
        prezenceImage->setPixmap(pixmap.scaled(imgSize, Qt::KeepAspectRatio));
    }
}

void MainWindow::prezenceClearInfo()
{
    prezenceLabel->setText("");
    QPixmap pixmap(imgSize);
    pixmap.fill(imgFill);
    prezenceImage->setPixmap(pixmap);
    prezenceID = 0;
}

void MainWindow::prezenceBody(int body)
{
    if (prezenceID == 0) prezenceClearInfo();

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QString s {"insert into hodnoceni (predmet,student,datum,body) "
               "values ('%1','%2','%3','%4');"};
    QString t {s.arg(predmet).arg(prezenceID).arg(datum).arg(body)};
    QSqlQuery query;
    query.exec(t);
    db.commit();
    prezenceClearInfo();
    if (body < 0) prednaskaInitComboBox();
}

// --------------------------------------------------------

QWidget* MainWindow::prednaskaTab()
{
    QWidget* tab = new QDialog;
    new QGridLayout;
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addItem(prednaskaButtons());
    hbox->addItem(prednaskaInfo());
    tab->setLayout(hbox);  
    return tab;
}

QVBoxLayout* MainWindow::prednaskaButtons()
{
    QVBoxLayout* buttons = new QVBoxLayout;
    QGridLayout* grid = new QGridLayout;
    prednaskaLosovani = new QPushButton(tr("losování"));

    prednaskaPlus = new QPushButton(tr("+"));
    QPalette paletteP = prednaskaPlus->palette();
    paletteP.setColor(QPalette::Button, QColor(Qt::green));
    prednaskaPlus->setPalette(paletteP);

    prednaskaNula = new QPushButton(tr("0"));

    prednaskaNepritomen = new QPushButton(tr("-"));
    QPalette paletteN = prednaskaNepritomen->palette();
    paletteN.setColor(QPalette::Button, QColor(Qt::red));
    prednaskaNepritomen->setPalette(paletteN);

    prednaskaComboBox = new QComboBox;

    //prednaskaFilter = new QLineEdit;

    grid->addWidget(prednaskaLosovani,   0, 0);
    grid->addWidget(prednaskaPlus,       0, 1);
    grid->addWidget(prednaskaNula,       1, 1);
    grid->addWidget(prednaskaNepritomen, 2, 1);

    QPixmap pixmap(50,10);
    pixmap.fill(Qt::transparent); // Qt::lightGray);
    QLabel* sep = new QLabel;
    sep->setPixmap(pixmap);
    grid->addWidget(sep, 3,1);

    grid->addWidget(prednaskaComboBox,   4, 0, 1, 2);
    //grid->addWidget(prednaskaFilter,     5, 0, 1, 2);

    buttons->addLayout(grid);
    buttons->addStretch();

    connect(prednaskaLosovani,  &QPushButton::clicked, [this](){prednaskaDisplay();});
    connect(prednaskaPlus,      &QPushButton::clicked, [this](){prednaskaBody(+1);});
    connect(prednaskaNula,      &QPushButton::clicked, [this](){prednaskaBody( 0);});
    connect(prednaskaNepritomen,&QPushButton::clicked, [this](){prednaskaBody(-1);});
    connect(prednaskaComboBox,  static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
       [this](int k){prednaskaComboBoxActivated(prednaskaComboBox->itemData(k).toInt());});

    return buttons;
}

void MainWindow::prednaskaDisplay()
{
    QSqlQuery query {"select s.student, jmeno, prijmeni, img_base64 "
                     "from   studenti as s "
                     "join   zapisy as z "
                     "       on s.student = z.student "
                     "       and predmet = '" + predmet + "'"
                     "where s.studuje = 1 and s.student not in ("
                     "   select student "
                     "   from hodnoceni "
                     "   where datum='" + datum + "' and predmet = '" + predmet + "'"
                     "         and body = -1"
                     ") "
                     "order by random();"};
    if (query.next()) {
        prednaskaID = query.value(0).toInt();
        QString jmeno    {query.value(1).toString()};
        QString prijmeni {query.value(2).toString()};
        prednaskaLabel->setText(jmeno + " " + prijmeni);

        QByteArray img = query.value(3).toByteArray();
        QPixmap pixmap;
        pixmap.loadFromData(QByteArray::fromBase64(img));
        prednaskaImage->setPixmap(pixmap.scaled(imgSize, Qt::KeepAspectRatio));
    }
}

QVBoxLayout* MainWindow::prednaskaInfo()
{
    QVBoxLayout* pred = new QVBoxLayout;
    prednaskaLabel = new QLabel(tr("jméno"));
    prednaskaImage = new QLabel(tr("obrázek"));
    prednaskaClearInfo();
    pred->addWidget(prednaskaLabel);
    pred->addWidget(prednaskaImage);
    pred->addStretch();
    return pred;
}

void MainWindow::prednaskaClearInfo()
{
    prednaskaLabel->setText("");
    QPixmap pixmap(imgSize);
    pixmap.fill(imgFill);
    prednaskaImage->setPixmap(pixmap);
    prednaskaID = 0;
}

void MainWindow::prednaskaBody(int body)
{
    if (prednaskaID == 0) prednaskaClearInfo();

    QSqlDatabase db = QSqlDatabase::database();
    QString t { " from hodnoceni where predmet='%1' and student='%2' and datum='%3'" };
    QString s { t.arg(predmet).arg(prednaskaID).arg(datum) };

    QSqlQuery q("select body" + s);
    if (q.next()) {
         if (body < 0) return prednaskaClearInfo(); // nemohu snizit pridelene body

         body += q.value(0).toInt();

         QSqlQuery r;
         r.exec("delete" + s);
    }
    if (body == 0) body++;

    QString u {"insert into hodnoceni (predmet,student,datum,body) "
               "values ('%1','%2','%3','%4');"};
    QString v {u.arg(predmet).arg(prednaskaID).arg(datum).arg(body)};
    QSqlQuery query;
    db.transaction();
    query.exec(v);
    db.commit();

    prednaskaClearInfo();
    if (body < 0) prednaskaInitComboBox();
}

void MainWindow::prednaskaInitComboBox()
{
    prednaskaComboBox->clear();

    QSqlQuery query {"select s.student, jmeno, prijmeni, img_base64 "
                     "from   studenti as s "
                     "join   zapisy as z "
                     "       on s.student = z.student "
                     "       and predmet = '" + predmet + "' "
                     "where s.studuje = 1 and s.student not in ("
                     "   select student "
                     "   from hodnoceni "
                     "   where datum='" + datum + "' and predmet = '" + predmet + "'"
                     "         and body = -1"
                     ") "
                     "order by prijmeni asc, jmeno asc;"};

   while (query.next()) {
       QString student = query.value(2).toString() + ", " +
                         query.value(1).toString();
       prednaskaComboBox->addItem(student, query.value(0));
   }
}

void MainWindow::prednaskaComboBoxActivated(int id)
{
    prednaskaID = id;
    QString s {"select jmeno, prijmeni, img_base64 from   studenti where student='%1'"};
    QSqlQuery query {s.arg(id)};
    if (query.next()) {
        QString jmeno    {query.value(0).toString()};
        QString prijmeni {query.value(1).toString()};
        prednaskaLabel->setText(jmeno + " " + prijmeni);

        QByteArray img = query.value(2).toByteArray();
        QPixmap pixmap;
        pixmap.loadFromData(QByteArray::fromBase64(img));
        prednaskaImage->setPixmap(pixmap.scaled(imgSize, Qt::KeepAspectRatio));
    }
}

// --------------------------------------------------------

QWidget* MainWindow::prehledTab()
{
    QWidget* tab = new QDialog;
    QVBoxLayout* hbox = new QVBoxLayout;
    prehledTableView = new QTableView;
    prehledModel = new QSqlQueryModel;
    // prehledModel->setQuery(prehledQuery); ... prenastaveno vzdy pri vyberu zalozky
    prehledTableView->setModel(prehledModel);
    hbox->addWidget(prehledTableView);
    tab->setLayout(hbox);

    return tab;
}
