#include "mainwindow.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QStringList>
#include <QFont>
#include <QColor>
#include <QPointF>
#include <QtMath>

#include <QPdfWriter>
#include <QPageSize>
#include <QPageLayout>

/* =====================================================
   STYLE DARK
   ===================================================== */

static const char *STYLE_DARK = R"(
QWidget {
    background: #050B14;
    color: #EAF2FF;
    font-family: "Segoe UI";
    font-size: 13px;
}

QMainWindow {
    background: #050B14;
}

QPushButton {
    background: #0B1726;
    color: #DCE7F4;
    border: 1px solid #243A52;
    border-radius: 8px;
    padding: 8px 12px;
    font-weight: bold;
}

QPushButton:hover {
    background: #10283D;
    border-color: #18D9EF;
}

QPushButton#primary {
    background: #18D9EF;
    color: #03212A;
    border-color: #18D9EF;
}

QLineEdit,
QComboBox {
    background: #091522;
    color: white;
    border: 1px solid #243A52;
    border-radius: 8px;
    padding: 9px;
}

QLineEdit:focus,
QComboBox:focus {
    border-color: #18D9EF;
}

QTableWidget {
    background: #081421;
    alternate-background-color: #0B1828;
    border: 1px solid #1A3046;
    gridline-color: #15283B;
    border-radius: 10px;
}

QTableWidget::item {
    padding: 8px;
}

QTableWidget::item:selected {
    background: #0C3042;
}

QHeaderView::section {
    background: #091522;
    color: #18D9EF;
    border: 0;
    padding: 11px;
    font-weight: bold;
}

QFrame#statCard {
    background: #091522;
    border: 1px solid #1C3045;
    border-radius: 10px;
}

QPushButton#iconButton {
    font-size: 17px;
    padding: 2px;
}
)";

/* =====================================================
   STYLE LIGHT
   ===================================================== */

static const char *STYLE_LIGHT = R"(
QWidget {
    background: #F4F7FB;
    color: #182433;
    font-family: "Segoe UI";
    font-size: 13px;
}

QMainWindow {
    background: #F4F7FB;
}

QPushButton {
    background: #FFFFFF;
    color: #243447;
    border: 1px solid #C8D3E0;
    border-radius: 8px;
    padding: 8px 12px;
    font-weight: bold;
}

QPushButton:hover {
    background: #EAFBFD;
    border-color: #18BFD4;
}

QPushButton#primary {
    background: #18D9EF;
    color: #03212A;
    border-color: #18D9EF;
}

QLineEdit,
QComboBox {
    background: #FFFFFF;
    color: #182433;
    border: 1px solid #C8D3E0;
    border-radius: 8px;
    padding: 9px;
}

QLineEdit:focus,
QComboBox:focus {
    border-color: #18BFD4;
}

QTableWidget {
    background: #FFFFFF;
    alternate-background-color: #F1F6FA;
    color: #182433;
    border: 1px solid #D3DDE8;
    gridline-color: #DCE4EC;
    border-radius: 10px;
}

QTableWidget::item {
    padding: 8px;
}

QTableWidget::item:selected {
    background: #DDF7FA;
    color: #182433;
}

QHeaderView::section {
    background: #EAF1F7;
    color: #087F91;
    border: 0;
    padding: 11px;
    font-weight: bold;
}

QFrame#statCard {
    background: #FFFFFF;
    border: 1px solid #D3DDE8;
    border-radius: 10px;
}

QPushButton#iconButton {
    font-size: 17px;
    padding: 2px;
}
)";

/* =====================================================
   EQUIPE
   ===================================================== */

Equipe::Equipe(int id, const QString &nom)
    : m_id(id),
    m_nom(nom)
{
}

bool Equipe::ajouter(const Participant &p)
{
    if (pleine())
        return false;

    for (const Participant &x : m_membres)
    {
        if (x.id == p.id)
            return false;
    }

    m_membres.append(p);
    return true;
}

bool Equipe::supprimer(int id)
{
    for (int i = 0; i < m_membres.size(); ++i)
    {
        if (m_membres[i].id == id)
        {
            m_membres.removeAt(i);
            return true;
        }
    }

    return false;
}

void Equipe::vider()
{
    m_membres.clear();
}

int Equipe::hommes() const
{
    int n = 0;

    for (const Participant &p : m_membres)
    {
        if (p.sexe.compare("Homme",
                           Qt::CaseInsensitive) == 0)
        {
            ++n;
        }
    }

    return n;
}

int Equipe::femmes() const
{
    int n = 0;

    for (const Participant &p : m_membres)
    {
        if (p.sexe.compare("Femme",
                           Qt::CaseInsensitive) == 0)
        {
            ++n;
        }
    }

    return n;
}

double Equipe::ageMoyen() const
{
    if (m_membres.isEmpty())
        return 0.0;

    int total = 0;

    for (const Participant &p : m_membres)
        total += p.age;

    return double(total) / m_membres.size();
}

bool Equipe::aCompetence(const QString &c) const
{
    for (const Participant &p : m_membres)
    {
        if (p.competence.compare(
                c,
                Qt::CaseInsensitive) == 0)
        {
            return true;
        }
    }

    return false;
}

QString Equipe::statut() const
{
    if (m_membres.isEmpty())
        return "Vide";

    if (pleine())
        return "Complete";

    return "Active";
}

/* =====================================================
   SCORE
   ===================================================== */

double scoreEquipe(
    const Equipe &e,
    const Participant &p)
{
    if (e.pleine())
        return -1000000.0;

    double score = (6 - e.taille()) * 6.0;

    int h = e.hommes();
    int f = e.femmes();

    if (p.sexe.compare("Homme",
                       Qt::CaseInsensitive) == 0)
        ++h;

    if (p.sexe.compare("Femme",
                       Qt::CaseInsensitive) == 0)
        ++f;

    int diffSexe = qAbs(h - f);

    if (diffSexe == 0)
        score += 40;
    else if (diffSexe == 1)
        score += 30;
    else if (diffSexe == 2)
        score += 15;
    else
        score -= 20;

    if (e.aCompetence(p.competence))
        score -= 10;
    else
        score += 35;

    if (!e.membres().isEmpty())
    {
        double diffAge =
            qAbs(e.ageMoyen() - p.age);

        if (diffAge <= 2)
            score += 25;
        else if (diffAge <= 5)
            score += 15;
        else
            score -= diffAge;
    }

    return score;
}

