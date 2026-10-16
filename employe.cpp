#include "employe.h"
#include "ui_employe.h"

#include <QApplication>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPrinter>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QTextDocument>
#include <QVBoxLayout>

namespace {
QString employeStyleSheet(bool dark)
{
    if (dark) {
        return R"(
            QWidget { background:#0b1220; color:#e8eef7; font-size:13px; }
            QFrame#headerFrame { background:#101a2d; border:1px solid #233453; border-radius:12px; }
            QLabel#logoLabel { color:#22d3ee; font-size:22px; font-weight:700; }
            QLabel#titleLabel { color:#f8fafc; font-size:24px; font-weight:700; }
            QLabel#subtitleLabel { color:#8fa0b6; }
            QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox { background:#111c31; color:#e8eef7; border:1px solid #2a3b5e; border-radius:8px; padding:8px 10px; }
            QPushButton { background:#111c31; color:#e8eef7; border:1px solid #2a3b5e; border-radius:8px; padding:8px 14px; }
            QPushButton:hover { border-color:#22d3ee; background:#16243c; }
            QPushButton#btnAjouter { background:#22d3ee; color:#06212a; border-color:#22d3ee; font-weight:700; }
            QPushButton#btnSupprimer { color:#ff7b8a; }
            QTableWidget { background:#0f192b; alternate-background-color:#111e33; border:1px solid #233453; border-radius:10px; gridline-color:#233453; selection-background-color:#183b52; selection-color:#ffffff; }
            QHeaderView::section { background:#101a2d; color:#9be9f5; border:none; border-bottom:1px solid #2a3b5e; padding:10px 6px; font-weight:700; }
            QStatusBar { color:#8fa0b6; }
        )";
    }
    return R"(
        QWidget { background:#f5f7fb; color:#1f2937; font-size:13px; }
        QFrame#headerFrame { background:#ffffff; border:1px solid #dce3ee; border-radius:12px; }
        QLabel#logoLabel { color:#1677ff; font-size:22px; font-weight:700; }
        QLabel#titleLabel { color:#111827; font-size:24px; font-weight:700; }
        QLabel#subtitleLabel { color:#64748b; }
        QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox { background:#ffffff; color:#1f2937; border:1px solid #cbd5e1; border-radius:8px; padding:8px 10px; }
        QPushButton { background:#ffffff; color:#1f2937; border:1px solid #cbd5e1; border-radius:8px; padding:8px 14px; }
        QPushButton:hover { border-color:#1677ff; background:#f1f7ff; }
        QPushButton#btnAjouter { background:#1677ff; color:#ffffff; border-color:#1677ff; font-weight:700; }
        QPushButton#btnSupprimer { color:#dc2626; }
        QTableWidget { background:#ffffff; alternate-background-color:#f8fafc; border:1px solid #dce3ee; border-radius:10px; gridline-color:#e5e7eb; selection-background-color:#dbeafe; selection-color:#111827; }
        QHeaderView::section { background:#f8fafc; color:#1d4ed8; border:none; border-bottom:1px solid #dce3ee; padding:10px 6px; font-weight:700; }
        QStatusBar { color:#64748b; }
    )";
}

QString esc(const QString &s)
{
    return s.toHtmlEscaped();
}
}

EmployePage::EmployePage(QWidget *parent)
    : QWidget(parent), ui(new Ui::EmployePage)
{
    ui->setupUi(this);
    setWindowTitle("Devena - Employés");
    resize(1400, 760);

    ui->tableEmployes->setColumnCount(10);
    ui->tableEmployes->setHorizontalHeaderLabels({
        "ID", "Nom", "Prénom", "Email", "Téléphone", "Date d'embauche",
        "Salaire", "Poste", "Statut", "Mot de passe"
    });
    ui->tableEmployes->horizontalHeader()->setStretchLastSection(true);
    ui->tableEmployes->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableEmployes->setAlternatingRowColors(true);
    ui->tableEmployes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableEmployes->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableEmployes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableEmployes->setSortingEnabled(true);

    ui->comboPoste->addItems({"Administrateur", "Moniteur"});
    ui->comboStatut->addItems({"Actif", "Inactif"});
    ui->dateEmbauche->setDate(QDate::currentDate());
    ui->salaire->setRange(0, 1000000);
    ui->salaire->setDecimals(2);
    ui->salaire->setSuffix(" DT");

    connect(ui->btnAjouter, &QPushButton::clicked, this, &EmployePage::ajouterEmploye);
    connect(ui->btnModifier, &QPushButton::clicked, this, &EmployePage::modifierEmploye);
    connect(ui->btnSupprimer, &QPushButton::clicked, this, &EmployePage::supprimerEmploye);
    connect(ui->btnExporterPDF, &QPushButton::clicked, this, &EmployePage::exporterPDF);
    connect(ui->btnTheme, &QPushButton::clicked, this, &EmployePage::changerTheme);
    connect(ui->btnEffacerRecherche, &QPushButton::clicked, this, &EmployePage::viderRecherche);
    connect(ui->txtRecherche, &QLineEdit::textChanged, this, &EmployePage::rechercherEmploye);
    connect(ui->tableEmployes, &QTableWidget::itemSelectionChanged, this, &EmployePage::chargerSelection);

    chargerExemples();
    appliquerStyle(m_sombre);
}

EmployePage::~EmployePage()
{
    delete ui;
}

void EmployePage::chargerExemples()
{
    ajouterLigne({"E001", "Trabelsi", "Rayen", "rayen@example.com", "+216 20 000 001", QDate(2026,1,15), 1800, "Administrateur", "demo123", "Actif"});
    ajouterLigne({"E002", "Ben Ali", "Sami", "sami@example.com", "+216 20 000 002", QDate(2026,2,10), 1400, "Moniteur", "demo123", "Actif"});
    ajouterLigne({"E003", "Mansour", "Lina", "lina@example.com", "+216 20 000 003", QDate(2026,3,5), 1350, "Moniteur", "demo123", "Inactif"});
}

void EmployePage::ajouterLigne(const EmployeData &e)
{
    const int row = ui->tableEmployes->rowCount();
    ui->tableEmployes->insertRow(row);
    const QStringList values = {
        e.id, e.nom, e.prenom, e.email, e.telephone,
        e.dateEmbauche.toString("dd/MM/yyyy"),
        QString::number(e.salaire, 'f', 2) + " DT",
        e.poste, e.statut, "••••••••"
    };
    for (int c = 0; c < values.size(); ++c) {
        auto *item = new QTableWidgetItem(values[c]);
        item->setData(Qt::UserRole, e.motDePasse);
        ui->tableEmployes->setItem(row, c, item);
    }
    ui->tableEmployes->setRowHeight(row, 42);
}

EmployeData EmployePage::lireLigne(int row) const
{
    EmployeData e;
    e.id = ui->tableEmployes->item(row,0)->text();
    e.nom = ui->tableEmployes->item(row,1)->text();
    e.prenom = ui->tableEmployes->item(row,2)->text();
    e.email = ui->tableEmployes->item(row,3)->text();
    e.telephone = ui->tableEmployes->item(row,4)->text();
    e.dateEmbauche = QDate::fromString(ui->tableEmployes->item(row,5)->text(), "dd/MM/yyyy");
    e.salaire = ui->tableEmployes->item(row,6)->text().remove(" DT").toDouble();
    e.poste = ui->tableEmployes->item(row,7)->text();
    e.statut = ui->tableEmployes->item(row,8)->text();
    e.motDePasse = ui->tableEmployes->item(row,9)->data(Qt::UserRole).toString();
    return e;
}

int EmployePage::ligneSelectionnee() const
{
    const auto ranges = ui->tableEmployes->selectedRanges();
    return ranges.isEmpty() ? -1 : ranges.first().topRow();
}


void EmployePage::chargerSelection()
{
    const int row = ligneSelectionnee();
    if (row < 0) return;
    const EmployeData e = lireLigne(row);
    ui->txtNom->setText(e.nom);
    ui->txtPrenom->setText(e.prenom);
    ui->txtEmail->setText(e.email);
    ui->txtTelephone->setText(e.telephone);
    ui->dateEmbauche->setDate(e.dateEmbauche.isValid() ? e.dateEmbauche : QDate::currentDate());
    ui->salaire->setValue(e.salaire);
    ui->comboPoste->setCurrentText(e.poste);
    ui->comboStatut->setCurrentText(e.statut);
    ui->txtMotDePasse->clear();
}

void EmployePage::ajouterEmploye()
{
    const QString nom = ui->txtNom->text().trimmed();
    const QString prenom = ui->txtPrenom->text().trimmed();
    const QString email = ui->txtEmail->text().trimmed();
    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires", "Nom, prénom et email sont obligatoires.");
        return;
    }

    EmployeData e;
    e.id = QString("E%1").arg(m_nextId++, 3, 10, QChar('0'));
    e.nom = nom;
    e.prenom = prenom;
    e.email = email;
    e.telephone = ui->txtTelephone->text().trimmed();
    e.dateEmbauche = ui->dateEmbauche->date();
    e.salaire = ui->salaire->value();
    e.poste = ui->comboPoste->currentText();
    e.motDePasse = ui->txtMotDePasse->text();
    e.statut = ui->comboStatut->currentText();
    ajouterLigne(e);
    ui->statusbar->showMessage("Employé ajouté : " + e.id, 3000);
}

void EmployePage::modifierEmploye()
{
    const int row = ligneSelectionnee();
    if (row < 0) {
        QMessageBox::information(this, "Modification", "Sélectionnez d'abord un employé.");
        return;
    }
    EmployeData e = lireLigne(row);
    e.nom = ui->txtNom->text().trimmed();
    e.prenom = ui->txtPrenom->text().trimmed();
    e.email = ui->txtEmail->text().trimmed();
    e.telephone = ui->txtTelephone->text().trimmed();
    e.dateEmbauche = ui->dateEmbauche->date();
    e.salaire = ui->salaire->value();
    e.poste = ui->comboPoste->currentText();
    if (!ui->txtMotDePasse->text().isEmpty()) e.motDePasse = ui->txtMotDePasse->text();
    e.statut = ui->comboStatut->currentText();

    ui->tableEmployes->removeRow(row);
    ui->tableEmployes->insertRow(row);
    const QStringList values = {e.id,e.nom,e.prenom,e.email,e.telephone,e.dateEmbauche.toString("dd/MM/yyyy"),QString::number(e.salaire,'f',2)+" DT",e.poste,e.statut,"••••••••"};
    for (int c=0;c<values.size();++c) {
        auto *item=new QTableWidgetItem(values[c]);
        item->setData(Qt::UserRole,e.motDePasse);
        ui->tableEmployes->setItem(row,c,item);
    }
    ui->tableEmployes->selectRow(row);
    ui->statusbar->showMessage("Employé modifié : " + e.id, 3000);
}

void EmployePage::supprimerEmploye()
{
    const int row = ligneSelectionnee();
    if (row < 0) {
        QMessageBox::information(this, "Suppression", "Sélectionnez d'abord un employé.");
        return;
    }
    const QString nom = ui->tableEmployes->item(row,2)->text() + " " + ui->tableEmployes->item(row,1)->text();
    if (QMessageBox::question(this, "Confirmer", "Supprimer " + nom + " ?") == QMessageBox::Yes) {
        ui->tableEmployes->removeRow(row);
        ui->statusbar->showMessage("Employé supprimé", 3000);
    }
}

void EmployePage::rechercherEmploye(const QString &texte)
{
    const QString q = texte.trimmed();
    for (int r=0; r<ui->tableEmployes->rowCount(); ++r) {
        bool visible = q.isEmpty();
        if (!visible) {
            for (int c=0; c<ui->tableEmployes->columnCount(); ++c) {
                if (ui->tableEmployes->item(r,c)->text().contains(q, Qt::CaseInsensitive)) {
                    visible = true;
                    break;
                }
            }
        }
        ui->tableEmployes->setRowHidden(r, !visible);
    }
}

void EmployePage::viderRecherche()
{
    ui->txtRecherche->clear();
}

void EmployePage::changerTheme()
{
    m_sombre = !m_sombre;
    appliquerStyle(m_sombre);
}

void EmployePage::appliquerStyle(bool sombre)
{
    qApp->setStyleSheet(employeStyleSheet(sombre));
    ui->btnTheme->setText(sombre ? "☀ Mode clair" : "☾ Mode sombre");
}

void EmployePage::exporterPDF()
{
    const QString file = QFileDialog::getSaveFileName(this, "Exporter les employés", "Employes.pdf", "PDF (*.pdf)");
    if (file.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(file);

    QString html = R"(<html><head><style>
        body{font-family:Arial;color:#172033;} h1{color:#1677ff;} table{border-collapse:collapse;width:100%;font-size:8pt;} th{background:#101a2d;color:white;padding:6px;} td{border:1px solid #d8dee9;padding:5px;} .logo{font-size:18pt;font-weight:bold;color:#1677ff;}
        </style></head><body>)";
    html += "<div class='logo'>◆ Devena</div><h1>Gestion des employés</h1>";
    html += "<p>Liste des employés — " + QDate::currentDate().toString("dd/MM/yyyy") + "</p>";
    html += "<table><tr>";
    const QStringList headers={"ID","Nom","Prénom","Email","Téléphone","Date d'embauche","Salaire","Poste","Statut"};
    for(const auto& h:headers) html += "<th>"+esc(h)+"</th>";
    html += "</tr>";
    for(int r=0;r<ui->tableEmployes->rowCount();++r){
        if(ui->tableEmployes->isRowHidden(r)) continue;
        html += "<tr>";
        for(int c=0;c<9;++c) html += "<td>"+esc(ui->tableEmployes->item(r,c)->text())+"</td>";
        html += "</tr>";
    }
    html += "</table></body></html>";
    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);
    ui->statusbar->showMessage("PDF exporté : " + file, 5000);
}