/* =====================================================
   CONSTRUCTEUR
   ===================================================== */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_table(nullptr),
    m_recherche(nullptr),
    m_filtre(nullptr),
    m_notification(nullptr),
    m_courbe(nullptr),
    m_totalEquipes(nullptr),
    m_totalMembres(nullptr),
    m_totalHommes(nullptr),
    m_totalFemmes(nullptr),
    m_side(nullptr),
    m_modeClair(false)
{
    resize(1400, 900);

    setWindowTitle(
        "Devena - Gestion des equipes"
        );

    setStyleSheet(STYLE_DARK);

    creerInterface();
    creerDonnees();
    rafraichir();
}

MainWindow::~MainWindow()
{
}

/* =====================================================
   INTERFACE
   ===================================================== */

void MainWindow::creerInterface()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *root =
        new QHBoxLayout(central);

    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    /* ================= SIDEBAR ================= */

    m_side = new QWidget;
    m_side->setFixedWidth(220);

    m_side->setStyleSheet(
        "QWidget{"
        "background:#030914;"
        "border-right:1px solid #17283A;"
        "}"
        "QPushButton{"
        "text-align:left;"
        "border:0;"
        "background:transparent;"
        "color:#718198;"
        "padding:12px;"
        "}"
        "QPushButton#active{"
        "background:#092A3D;"
        "color:#18D9EF;"
        "border:1px solid #18D9EF;"
        "}"
        );

    QVBoxLayout *sl =
        new QVBoxLayout(m_side);

    sl->setContentsMargins(
        15, 22, 15, 18
        );

    QLabel *logo =
        new QLabel("◆ Devena");

    logo->setObjectName("logoLabel");

    logo->setStyleSheet(
        "color:#18D9EF;"
        "font-size:27px;"
        "font-weight:bold;"
        );

    sl->addWidget(logo);

    QLabel *slogan =
        new QLabel(
            "Connecter les talents,\n"
            "construire l'avenir"
            );

    slogan->setObjectName("sloganLabel");

    slogan->setStyleSheet(
        "color:#718198;"
        );

    sl->addWidget(slogan);

    sl->addSpacing(22);

    QStringList menu;

    menu
        << "Tableau de bord"
        << "Participants"
        << "Equipes"
        << "Projets"
        << "Evaluations"
        << "Parametres";

    for (int i = 0; i < menu.size(); ++i)
    {
        QPushButton *b =
            new QPushButton(menu[i]);

        if (i == 2)
        {
            b->setObjectName("active");
        }
        else
        {
            b->setEnabled(false);
        }

        sl->addWidget(b);
    }

    sl->addStretch();

    QLabel *footer =
        new QLabel(
            "Hack today\n"
            "Build tomorrow"
            );

    footer->setObjectName("footerLabel");

    footer->setAlignment(
        Qt::AlignCenter
        );

    footer->setStyleSheet(
        "color:#3A4D65;"
        );

    sl->addWidget(footer);

    root->addWidget(m_side);

    /* ================= CORPS ================= */

    QWidget *body = new QWidget;

    QVBoxLayout *v =
        new QVBoxLayout(body);

    v->setContentsMargins(
        25, 20, 25, 18
        );

    /* ================= HEADER ================= */

    QHBoxLayout *header =
        new QHBoxLayout;

    QVBoxLayout *titles =
        new QVBoxLayout;

    QLabel *title =
        new QLabel("Equipes");

    title->setObjectName("titleLabel");

    title->setStyleSheet(
        "font-size:30px;"
        "font-weight:bold;"
        "color:white;"
        );

    QLabel *subtitle =
        new QLabel(
            "Gestion moderne et intelligente "
            "des equipes"
            );

    subtitle->setObjectName("subtitleLabel");

    subtitle->setStyleSheet(
        "color:#7D8BA3;"
        );

    titles->addWidget(title);
    titles->addWidget(subtitle);

    header->addLayout(titles);
    header->addStretch();

    QLabel *user =
        new QLabel(
            "Alaa\n"
            "Participant"
            );

    user->setObjectName("userLabel");

    header->addWidget(user);

    v->addLayout(header);

    /* ================= ACTIONS ================= */

    QHBoxLayout *actions =
        new QHBoxLayout;

    QPushButton *addTeam =
        new QPushButton(
            "+ Ajouter equipe"
            );

    QPushButton *addMember =
        new QPushButton(
            "+ Ajouter membre"
            );

    QPushButton *addToTeam =
        new QPushButton(
            "+ Dans equipe"
            );

    QPushButton *automatic =
        new QPushButton(
            "⚡ Affectation automatique"
            );

    /* NOUVEAUX BOUTONS */

    QPushButton *exportPDF =
        new QPushButton(
            "📄 Exporter PDF"
            );

    QPushButton *lightMode =
        new QPushButton(
            "☀ Mode clair"
            );

    addTeam->setObjectName("primary");
    automatic->setObjectName("primary");

    /* Même taille que les autres boutons */
    exportPDF->setMinimumHeight(38);
    lightMode->setMinimumHeight(38);

    actions->addWidget(addTeam);
    actions->addWidget(addMember);
    actions->addWidget(addToTeam);
    actions->addWidget(automatic);
    actions->addWidget(exportPDF);
    actions->addWidget(lightMode);
    actions->addStretch();

    v->addLayout(actions);

    connect(
        addTeam,
        &QPushButton::clicked,
        this,
        [this]()
        {
            ajouterEquipe();
        }
        );

    connect(
        addMember,
        &QPushButton::clicked,
        this,
        [this]()
        {
            ajouterMembre(true);
        }
        );

    connect(
        addToTeam,
        &QPushButton::clicked,
        this,
        [this]()
        {
            int row =
                m_table->currentRow();

            if (row < 0)
            {
                QMessageBox::information(
                    this,
                    "Equipe",
                    "Selectionnez une equipe."
                    );
            }
            else
            {
                ajouterMembre(false, row);
            }
        }
        );

    connect(
        automatic,
        &QPushButton::clicked,
        this,
        [this]()
        {
            affecterAutomatiquement();
        }
        );

    /* PDF */

    connect(
        exportPDF,
        &QPushButton::clicked,
        this,
        [this]()
        {
            exporterPDF();
        }
        );

    /* MODE CLAIR */

    connect(
        lightMode,
        &QPushButton::clicked,
        this,
        [this]()
        {
            changerModeClair();
        }
        );

    /* ================= NOTIFICATION ================= */

    m_notification = new QLabel;

    m_notification->setAlignment(
        Qt::AlignCenter
        );

    m_notification->hide();

    v->addWidget(m_notification);

    /* ================= RECHERCHE ================= */

    QHBoxLayout *filters =
        new QHBoxLayout;

    m_recherche =
        new QLineEdit;

    m_recherche->setPlaceholderText(
        "🔎 Rechercher une equipe..."
        );

    m_filtre =
        new QComboBox;

    m_filtre->addItems(
        QStringList()
        << "Toutes les equipes"
        << "Equipe avec membres"
        << "Equipe vide"
        << "Equipe complete"
        );

    filters->addWidget(
        m_recherche,
        1
        );

    filters->addWidget(m_filtre);

    v->addLayout(filters);

    connect(
        m_recherche,
        &QLineEdit::textChanged,
        this,
        [this](const QString &)
        {
            filtrer();
        }
        );

    connect(
        m_filtre,
        &QComboBox::currentTextChanged,
        this,
        [this](const QString &)
        {
            filtrer();
        }
        );

    /* ================= TABLE ================= */

    QLabel *tableTitle =
        new QLabel(
            "Tableau des equipes"
            );

    tableTitle->setObjectName(
        "tableTitleLabel"
        );

    tableTitle->setStyleSheet(
        "font-size:19px;"
        "font-weight:bold;"
        "color:white;"
        );

    v->addWidget(tableTitle);

    m_table =
        new QTableWidget;

    m_table->setColumnCount(6);

    m_table->setHorizontalHeaderLabels(
        QStringList()
        << "ID"
        << "Equipe"
        << "Membres"
        << "H / F"
        << "Age moyen"
        << "Actions"
        );

    m_table->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    m_table->setSelectionMode(
        QAbstractItemView::SingleSelection
        );

    m_table->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    m_table->verticalHeader()
        ->setVisible(false);

    m_table->horizontalHeader()
        ->setSectionResizeMode(
            QHeaderView::ResizeToContents
            );

    m_table->horizontalHeader()
        ->setSectionResizeMode(
            1,
            QHeaderView::Stretch
            );

    m_table->setColumnWidth(
        5,
        300
        );

    v->addWidget(
        m_table,
        1
        );

    /* ================= STATISTIQUES ================= */

    QLabel *statsTitle =
        new QLabel("Statistiques");

    statsTitle->setObjectName(
        "statsTitleLabel"
        );

    statsTitle->setStyleSheet(
        "font-size:20px;"
        "font-weight:bold;"
        "color:white;"
        );

    v->addWidget(statsTitle);

    m_courbe =
        new QLabel;

    m_courbe->setMinimumHeight(
        165
        );

    m_courbe->setAlignment(
        Qt::AlignCenter
        );

    v->addWidget(m_courbe);

    QHBoxLayout *stats =
        new QHBoxLayout;

    auto statCard =
        [](const QString &name,
           QLabel **value)
    {
        QFrame *box =
            new QFrame;

        box->setObjectName(
            "statCard"
            );

        QVBoxLayout *l =
            new QVBoxLayout(box);

        QLabel *n =
            new QLabel(name);

        n->setStyleSheet(
            "color:#7D8BA3;"
            );

        *value =
            new QLabel("0");

        (*value)->setStyleSheet(
            "color:#18D9EF;"
            "font-size:21px;"
            "font-weight:bold;"
            );

        l->addWidget(n);
        l->addWidget(*value);

        return box;
    };

    stats->addWidget(
        statCard(
            "Equipes",
            &m_totalEquipes
            )
        );

    stats->addWidget(
        statCard(
            "Membres",
            &m_totalMembres
            )
        );

    stats->addWidget(
        statCard(
            "Hommes",
            &m_totalHommes
            )
        );

    stats->addWidget(
        statCard(
            "Femmes",
            &m_totalFemmes
            )
        );

    v->addLayout(stats);

    root->addWidget(
        body,
        1
        );
}

/* =====================================================
   DONNEES
   ===================================================== */

void MainWindow::creerDonnees()
{
    m_equipes
        << Equipe(
               1,
               "Equipe Innovation"
               )
        << Equipe(
               2,
               "Equipe Digital"
               )
        << Equipe(
               3,
               "Equipe Data"
               );

    m_participants
        << Participant{
               1,
               "Ben Ali",
               "Yassine",
               "Homme",
               "Web",
               22
           }
        << Participant{
               2,
               "Trabelsi",
               "Aya",
               "Femme",
               "UI/UX",
               21
           }
        << Participant{
               3,
               "Sassi",
               "Omar",
               "Homme",
               "Java",
               23
           }
        << Participant{
               4,
               "Kacem",
               "Salma",
               "Femme",
               "Python",
               22
           }
        << Participant{
               5,
               "Jaziri",
               "Mohamed",
               "Homme",
               "Reseau",
               24
           }
        << Participant{
               6,
               "Hammami",
               "Lina",
               "Femme",
               "Design",
               23
           }
        << Participant{
               7,
               "Mejri",
               "Zeineb",
               "Femme",
               "Marketing",
               21
           }
        << Participant{
               8,
               "Nasri",
               "Ali",
               "Homme",
               "Data",
               25
           }
        << Participant{
               9,
               "Fekih",
               "Rim",
               "Femme",
               "IA",
               22
           }
        << Participant{
               10,
               "Daly",
               "Iheb",
               "Homme",
               "DevOps",
               24
           };
}

/* =====================================================
   IDS
   ===================================================== */

int MainWindow::prochainIdEquipe() const
{
    int id = 0;

    for (const Equipe &e : m_equipes)
        id = qMax(id, e.id());

    return id + 1;
}

int MainWindow::prochainIdParticipant() const
{
    int id = 0;

    for (const Participant &p : m_participants)
        id = qMax(id, p.id);

    return id + 1;
}

/* =====================================================
   MEILLEURE EQUIPE
   ===================================================== */

int MainWindow::meilleureEquipe(
    const Participant &p) const
{
    int bestIndex = -1;

    double bestScore =
        -1000000.0;

    for (int i = 0;
         i < m_equipes.size();
         ++i)
    {
        double score =
            scoreEquipe(
                m_equipes[i],
                p
                );

        if (score > bestScore)
        {
            bestScore = score;
            bestIndex = i;
        }
    }

    return bestIndex;
}

/* =====================================================
   AFFECTATION
   ===================================================== */

void MainWindow::affecterAutomatiquement()
{
    for (Equipe &e : m_equipes)
        e.vider();

    for (const Participant &p :
         m_participants)
    {
        int i =
            meilleureEquipe(p);

        if (i >= 0)
            m_equipes[i].ajouter(p);
    }

    rafraichir();

    afficherNotification(
        "Affectation automatique terminee."
        );
}

/* =====================================================
   AJOUT EQUIPE
   ===================================================== */

void MainWindow::ajouterEquipe()
{
    if (m_equipes.size() >= 6)
    {
        QMessageBox::warning(
            this,
            "Limite",
            "Le nombre maximum d'equipes est 6."
            );

        return;
    }

    bool ok = false;

    QString nom =
        QInputDialog::getText(
            this,
            "Ajouter une equipe",
            "Nom :",
            QLineEdit::Normal,
            "",
            &ok
            ).trimmed();

    if (!ok || nom.isEmpty())
        return;

    for (const Equipe &e :
         m_equipes)
    {
        if (e.nom().compare(
                nom,
                Qt::CaseInsensitive
                ) == 0)
        {
            QMessageBox::warning(
                this,
                "Erreur",
                "Cette equipe existe deja."
                );

            return;
        }
    }

    m_equipes.append(
        Equipe(
            prochainIdEquipe(),
            nom
            )
        );

    rafraichir();

    afficherNotification(
        "Equipe ajoutee : " + nom
        );
}

/* =====================================================
   MODIFIER
   ===================================================== */

void MainWindow::modifierEquipe(
    int index)
{
    if (index < 0 ||
        index >= m_equipes.size())
        return;

    bool ok = false;

    QString nom =
        QInputDialog::getText(
            this,
            "Modifier equipe",
            "Nouveau nom :",
            QLineEdit::Normal,
            m_equipes[index].nom(),
            &ok
            ).trimmed();

    if (!ok || nom.isEmpty())
        return;

    m_equipes[index].setNom(nom);

    rafraichir();

    afficherNotification(
        "Equipe modifiee."
        );
}

/* =====================================================
   SUPPRIMER
   ===================================================== */

void MainWindow::supprimerEquipe(
    int index)
{
    if (index < 0 ||
        index >= m_equipes.size())
        return;

    if (m_equipes.size() == 1 &&
        !m_participants.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Impossible",
            "Gardez au moins une equipe."
            );

        return;
    }

    QMessageBox::StandardButton reponse =
        QMessageBox::question(
            this,
            "Confirmation",
            "Supprimer " +
                m_equipes[index].nom() +
                " ?"
            );

    if (reponse != QMessageBox::Yes)
        return;

    m_equipes.removeAt(index);

    if (!m_equipes.isEmpty())
        affecterAutomatiquement();
    else
        rafraichir();

    afficherNotification(
        "Equipe supprimee."
        );
}

/* =====================================================
   AJOUT MEMBRE
   ===================================================== */

void MainWindow::ajouterMembre(
    bool automatique,
    int indexEquipe)
{
    if (m_equipes.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Erreur",
            "Ajoutez une equipe d'abord."
            );

        return;
    }

    bool ok = false;

    QString nom =
        QInputDialog::getText(
            this,
            "Ajouter membre",
            "Nom :",
            QLineEdit::Normal,
            "",
            &ok
            ).trimmed();

    if (!ok || nom.isEmpty())
        return;

    QString prenom =
        QInputDialog::getText(
            this,
            "Ajouter membre",
            "Prenom :",
            QLineEdit::Normal,
            "",
            &ok
            ).trimmed();

    if (!ok || prenom.isEmpty())
        return;

    QString sexe =
        QInputDialog::getItem(
            this,
            "Ajouter membre",
            "Sexe :",
            QStringList()
                << "Homme"
                << "Femme",
            0,
            false,
            &ok
            );

    if (!ok)
        return;

    int age =
        QInputDialog::getInt(
            this,
            "Ajouter membre",
            "Age :",
            22,
            16,
            80,
            1,
            &ok
            );

    if (!ok)
        return;

    QString competence =
        QInputDialog::getText(
            this,
            "Ajouter membre",
            "Competence :",
            QLineEdit::Normal,
            "",
            &ok
            ).trimmed();

    if (!ok ||
        competence.isEmpty())
        return;

    Participant p{
        prochainIdParticipant(),
        nom,
        prenom,
        sexe,
        competence,
        age
    };

    int index =
        automatique
            ? meilleureEquipe(p)
            : indexEquipe;

    if (index < 0 ||
        index >= m_equipes.size())
    {
        QMessageBox::warning(
            this,
            "Erreur",
            "Aucune equipe disponible."
            );

        return;
    }

    if (!m_equipes[index].ajouter(p))
    {
        QMessageBox::warning(
            this,
            "Equipe pleine",
            "Cette equipe contient deja 6 membres."
            );

        return;
    }

    m_participants.append(p);

    rafraichir();

    afficherNotification(
        "🔔 Nouveau membre : " +
        prenom +
        " " +
        nom +
        " → " +
        m_equipes[index].nom()
        );
}

/* =====================================================
   VOIR MEMBRES
   ===================================================== */

void MainWindow::voirMembres(
    int index)
{
    if (index < 0 ||
        index >= m_equipes.size())
        return;

    QString texte =
        "Equipe : " +
        m_equipes[index].nom() +
        "\n\n";

    const QVector<Participant> &liste =
        m_equipes[index].membres();

    if (liste.isEmpty())
    {
        texte += "Aucun membre.";
    }
    else
    {
        for (const Participant &p :
             liste)
        {
            texte +=
                QString(
                    "P%1 - %2 %3 | %4 | %5 ans | %6\n"
                    )
                    .arg(p.id)
                    .arg(p.prenom)
                    .arg(p.nom)
                    .arg(p.sexe)
                    .arg(p.age)
                    .arg(p.competence);
        }
    }

    QMessageBox::information(
        this,
        "Membres",
        texte
        );
}

/* =====================================================
   SUPPRIMER MEMBRE
   ===================================================== */

void MainWindow::supprimerMembre(
    int index)
{
    if (index < 0 ||
        index >= m_equipes.size())
        return;

    QVector<Participant> liste =
        m_equipes[index].membres();

    if (liste.isEmpty())
    {
        QMessageBox::information(
            this,
            "Membre",
            "Cette equipe est vide."
            );

        return;
    }

    QStringList choix;

    for (const Participant &p :
         liste)
    {
        choix
            << QString(
                   "P%1 - %2 %3"
                   )
                   .arg(p.id)
                   .arg(p.prenom)
                   .arg(p.nom);
    }

    bool ok = false;

    QString selected =
        QInputDialog::getItem(
            this,
            "Retirer membre",
            "Membre :",
            choix,
            0,
            false,
            &ok
            );

    if (!ok)
        return;

    int pos =
        choix.indexOf(selected);

    if (pos < 0)
        return;

    int id =
        liste[pos].id;

    m_equipes[index].supprimer(id);

    for (int i = 0;
         i < m_participants.size();
         ++i)
    {
        if (m_participants[i].id == id)
        {
            m_participants.removeAt(i);
            break;
        }
    }

    rafraichir();

    afficherNotification(
        "Membre retire."
        );
}

/* =====================================================
   QR
   ===================================================== */

QImage MainWindow::creerQR(
    const QString &code) const
{
    const int n = 21;
    const int s = 7;

    QImage image(
        (n + 8) * s,
        (n + 8) * s,
        QImage::Format_RGB32
        );

    image.fill(Qt::white);

    QPainter p(&image);

    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);

    int total = 0;

    for (char c :
         code.toUtf8())
    {
        total +=
            static_cast<unsigned char>(c);
    }

    for (int y = 0;
         y < n;
         ++y)
    {
        for (int x = 0;
             x < n;
             ++x)
        {
            bool fixed =
                (x < 7 && y < 7) ||
                (x >= n - 7 && y < 7) ||
                (x < 7 && y >= n - 7);

            if (fixed)
                continue;

            int value =
                x * 31 +
                y * 17 +
                total;

            if (value % 5 == 0 ||
                value % 11 == 0)
            {
                p.drawRect(
                    (x + 4) * s,
                    (y + 4) * s,
                    s,
                    s
                    );
            }
        }
    }

    auto finder =
        [&](int ox, int oy)
    {
        for (int y = 0;
             y < 7;
             ++y)
        {
            for (int x = 0;
                 x < 7;
                 ++x)
            {
                bool black =
                    x == 0 ||
                    x == 6 ||
                    y == 0 ||
                    y == 6 ||
                    (
                        x >= 2 &&
                        x <= 4 &&
                        y >= 2 &&
                        y <= 4
                        );

                if (black)
                {
                    p.drawRect(
                        (ox + x + 4) * s,
                        (oy + y + 4) * s,
                        s,
                        s
                        );
                }
            }
        }
    };

    finder(0, 0);
    finder(n - 7, 0);
    finder(0, n - 7);

    p.end();

    return image;
}

/* =====================================================
   AFFICHER QR
   ===================================================== */

void MainWindow::afficherQR(
    int index)
{
    if (index < 0 ||
        index >= m_equipes.size())
        return;

    QString code =
        "DEVE-EQ-" +
        QString::number(
            m_equipes[index].id()
            );

    QImage image =
        creerQR(code);

    QDialog dialog(this);

    dialog.setWindowTitle(
        "QR - " +
        m_equipes[index].nom()
        );

    dialog.resize(
        340,
        420
        );

    QVBoxLayout *v =
        new QVBoxLayout(&dialog);

    QLabel *title =
        new QLabel(
            m_equipes[index].nom()
            );

    title->setAlignment(
        Qt::AlignCenter
        );

    title->setStyleSheet(
        "color:#18D9EF;"
        "font-size:20px;"
        "font-weight:bold;"
        );

    QLabel *img =
        new QLabel;

    img->setAlignment(
        Qt::AlignCenter
        );

    img->setPixmap(
        QPixmap::fromImage(image)
        );

    QPushButton *save =
        new QPushButton(
            "Enregistrer PNG"
            );

    QPushButton *close =
        new QPushButton(
            "Fermer"
            );

    v->addWidget(title);
    v->addWidget(img);
    v->addWidget(new QLabel(code));
    v->addWidget(save);
    v->addWidget(close);

    connect(
        save,
        &QPushButton::clicked,
        [this, &dialog, image]()
        {
            QString file =
                QFileDialog::getSaveFileName(
                    this,
                    "Enregistrer QR",
                    "qr_equipe.png",
                    "PNG (*.png)"
                    );

            if (!file.isEmpty())
                image.save(file);
        }
        );

    connect(
        close,
        &QPushButton::clicked,
        &dialog,
        &QDialog::accept
        );

    dialog.exec();
}

/* =====================================================
   BADGE
   ===================================================== */

QImage MainWindow::creerBadge(
    const Equipe &e) const
{
    QImage image(
        900,
        500,
        QImage::Format_RGB32
        );

    image.fill(
        QColor("#081522")
        );

    QPainter p(&image);

    p.setRenderHint(
        QPainter::Antialiasing
        );

    p.setPen(
        QPen(
            QColor("#18D9EF"),
            3
            )
        );

    p.drawRoundedRect(
        10,
        10,
        880,
        480,
        20,
        20
        );

    p.setPen(
        QColor("#18D9EF")
        );

    p.setFont(
        QFont(
            "Segoe UI",
            24,
            QFont::Bold
            )
        );

    p.drawText(
        40,
        55,
        "◆ Devena"
        );

    p.setPen(Qt::white);

    p.setFont(
        QFont(
            "Segoe UI",
            30,
            QFont::Bold
            )
        );

    p.drawText(
        40,
        120,
        e.nom()
        );

    p.setFont(
        QFont(
            "Segoe UI",
            16
            )
        );

    p.drawText(
        45,
        170,
        "Equipe #" +
            QString::number(e.id())
        );

    p.drawText(
        45,
        220,
        "Membres : " +
            QString::number(e.taille()) +
            " / 6"
        );

    p.drawText(
        45,
        255,
        "Hommes : " +
            QString::number(e.hommes())
        );

    p.drawText(
        45,
        290,
        "Femmes : " +
            QString::number(e.femmes())
        );

    p.drawText(
        45,
        325,
        "Age moyen : " +
            QString::number(
                e.ageMoyen(),
                'f',
                1
                )
        );

    QImage qr =
        creerQR(
            "DEVE-EQ-" +
            QString::number(e.id())
            );

    qr = qr.scaled(
        190,
        190,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    p.drawImage(
        660,
        100,
        qr
        );

    p.end();

    return image;
}

/* =====================================================
   AFFICHER BADGE
   ===================================================== */

void MainWindow::afficherBadge(
    int index)
{
    if (index < 0 ||
        index >= m_equipes.size())
        return;

    QImage image =
        creerBadge(
            m_equipes[index]
            );

    QDialog dialog(this);

    dialog.setWindowTitle(
        "Badge - " +
        m_equipes[index].nom()
        );

    dialog.resize(
        950,
        600
        );

    QVBoxLayout *v =
        new QVBoxLayout(&dialog);

    QLabel *img =
        new QLabel;

    img->setAlignment(
        Qt::AlignCenter
        );

    img->setPixmap(
        QPixmap::fromImage(image)
            .scaled(
                900,
                500,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
        );

    QPushButton *save =
        new QPushButton(
            "Enregistrer le badge"
            );

    QPushButton *close =
        new QPushButton(
            "Fermer"
            );

    v->addWidget(
        img,
        1
        );

    v->addWidget(save);
    v->addWidget(close);

    connect(
        save,
        &QPushButton::clicked,
        [this, &dialog, image]()
        {
            QString file =
                QFileDialog::getSaveFileName(
                    this,
                    "Enregistrer le badge",
                    "badge_equipe.png",
                    "PNG (*.png)"
                    );

            if (!file.isEmpty())
                image.save(file);
        }
        );

    connect(
        close,
        &QPushButton::clicked,
        &dialog,
        &QDialog::accept
        );

    dialog.exec();
}

/* =====================================================
   AFFICHER EQUIPES
   ===================================================== */

void MainWindow::afficherEquipes()
{
    m_table->clearContents();

    m_table->setRowCount(
        m_equipes.size()
        );

    for (int i = 0;
         i < m_equipes.size();
         ++i)
    {
        const Equipe &e =
            m_equipes[i];

        m_table->setItem(
            i,
            0,
            new QTableWidgetItem(
                "E" +
                QString::number(
                    e.id()
                    )
                )
            );

        m_table->setItem(
            i,
            1,
            new QTableWidgetItem(
                e.nom()
                )
            );

        m_table->setItem(
            i,
            2,
            new QTableWidgetItem(
                QString::number(
                    e.taille()
                    ) +
                " / 6"
                )
            );

        m_table->setItem(
            i,
            3,
            new QTableWidgetItem(
                QString::number(
                    e.hommes()
                    ) +
                " H / " +
                QString::number(
                    e.femmes()
                    ) +
                " F"
                )
            );

        m_table->setItem(
            i,
            4,
            new QTableWidgetItem(
                QString::number(
                    e.ageMoyen(),
                    'f',
                    1
                    )
                )
            );

        QWidget *actions =
            new QWidget;

        QHBoxLayout *a =
            new QHBoxLayout(actions);

        a->setContentsMargins(
            2, 2, 2, 2
            );

        a->setSpacing(6);

        QPushButton *eye =
            new QPushButton("👁");

        QPushButton *edit =
            new QPushButton("✎");

        QPushButton *trash =
            new QPushButton("🗑");

        QPushButton *qr =
            new QPushButton("▣");

        QPushButton *badge =
            new QPushButton("★");

        eye->setObjectName("iconButton");
        edit->setObjectName("iconButton");
        trash->setObjectName("iconButton");
        qr->setObjectName("iconButton");
        badge->setObjectName("iconButton");

        eye->setFixedSize(34, 30);
        edit->setFixedSize(34, 30);
        trash->setFixedSize(34, 30);
        qr->setFixedSize(34, 30);
        badge->setFixedSize(34, 30);

        eye->setToolTip(
            "Voir les membres"
            );

        edit->setToolTip(
            "Modifier l'equipe"
            );

        trash->setToolTip(
            "Supprimer l'equipe"
            );

        qr->setToolTip(
            "Afficher le QR"
            );

        badge->setToolTip(
            "Afficher le badge"
            );

        a->addWidget(eye);
        a->addWidget(edit);
        a->addWidget(trash);
        a->addWidget(qr);
        a->addWidget(badge);

        m_table->setCellWidget(
            i,
            5,
            actions
            );

        m_table->setRowHeight(
            i,
            46
            );

        connect(
            eye,
            &QPushButton::clicked,
            [this, i]()
            {
                voirMembres(i);
            }
            );

        connect(
            edit,
            &QPushButton::clicked,
            [this, i]()
            {
                modifierEquipe(i);
            }
            );

        connect(
            trash,
            &QPushButton::clicked,
            [this, i]()
            {
                supprimerEquipe(i);
            }
            );

        connect(
            qr,
            &QPushButton::clicked,
            [this, i]()
            {
                afficherQR(i);
            }
            );

        connect(
            badge,
            &QPushButton::clicked,
            [this, i]()
            {
                afficherBadge(i);
            }
            );
    }
}

/* =====================================================
   COURBE
   ===================================================== */

QPixmap MainWindow::creerCourbe() const
{
    const int W = 1000;
    const int H = 170;

    QPixmap pix(W, H);

    pix.fill(
        m_modeClair
            ? QColor("#FFFFFF")
            : QColor("#081421")
        );

    QPainter p(&pix);

    p.setRenderHint(
        QPainter::Antialiasing
        );

    p.setPen(
        m_modeClair
            ? QColor("#D5DEE8")
            : QColor("#162A3E")
        );

    for (int y = 30;
         y < 145;
         y += 28)
    {
        p.drawLine(
            40,
            y,
            W - 25,
            y
            );
    }

    p.setPen(
        m_modeClair
            ? QColor("#526579")
            : QColor("#8EA0B7")
        );

    p.drawText(
        40,
        18,
        "Membres par equipe"
        );

    if (!m_equipes.isEmpty())
    {
        QVector<QPointF> points;

        for (int i = 0;
             i < m_equipes.size();
             ++i)
        {
            double x =
                (
                    m_equipes.size() == 1
                    )
                    ? W / 2.0
                    : 55 +
                          i *
                              (
                                  900.0 /
                                  (
                                      m_equipes.size() - 1
                                      )
                                  );

            double y =
                135 -
                m_equipes[i].taille() * 18;

            points.append(
                QPointF(x, y)
                );
        }

        p.setPen(
            QPen(
                QColor("#18BFD4"),
                3
                )
            );

        for (int i = 1;
             i < points.size();
             ++i)
        {
            p.drawLine(
                points[i - 1],
                points[i]
                );
        }

        for (int i = 0;
             i < points.size();
             ++i)
        {
            p.setBrush(
                QColor("#18BFD4")
                );

            p.drawEllipse(
                points[i],
                5,
                5
                );

            p.setPen(
                m_modeClair
                    ? QColor("#182433")
                    : Qt::white
                );

            p.drawText(
                points[i].x() - 10,
                points[i].y() - 12,
                QString::number(
                    m_equipes[i].taille()
                    )
                );

            p.drawText(
                points[i].x() - 8,
                160,
                "E" +
                    QString::number(
                        m_equipes[i].id()
                        )
                );
        }
    }

    p.end();

    return pix;
}

/* =====================================================
   STATISTIQUES
   ===================================================== */

void MainWindow::afficherStatistiques()
{
    int total = 0;
    int h = 0;
    int f = 0;

    for (const Equipe &e :
         m_equipes)
    {
        total += e.taille();
        h += e.hommes();
        f += e.femmes();
    }

    m_totalEquipes->setText(
        QString::number(
            m_equipes.size()
            )
        );

    m_totalMembres->setText(
        QString::number(total)
        );

    m_totalHommes->setText(
        QString::number(h)
        );

    m_totalFemmes->setText(
        QString::number(f)
        );

    m_courbe->setPixmap(
        creerCourbe()
        );
}

/* =====================================================
   FILTRE
   ===================================================== */

void MainWindow::filtrer()
{
    QString search =
        m_recherche->text()
            .trimmed()
            .toLower();

    QString filter =
        m_filtre->currentText();

    for (int i = 0;
         i < m_equipes.size();
         ++i)
    {
        const Equipe &e =
            m_equipes[i];

        bool visible =
            search.isEmpty() ||
            e.nom()
                .toLower()
                .contains(search);

        if (filter ==
            "Equipe avec membres")
        {
            visible =
                visible &&
                e.taille() > 0;
        }
        else if (filter ==
                 "Equipe vide")
        {
            visible =
                visible &&
                e.taille() == 0;
        }
        else if (filter ==
                 "Equipe complete")
        {
            visible =
                visible &&
                e.pleine();
        }

        m_table->setRowHidden(
            i,
            !visible
            );
    }
}

/* =====================================================
   NOTIFICATION
   ===================================================== */

void MainWindow::afficherNotification(
    const QString &texte,
    bool succes)
{
    m_notification->setText(
        texte
        );

    m_notification->setStyleSheet(
        succes
            ? "background:#0A3A30;"
              "color:#67F0B5;"
              "border:1px solid #1BAA7A;"
              "border-radius:8px;"
              "padding:7px;"
              "font-weight:bold;"

            : "background:#421F28;"
              "color:#FF8C99;"
              "border:1px solid #C44858;"
              "border-radius:8px;"
              "padding:7px;"
              "font-weight:bold;"
        );

    m_notification->show();

    QTimer::singleShot(
        4000,
        m_notification,
        &QLabel::hide
        );
}

/* =====================================================
   EXPORT PDF
   ===================================================== */

void MainWindow::exporterPDF()
{
    QString file =
        QFileDialog::getSaveFileName(
            this,
            "Exporter les equipes en PDF",
            "Devena_Equipes.pdf",
            "PDF (*.pdf)"
            );

    if (file.isEmpty())
        return;

    QPdfWriter pdf(file);

    pdf.setPageSize(
        QPageSize(QPageSize::A4)
        );

    pdf.setPageOrientation(
        QPageLayout::Landscape
        );

    pdf.setResolution(96);

    QPainter painter(&pdf);

    if (!painter.isActive())
    {
        QMessageBox::warning(
            this,
            "Erreur",
            "Impossible de creer le PDF."
            );

        return;
    }

    QRect page =
        painter.viewport();

    int left = 50;
    int top = 50;

    painter.setPen(
        QColor("#087F91")
        );

    painter.setFont(
        QFont(
            "Segoe UI",
            24,
            QFont::Bold
            )
        );

    painter.drawText(
        left,
        top + 25,
        "◆ Devena"
        );

    painter.setPen(Qt::black);

    painter.setFont(
        QFont(
            "Segoe UI",
            18,
            QFont::Bold
            )
        );

    painter.drawText(
        left,
        top + 65,
        "Liste des equipes"
        );

    painter.setFont(
        QFont(
            "Segoe UI",
            10
            )
        );

    painter.drawText(
        left,
        top + 90,
        "Gestion des equipes - Hackathon"
        );

    int y = top + 125;

    int colID = left;
    int colEquipe = left + 65;
    int colMembres = left + 300;
    int colHF = left + 405;
    int colAge = left + 500;
    int colStatut = left + 600;

    painter.setBrush(
        QColor("#EAF1F7")
        );

    painter.setPen(
        QColor("#087F91")
        );

    painter.drawRect(
        left,
        y,
        page.width() - 100,
        35
        );

    painter.setFont(
        QFont(
            "Segoe UI",
            10,
            QFont::Bold
            )
        );

    painter.drawText(
        colID + 8,
        y + 23,
        "ID"
        );

    painter.drawText(
        colEquipe + 8,
        y + 23,
        "Equipe"
        );

    painter.drawText(
        colMembres + 8,
        y + 23,
        "Membres"
        );

    painter.drawText(
        colHF + 8,
        y + 23,
        "H / F"
        );

    painter.drawText(
        colAge + 8,
        y + 23,
        "Age moyen"
        );

    painter.drawText(
        colStatut + 8,
        y + 23,
        "Statut"
        );

    y += 35;

    painter.setFont(
        QFont(
            "Segoe UI",
            10
            )
        );

    painter.setPen(
        Qt::black
        );

    for (const Equipe &e : m_equipes)
    {
        if (y > page.height() - 100)
        {
            pdf.newPage();
            y = 70;
        }

        painter.setPen(
            QColor("#D5DEE8")
            );

        painter.drawLine(
            left,
            y + 35,
            page.width() - 50,
            y + 35
            );

        painter.setPen(Qt::black);

        painter.drawText(
            colID + 8,
            y + 22,
            "E" +
                QString::number(e.id())
            );

        painter.drawText(
            colEquipe + 8,
            y + 22,
            e.nom()
            );

        painter.drawText(
            colMembres + 8,
            y + 22,
            QString::number(e.taille()) +
                " / 6"
            );

        painter.drawText(
            colHF + 8,
            y + 22,
            QString::number(e.hommes()) +
                " / " +
                QString::number(e.femmes())
            );

        painter.drawText(
            colAge + 8,
            y + 22,
            QString::number(
                e.ageMoyen(),
                'f',
                1
                )
            );

        painter.drawText(
            colStatut + 8,
            y + 22,
            e.statut()
            );

        y += 36;

        /* Membres */

        for (const Participant &p :
             e.membres())
        {
            if (y > page.height() - 100)
            {
                pdf.newPage();
                y = 70;
            }

            painter.setPen(
                QColor("#526579")
                );

            QString membre =
                "   - " +
                p.prenom +
                " " +
                p.nom +
                " | " +
                p.sexe +
                " | " +
                QString::number(p.age) +
                " ans | " +
                p.competence;

            painter.drawText(
                colEquipe + 8,
                y + 18,
                membre
                );

            y += 25;
        }

        y += 10;
    }

    /* STATISTIQUES */

    if (y > page.height() - 180)
    {
        pdf.newPage();
        y = 70;
    }

    int total = 0;
    int hommes = 0;
    int femmes = 0;

    for (const Equipe &e : m_equipes)
    {
        total += e.taille();
        hommes += e.hommes();
        femmes += e.femmes();
    }

    painter.setPen(
        QColor("#087F91")
        );

    painter.setFont(
        QFont(
            "Segoe UI",
            14,
            QFont::Bold
            )
        );

    painter.drawText(
        left,
        y + 30,
        "Statistiques"
        );

    painter.setPen(Qt::black);

    painter.setFont(
        QFont(
            "Segoe UI",
            11
            )
        );

    painter.drawText(
        left,
        y + 60,
        "Nombre d'equipes : " +
            QString::number(
                m_equipes.size()
                )
        );

    painter.drawText(
        left,
        y + 85,
        "Nombre de membres : " +
            QString::number(total)
        );

    painter.drawText(
        left,
        y + 110,
        "Hommes : " +
            QString::number(hommes)
        );

    painter.drawText(
        left,
        y + 135,
        "Femmes : " +
            QString::number(femmes)
        );

    painter.end();

    QMessageBox::information(
        this,
        "Export PDF",
        "Le fichier PDF a ete exporte avec succes."
        );
}

/* =====================================================
   MODE CLAIR
   ===================================================== */

void MainWindow::changerModeClair()
{
    m_modeClair = !m_modeClair;

    if (m_modeClair)
    {
        setStyleSheet(STYLE_LIGHT);

        m_side->setStyleSheet(
            "QWidget{"
            "background:#FFFFFF;"
            "border-right:1px solid #D6E0EA;"
            "}"
            "QPushButton{"
            "text-align:left;"
            "border:0;"
            "background:transparent;"
            "color:#64748B;"
            "padding:12px;"
            "}"
            "QPushButton#active{"
            "background:#E3F8FB;"
            "color:#087F91;"
            "border:1px solid #18D9EF;"
            "}"
            );

        QLabel *logo =
            findChild<QLabel *>("logoLabel");

        QLabel *slogan =
            findChild<QLabel *>("sloganLabel");

        QLabel *footer =
            findChild<QLabel *>("footerLabel");

        QLabel *title =
            findChild<QLabel *>("titleLabel");

        QLabel *subtitle =
            findChild<QLabel *>("subtitleLabel");

        QLabel *tableTitle =
            findChild<QLabel *>("tableTitleLabel");

        QLabel *statsTitle =
            findChild<QLabel *>("statsTitleLabel");

        if (logo)
            logo->setStyleSheet(
                "color:#087F91;"
                "font-size:27px;"
                "font-weight:bold;"
                );

        if (slogan)
            slogan->setStyleSheet(
                "color:#64748B;"
                );

        if (footer)
            footer->setStyleSheet(
                "color:#94A3B8;"
                );

        if (title)
            title->setStyleSheet(
                "font-size:30px;"
                "font-weight:bold;"
                "color:#182433;"
                );

        if (subtitle)
            subtitle->setStyleSheet(
                "color:#64748B;"
                );

        if (tableTitle)
            tableTitle->setStyleSheet(
                "font-size:19px;"
                "font-weight:bold;"
                "color:#182433;"
                );

        if (statsTitle)
            statsTitle->setStyleSheet(
                "font-size:20px;"
                "font-weight:bold;"
                "color:#182433;"
                );
    }
    else
    {
        setStyleSheet(STYLE_DARK);

        m_side->setStyleSheet(
            "QWidget{"
            "background:#030914;"
            "border-right:1px solid #17283A;"
            "}"
            "QPushButton{"
            "text-align:left;"
            "border:0;"
            "background:transparent;"
            "color:#718198;"
            "padding:12px;"
            "}"
            "QPushButton#active{"
            "background:#092A3D;"
            "color:#18D9EF;"
            "border:1px solid #18D9EF;"
            "}"
            );

        QLabel *logo =
            findChild<QLabel *>("logoLabel");

        QLabel *slogan =
            findChild<QLabel *>("sloganLabel");

        QLabel *footer =
            findChild<QLabel *>("footerLabel");

        QLabel *title =
            findChild<QLabel *>("titleLabel");

        QLabel *subtitle =
            findChild<QLabel *>("subtitleLabel");

        QLabel *tableTitle =
            findChild<QLabel *>("tableTitleLabel");

        QLabel *statsTitle =
            findChild<QLabel *>("statsTitleLabel");

        if (logo)
            logo->setStyleSheet(
                "color:#18D9EF;"
                "font-size:27px;"
                "font-weight:bold;"
                );

        if (slogan)
            slogan->setStyleSheet(
                "color:#718198;"
                );

        if (footer)
            footer->setStyleSheet(
                "color:#3A4D65;"
                );

        if (title)
            title->setStyleSheet(
                "font-size:30px;"
                "font-weight:bold;"
                "color:white;"
                );

        if (subtitle)
            subtitle->setStyleSheet(
                "color:#7D8BA3;"
                );

        if (tableTitle)
            tableTitle->setStyleSheet(
                "font-size:19px;"
                "font-weight:bold;"
                "color:white;"
                );

        if (statsTitle)
            statsTitle->setStyleSheet(
                "font-size:20px;"
                "font-weight:bold;"
                "color:white;"
                );
    }

    afficherStatistiques();
}

/* =====================================================
   RAFRAICHIR
   ===================================================== */

void MainWindow::rafraichir()
{
    afficherEquipes();
    afficherStatistiques();
    filtrer();
}