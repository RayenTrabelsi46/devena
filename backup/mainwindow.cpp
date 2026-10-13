#include "mainwindow.h"

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QMessageBox>
#include <QDialog>
#include <QPainter>
#include <QTimer>
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSet>
#include <QAbstractAnimation>
#include <QFileDialog>
#include <QTextDocument>
#include <QPdfWriter>
#include <QPageSize>

// ============================================================
// GRAPHIQUE DES STATISTIQUES
// ============================================================

class StatsChart : public QWidget
{
public:
    explicit StatsChart(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(180);
    }

    QList<int> valeurs;
    QStringList noms;

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor("#111927"));

        painter.setPen(QColor("#24344D"));

        for (int i = 1; i <= 4; ++i)
        {
            int y = 25 + i * 30;
            painter.drawLine(30, y, width() - 30, y);
        }

        if (valeurs.isEmpty())
            return;

        int maxValue = 1;

        for (int value : valeurs)
        {
            if (value > maxValue)
                maxValue = value;
        }

        QVector<QPoint> points;

        int espace = 0;

        if (valeurs.size() > 1)
        {
            espace = (width() - 80) / (valeurs.size() - 1);
        }

        for (int i = 0; i < valeurs.size(); ++i)
        {
            int x = (valeurs.size() == 1)
            ? width() / 2
            : 40 + i * espace;

            int y = 150 - (valeurs[i] * 110 / maxValue);

            points.append(QPoint(x, y));
        }

        painter.setPen(QPen(QColor("#25C7E8"), 3));

        for (int i = 0; i < points.size() - 1; ++i)
        {
            painter.drawLine(points[i], points[i + 1]);
        }

        for (int i = 0; i < points.size(); ++i)
        {
            painter.setBrush(QColor("#25C7E8"));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(points[i], 6, 6);

            painter.setPen(QColor("#8B9BB4"));

            QString nom = noms.value(i, "Projet");
            if (nom.length() > 12)
                nom = nom.left(12) + "...";

            painter.drawText(
                points[i].x() - 50,
                175,
                100,
                20,
                Qt::AlignCenter,
                nom
                );

            painter.drawText(
                points[i].x() - 25,
                points[i].y() - 25,
                50,
                20,
                Qt::AlignCenter,
                QString::number(valeurs[i] / 10.0, 'f', 1)
                );
        }
    }
};

// ============================================================
// CONFETTIS
// ============================================================

class ConfettiWidget : public QWidget
{
public:
    explicit ConfettiWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);

        for (int i = 0; i < 120; ++i)
        {
            Particle p;

            p.x = QRandomGenerator::global()->bounded(1300);
            p.y = QRandomGenerator::global()->bounded(-800, 0);
            p.speed = QRandomGenerator::global()->bounded(2, 7);
            p.size = QRandomGenerator::global()->bounded(5, 12);
            p.rotation = QRandomGenerator::global()->bounded(0, 360);
            p.rotationSpeed = QRandomGenerator::global()->bounded(-8, 9);
            p.colorIndex = QRandomGenerator::global()->bounded(5);

            particles.append(p);
        }

        timer = new QTimer(this);

        connect(
            timer,
            &QTimer::timeout,
            this,
            [this]()
            {
                updateParticles();
                update();
            }
            );
    }

    void start()
    {
        show();
        raise();
        timer->start(30);
    }

    void stop()
    {
        timer->stop();
        hide();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const QColor couleurs[] =
            {
                QColor("#25C7E8"),
                QColor("#ffcc00"),
                QColor("#ff4d6d"),
                QColor("#7cff00"),
                QColor("#c66cff")
            };

        for (const Particle &p : particles)
        {
            painter.save();
            painter.translate(p.x, p.y);
            painter.rotate(p.rotation);
            painter.setBrush(couleurs[p.colorIndex]);
            painter.setPen(Qt::NoPen);
            painter.drawRect(
                -p.size / 2,
                -p.size / 2,
                p.size,
                p.size
                );
            painter.restore();
        }
    }

private:
    struct Particle
    {
        float x;
        float y;
        float speed;
        int size;
        float rotation;
        float rotationSpeed;
        int colorIndex;
    };

    QList<Particle> particles;
    QTimer *timer = nullptr;

    void updateParticles()
    {
        for (Particle &p : particles)
        {
            p.y += p.speed;
            p.rotation += p.rotationSpeed;

            if (p.y > height() + 20)
            {
                p.x = QRandomGenerator::global()->bounded(qMax(1, width()));
                p.y = QRandomGenerator::global()->bounded(-100, 0);
                p.speed = QRandomGenerator::global()->bounded(2, 7);
            }
        }
    }
};

static StatsChart *g_statsChart = nullptr;
static QString g_gagnant;

// ============================================================
// CONSTRUCTEUR
// ============================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1300, 800);
    setWindowTitle("Devena - Gestion des projets");

    creerInterface();

    // Données de démonstration
    Projet p1;
    p1.ID_Projet = "P001";
    p1.titre = "Smart City";
    p1.description = "Application intelligente pour la ville.";
    p1.thematique = "IA";
    p1.noteFinale = 17.5;
    p1.ID_Equipe = "E001";

    Projet p2;
    p2.ID_Projet = "P002";
    p2.titre = "Health App";
    p2.description = "Application mobile de santé.";
    p2.thematique = "Web";
    p2.noteFinale = 15.0;
    p2.ID_Equipe = "E002";

    Projet p3;
    p3.ID_Projet = "P003";
    p3.titre = "EduTech";
    p3.description = "Plateforme éducative.";
    p3.thematique = "IoT";
    p3.noteFinale = 18.0;
    p3.ID_Equipe = "E003";

    projets.append(p1);
    projets.append(p2);
    projets.append(p3);

    afficherListe();
}

// ============================================================
// INTERFACE
// ============================================================

void MainWindow::creerInterface()
{
    QWidget *central = new QWidget;
    central->setStyleSheet("font-family:\"Segoe UI\"; font-size:13px;");
    setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ========================================================
    // SIDEBAR
    // ========================================================

    QWidget *sidebar = new QWidget;
    sidebar->setFixedWidth(235);
    sidebar->setStyleSheet("background:#152032;");

    QVBoxLayout *side = new QVBoxLayout(sidebar);
    side->setContentsMargins(16, 25, 16, 20);

    QLabel *logo = new QLabel("◆ Devena");
    logo->setStyleSheet(
        "color:#25C7E8;"
        "font-size:24px;"
        "font-weight:500;"
        );
    side->addWidget(logo);

    QLabel *slogan = new QLabel(
        "Connecter les talents,\n"
        "construire l'avenir"
        );
    slogan->setStyleSheet(
        "color:#8B9BB4;"
        "font-size:12px;"
        );
    side->addWidget(slogan);
    // ========================================================
    // FLYER HACKATHON
    // ========================================================

    QLabel *flyer = new QLabel;
    QPixmap image(":/hackathon.jpg");
    if(!image.isNull()) flyer->setPixmap(image.scaled(170,170,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    flyer->setAlignment(Qt::AlignCenter);
    flyer->setStyleSheet("background:transparent;");
    side->addSpacing(10); side->addWidget(flyer); side->addSpacing(15);

    QString buttonStyle =
        "QPushButton {"
        "background:transparent;"
        "color:#8B9BB4;"
        "border:none;"
        "padding:13px;"
        "text-align:left;"
        "font-size:14px;"
        "font-weight:400;"
        "}"
        "QPushButton:hover {"
        "color:#25C7E8;"
        "background:#16485A;"
        "border-radius:7px;"
        "}";

    QStringList menus =
        {
            "▣   Tableau de bord",
            "♣   Participants",
            "♟   Equipes",
            "⌘   Projets",
            "★   Evaluations",
            "⚙   Paramètres"
        };

    for (const QString &menu : menus)
    {
        QPushButton *button = new QPushButton(menu);
        button->setStyleSheet(buttonStyle);
        side->addWidget(button);
    }

    side->addStretch();

    QLabel *bottom = new QLabel(
        "Hack today\n"
        "Build tomorrow"
        );
    bottom->setAlignment(Qt::AlignCenter);
    bottom->setStyleSheet("color:#52627A;");
    side->addWidget(bottom);

    mainLayout->addWidget(sidebar);

    // ========================================================
    // CONTENU
    // ========================================================

    QWidget *content = new QWidget;
    content->setStyleSheet(
        "background:#0D131F;"
        "color:white;"
        );

    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(27, 25, 27, 20);

    // HEADER : titre à gauche, actions à droite (comme la photo)
    QHBoxLayout *header = new QHBoxLayout;
    header->setSpacing(10);

    QVBoxLayout *titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(2);
    QLabel *title = new QLabel("Projets");
    title->setStyleSheet("font-size:25px; font-weight:400; color:#F1F5F9;");
    QLabel *subtitle = new QLabel("Gérez les projets du hackathon");
    subtitle->setStyleSheet("color:#8B9BB4; font-size:13px; font-weight:400;");
    titleBlock->addWidget(title);
    titleBlock->addWidget(subtitle);

    QPushButton *modeClair = new QPushButton("☼  Mode clair");
    QPushButton *exporter = new QPushButton("Exporter en PDF");
    QPushButton *ajouter = new QPushButton("+ Ajouter un projet");

    QString topButtonStyle =
        "QPushButton { background:#111927; color:#F1F5F9; border:1px solid #24344D; "
        "padding:9px 13px; border-radius:7px; font-size:13px; }"
        "QPushButton:hover { background:#1A3448; color:#25C7E8; }";

    modeClair->setStyleSheet(topButtonStyle);
    exporter->setStyleSheet(topButtonStyle);
    ajouter->setStyleSheet(
        "QPushButton { background:#25C7E8; color:#001018; border:none; "
        "padding:9px 14px; border-radius:7px; font-size:13px; font-weight:500; }"
        "QPushButton:hover { background:#35D5F3; }");

    header->addLayout(titleBlock);
    header->addStretch();
    header->addWidget(modeClair);
    header->addWidget(exporter);
    header->addWidget(ajouter);
    layout->addLayout(header);

    // RECHERCHE + ANNULER sur la même ligne
    QHBoxLayout *searchRow = new QHBoxLayout;
    recherche = new QLineEdit;
    recherche->setPlaceholderText("Rechercher un projet");
    recherche->setStyleSheet(
        "QLineEdit { background:#111927; border:1px solid #24344D; border-radius:7px; "
        "padding:10px 12px; color:#F1F5F9; font-size:13px; }"
        "QLineEdit:focus { border:1px solid #25C7E8; }");

    QPushButton *undoButton = new QPushButton("↶  Annuler (Ctrl+Z)");
    undoButton->setStyleSheet(
        "QPushButton { background:transparent; color:#8B9BB4; border:none; "
        "padding:8px 4px; font-size:13px; }"
        "QPushButton:hover { color:#25C7E8; }");

    searchRow->addWidget(recherche, 1);
    searchRow->addSpacing(14);
    searchRow->addWidget(undoButton);
    layout->addLayout(searchRow);

    connect(ajouter, &QPushButton::clicked, this, &MainWindow::ajouterProjet);

    // Exporter tous les projets en PDF
    connect(exporter, &QPushButton::clicked, this, [this]()
            {
                if (projets.isEmpty())
                {
                    QMessageBox::information(
                        this,
                        "Exporter en PDF",
                        "Aucun projet à exporter."
                        );
                    return;
                }

                QString fileName = QFileDialog::getSaveFileName(
                    this,
                    "Exporter les projets en PDF",
                    "projets_hackathon.pdf",
                    "Fichiers PDF (*.pdf)"
                    );

                if (fileName.isEmpty())
                    return;

                if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
                    fileName += ".pdf";

                int meilleurIndex = 0;
                double sommeNotes = 0.0;
                QSet<QString> thematiques;

                for (int i = 0; i < projets.size(); ++i)
                {
                    sommeNotes += projets[i].noteFinale;
                    thematiques.insert(projets[i].thematique);

                    if (projets[i].noteFinale > projets[meilleurIndex].noteFinale)
                        meilleurIndex = i;
                }

                const Projet &gagnantProjet = projets[meilleurIndex];
                double moyenne = sommeNotes / projets.size();

                QString html;
                html += "<html><head><meta charset='utf-8'></head><body>";
                html += "<h1 style='color:#123; font-size:24pt;'>Projets du Hackathon</h1>";
                html += "<p style='color:#666;'>Rapport des projets</p>";

                html += "<h2>Statistiques</h2>";
                html += "<p><b>Nombre de projets :</b> " + QString::number(projets.size()) + "<br>";
                html += "<b>Note moyenne :</b> " + QString::number(moyenne, 'f', 1) + "/20<br>";
                html += "<b>Thématiques :</b> " + QString::number(thematiques.size()) + "<br>";
                html += "<b>Gagnant :</b> " + gagnantProjet.titre.toHtmlEscaped() +
                        " (" + QString::number(gagnantProjet.noteFinale, 'f', 1) + "/20)</p>";

                html += "<h2>Liste des projets</h2>";
                html += "<table border='1' cellspacing='0' cellpadding='6' width='100%'>";
                html += "<tr bgcolor='#e8f4f8'>"
                        "<th>ID_Projet</th><th>Titre</th><th>Description</th>"
                        "<th>Thématique</th><th>Note Finale</th><th>ID_Equipe</th></tr>";

                for (const Projet &p : projets)
                {
                    html += "<tr>";
                    html += "<td>" + p.ID_Projet.toHtmlEscaped() + "</td>";
                    html += "<td>" + p.titre.toHtmlEscaped() + "</td>";
                    html += "<td>" + p.description.toHtmlEscaped() + "</td>";
                    html += "<td>" + p.thematique.toHtmlEscaped() + "</td>";
                    html += "<td align='center'>" + QString::number(p.noteFinale, 'f', 1) + "/20</td>";
                    html += "<td>" + p.ID_Equipe.toHtmlEscaped() + "</td>";
                    html += "</tr>";
                }

                html += "</table>";
                html += "<p style='color:#777; margin-top:20px;'>"
                        "Le gagnant est déterminé automatiquement selon la Note Finale la plus élevée."
                        "</p>";
                html += "</body></html>";

                QPdfWriter pdf(fileName);
                pdf.setPageSize(QPageSize(QPageSize::A4));
                pdf.setTitle("Projets du Hackathon");
                pdf.setCreator("Devena");

                QTextDocument document;
                document.setHtml(html);
                document.setPageSize(QSizeF(pdf.width(), pdf.height()));
                document.print(&pdf);

                QMessageBox::information(
                    this,
                    "Export PDF",
                    "Le PDF a été créé avec succès :\n" + fileName
                    );
            });

    connect(undoButton, &QPushButton::clicked, this, [this]()
            {
                QMessageBox::information(
                    this,
                    "Annuler",
                    "Le bouton Ctrl+Z sera relié à l'historique des modifications."
                    );
            });

    QLabel *tableTitle = new QLabel(
        "Tableau des projets"
        );
    tableTitle->setStyleSheet(
        "font-size:16px;"
        "font-weight:500;"
        );
    layout->addWidget(tableTitle);

    // ========================================================
    // LISTE DES PROJETS — même style que l'interface originale
    // ========================================================

    // En-têtes : on affiche simplement les nouveaux champs du projet
    // sans changer les boutons d'action de chaque ligne.
    QWidget *headerWidget = new QWidget;
    headerWidget->setStyleSheet(
        "QWidget { background:#111927; border:none; border-bottom:1px solid #24344D; }"
        );
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 10, 14, 10);
    headerLayout->setSpacing(12);

    const QStringList headers = {
        "ID Projet", "Titre", "Description", "Thématique",
        "Note Finale", "ID Équipe"
    };
    const QList<int> headerStretch = {2, 3, 5, 2, 2, 2};

    for (int h = 0; h < headers.size(); ++h)
    {
        QLabel *label = new QLabel(headers[h]);
        label->setStyleSheet(
            "color:#8B9BB4; font-size:11px; font-weight:600; padding:2px;"
            );
        headerLayout->addWidget(label, headerStretch[h]);
    }

    QLabel *actionsHeader = new QLabel("Actions");
    actionsHeader->setStyleSheet(
        "color:#8B9BB4; font-size:11px; font-weight:600; padding:2px;"
        );
    headerLayout->addWidget(actionsHeader, 4);
    layout->addWidget(headerWidget);

    listeProjets = new QListWidget;
    listeProjets->setSelectionMode(QAbstractItemView::SingleSelection);
    listeProjets->setSpacing(2);
    listeProjets->setStyleSheet(
        "QListWidget { background:transparent; border:none; outline:none; }"
        "QListWidget::item { background:transparent; border:none; }"
        "QListWidget::item:selected { background:transparent; }"
        "QScrollBar:vertical { background:#111927; width:10px; margin:0; }"
        "QScrollBar::handle:vertical { background:#24344D; border-radius:5px; min-height:25px; }"
        );
    layout->addWidget(listeProjets, 1);

    // ========================================================
    // STATISTIQUES
    // ========================================================

    QLabel *statsTitle = new QLabel("Statistiques");
    statsTitle->setStyleSheet(
        "font-size:16px;"
        "font-weight:500;"
        );
    layout->addWidget(statsTitle);

    g_statsChart = new StatsChart;
    layout->addWidget(g_statsChart);

    QHBoxLayout *cards = new QHBoxLayout;

    statProjets = new QLabel("<span style='color:#8B9BB4;font-size:12px;'>Projets</span><br><span style='font-size:23px;color:#F1F5F9;'>0</span>");
    statNombre = new QLabel("<span style='color:#8B9BB4;font-size:12px;'>Total nombre</span><br><span style='font-size:23px;color:#F1F5F9;'>0</span>");
    statDomaines = new QLabel("<span style='color:#8B9BB4;font-size:12px;'>Domaines</span><br><span style='font-size:23px;color:#F1F5F9;'>0</span>");
    statGagnant = new QLabel("<span style='color:#8B9BB4;font-size:12px;'>Gagnant</span><br><span style='font-size:18px;color:#F1F5F9;'>Aucun</span>");

    QList<QLabel*> labels =
        {
            statProjets,
            statNombre,
            statDomaines,
            statGagnant
        };

    for (QLabel *label : labels)
    {
        label->setMinimumHeight(70);
        label->setStyleSheet(
            "background:#152032;"
            "border:1px solid #24344D;"
            "border-radius:9px;"
            "padding:12px;"
            "color:#F1F5F9;"
            "font-size:14px;"
            "font-weight:normal;"
            );
        label->setAlignment(
            Qt::AlignLeft | Qt::AlignVCenter
            );
        cards->addWidget(label);
    }

    layout->addLayout(cards);
    mainLayout->addWidget(content);

    // ========================================================
    // CONNECTIONS DU HAUT
    // ========================================================

    connect(
        ajouter,
        &QPushButton::clicked,
        this,
        &MainWindow::ajouterProjet
        );

    connect(
        recherche,
        &QLineEdit::textChanged,
        this,
        &MainWindow::afficherListe
        );

    connect(
        listeProjets,
        &QListWidget::itemDoubleClicked,
        this,
        [this](QListWidgetItem *item)
        {
            if (!item)
                return;

            int index = item->data(Qt::UserRole).toInt();
            if (index >= 0 && index < projets.size())
                afficherProjet(index);
        }
        );
}

// ============================================================
// AFFICHER LISTE
// ============================================================

void MainWindow::afficherListe()
{
    listeProjets->clear();

    QString rechercheTexte = recherche->text().trimmed().toLower();

    for (int i = 0; i < projets.size(); ++i)
    {
        const Projet &p = projets[i];

        if (!rechercheTexte.isEmpty())
        {
            bool trouve =
                p.ID_Projet.toLower().contains(rechercheTexte) ||
                p.titre.toLower().contains(rechercheTexte) ||
                p.description.toLower().contains(rechercheTexte) ||
                p.thematique.toLower().contains(rechercheTexte) ||
                p.ID_Equipe.toLower().contains(rechercheTexte);

            if (!trouve)
                continue;
        }

        QListWidgetItem *item = new QListWidgetItem;
        item->setData(Qt::UserRole, i);

        QWidget *rowWidget = new QWidget;
        rowWidget->setStyleSheet(
            "QWidget { background:#111927; border:none; border-bottom:1px solid #24344D; }"
            "QWidget:hover { background:#152032; }"
            );

        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(16, 11, 14, 11);
        rowLayout->setSpacing(12);

        const QList<int> stretch = {2, 3, 5, 2, 2, 2};
        const QStringList values = {
            p.ID_Projet,
            p.titre,
            p.description,
            p.thematique,
            QString::number(p.noteFinale, 'f', 1) + " / 20",
            p.ID_Equipe
        };

        for (int c = 0; c < values.size(); ++c)
        {
            QLabel *label = new QLabel(values[c]);
            label->setWordWrap(c == 2);
            label->setToolTip(values[c]);
            label->setStyleSheet(
                "color:#F1F5F9; font-size:13px; font-weight:400; "
                "padding:3px 2px; background:transparent; border:none;"
                );

            if (c == 0)
            {
                label->setStyleSheet(
                    "color:#25C7E8; font-size:13px; font-weight:600; "
                    "padding:3px 2px; background:transparent; border:none;"
                    );
                label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            }
            else if (c == 3)
            {
                label->setStyleSheet(
                    "color:#25C7E8; font-size:12px; font-weight:500; "
                    "padding:5px 9px; background:#16485A; border-radius:10px;"
                    );
                label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            }
            else if (c == 4)
            {
                label->setStyleSheet(
                    "color:#F1F5F9; font-size:13px; font-weight:600; "
                    "padding:3px 2px; background:transparent; border:none;"
                    );
                label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            }
            else if (c == 5)
            {
                label->setStyleSheet(
                    "color:#8B9BB4; font-size:12px; font-weight:500; "
                    "padding:3px 2px; background:transparent; border:none;"
                    );
                label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            }
            else
            {
                label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            }

            rowLayout->addWidget(label, stretch[c]);
        }

        // Les mêmes symboles d'action que dans l'ancienne interface.
        QWidget *actionsWidget = new QWidget;
        actionsWidget->setStyleSheet("background:transparent; border:none;");
        QHBoxLayout *actions = new QHBoxLayout(actionsWidget);
        actions->setContentsMargins(0, 0, 0, 0);
        actions->setSpacing(3);

        QPushButton *btnAfficher = new QPushButton("👁");
        QPushButton *btnModifier = new QPushButton("✎");
        QPushButton *btnSupprimer = new QPushButton("🗑");
        QPushButton *btnGagnant = new QPushButton("⭐");

        btnAfficher->setToolTip("Afficher");
        btnModifier->setToolTip("Modifier");
        btnSupprimer->setToolTip("Supprimer");
        btnGagnant->setToolTip("Voir le gagnant (note la plus élevée)");

        QString actionStyle =
            "QPushButton { background:transparent; color:#8B9BB4; border:none; "
            "border-radius:5px; padding:5px 7px; font-size:16px; }"
            "QPushButton:hover { background:#16485A; color:#25C7E8; }";

        btnAfficher->setStyleSheet(actionStyle);
        btnModifier->setStyleSheet(actionStyle);

        btnSupprimer->setStyleSheet(
            "QPushButton { background:transparent; color:#ff7777; border:none; "
            "border-radius:5px; padding:5px 7px; font-size:16px; }"
            "QPushButton:hover { background:#4a1d24; color:#ff4444; }"
            );

        btnGagnant->setStyleSheet(
            "QPushButton { background:transparent; color:#ffd43b; border:none; "
            "border-radius:5px; padding:5px 7px; font-size:16px; }"
            "QPushButton:hover { background:#3c3618; }"
            );

        actions->addWidget(btnAfficher);
        actions->addWidget(btnModifier);
        actions->addWidget(btnSupprimer);
        actions->addWidget(btnGagnant);
        rowLayout->addWidget(actionsWidget, 4);

        listeProjets->addItem(item);
        item->setSizeHint(rowWidget->sizeHint());
        listeProjets->setItemWidget(item, rowWidget);

        // Met en évidence uniquement le gagnant automatique.
        if (!g_gagnant.isEmpty() && p.titre == g_gagnant)
        {
            rowWidget->setStyleSheet(
                "QWidget { background:#152C3A; border:none; border-left:2px solid #25C7E8; }"
                );
        }

        connect(btnAfficher, &QPushButton::clicked, this, [this, item]()
                {
                    afficherProjet(item->data(Qt::UserRole).toInt());
                });

        connect(btnModifier, &QPushButton::clicked, this, [this, item]()
                {
                    modifierProjet(item->data(Qt::UserRole).toInt());
                });

        connect(btnSupprimer, &QPushButton::clicked, this, [this, item]()
                {
                    supprimerProjet(item->data(Qt::UserRole).toInt());
                });

        connect(btnGagnant, &QPushButton::clicked, this, [this]()
                {
                    // Le projet ayant la meilleure Note_Finale est automatiquement gagnant.
                    definirGagnant(-1);
                });
    }

    afficherStatistiques();
}

// ============================================================
// AJOUTER PROJET
// ============================================================

void MainWindow::ajouterProjet()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter un projet");
    dialog.resize(520, 460);
    QFormLayout form(&dialog);

    QLineEdit *idProjet = new QLineEdit;
    QLineEdit *titre = new QLineEdit;
    QTextEdit *description = new QTextEdit;
    QComboBox *thematique = new QComboBox;
    QDoubleSpinBox *note = new QDoubleSpinBox;
    QLineEdit *idEquipe = new QLineEdit;

    thematique->addItems({"IA", "Web", "IoT"});
    note->setRange(0.0, 20.0); note->setDecimals(1); note->setSingleStep(0.5);

    form.addRow("ID_Projet :", idProjet);
    form.addRow("Titre :", titre);
    form.addRow("Description :", description);
    form.addRow("Thématique :", thematique);
    form.addRow("Note Finale :", note);
    form.addRow("ID_Equipe :", idEquipe);

    QPushButton *ok = new QPushButton("Ajouter");
    QPushButton *cancel = new QPushButton("Annuler");
    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addStretch(); buttons->addWidget(ok); buttons->addWidget(cancel);
    form.addRow("", buttons);

    dialog.setStyleSheet("QDialog { background:#111927; color:white; font-family:'Segoe UI'; }"
                         "QLabel { color:#F1F5F9; }"
                         "QLineEdit,QDoubleSpinBox,QComboBox,QTextEdit { background:#0D131F; color:white; border:1px solid #24344D; border-radius:6px; padding:8px; }"
                         "QLineEdit:focus,QDoubleSpinBox:focus,QComboBox:focus,QTextEdit:focus { border:1px solid #25C7E8; }"
                         "QPushButton { background:#25C7E8; color:#001018; padding:9px 18px; border-radius:6px; font-weight:bold; }"
                         "QPushButton:hover { background:#35D5F3; }");

    connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject);
    connect(ok,&QPushButton::clicked,&dialog,[&](){
        if(idProjet->text().trimmed().isEmpty() || titre->text().trimmed().isEmpty() || idEquipe->text().trimmed().isEmpty()){
            QMessageBox::warning(&dialog,"Erreur","ID_Projet, Titre et ID_Equipe sont obligatoires."); return; }
        for(const Projet &p: projets) if(p.ID_Projet.compare(idProjet->text().trimmed(),Qt::CaseInsensitive)==0){
                QMessageBox::warning(&dialog,"Erreur","Cet ID_Projet existe déjà."); return; }
        dialog.accept();
    });
    if(dialog.exec()!=QDialog::Accepted) return;

    Projet p;
    p.ID_Projet=idProjet->text().trimmed(); p.titre=titre->text().trimmed();
    p.description=description->toPlainText().trimmed(); p.thematique=thematique->currentText();
    p.noteFinale=note->value(); p.ID_Equipe=idEquipe->text().trimmed();
    projets.append(p); afficherListe();
}

// ============================================================
// AFFICHER PROJET
// ============================================================

void MainWindow::afficherProjet(int index)
{
    if(index<0 || index>=projets.size()) return;
    const Projet &p=projets[index];
    QMessageBox::information(this,"Informations du projet",
                             "ID_PROJET : "+p.ID_Projet+"\n\nTITRE : "+p.titre+
                                 "\n\nDESCRIPTION :\n"+p.description+"\n\nTHÉMATIQUE : "+p.thematique+
                                 "\n\nNOTE FINALE : "+QString::number(p.noteFinale,'f',1)+"\n\nID_EQUIPE : "+p.ID_Equipe);
}

// ============================================================
// MODIFIER PROJET
// ============================================================

void MainWindow::modifierProjet(int index)
{
    if(index<0 || index>=projets.size()) return;
    Projet &p=projets[index];
    QDialog dialog(this); dialog.setWindowTitle("Modifier le projet"); dialog.resize(520,460);
    QFormLayout form(&dialog);
    QLineEdit *idProjet=new QLineEdit(p.ID_Projet); QLineEdit *titre=new QLineEdit(p.titre);
    QTextEdit *description=new QTextEdit; QComboBox *thematique=new QComboBox;
    QDoubleSpinBox *note=new QDoubleSpinBox; QLineEdit *idEquipe=new QLineEdit(p.ID_Equipe);
    description->setPlainText(p.description); thematique->addItems({"IA","Web","IoT"});
    int ti=thematique->findText(p.thematique); if(ti>=0) thematique->setCurrentIndex(ti);
    note->setRange(0.0,20.0); note->setDecimals(1); note->setSingleStep(0.5); note->setValue(p.noteFinale);
    form.addRow("ID_Projet :",idProjet); form.addRow("Titre :",titre); form.addRow("Description :",description);
    form.addRow("Thématique :",thematique); form.addRow("Note Finale :",note); form.addRow("ID_Equipe :",idEquipe);
    QPushButton *save=new QPushButton("Enregistrer"); QPushButton *cancel=new QPushButton("Annuler");
    QHBoxLayout *buttons=new QHBoxLayout; buttons->addStretch(); buttons->addWidget(save); buttons->addWidget(cancel); form.addRow("",buttons);
    dialog.setStyleSheet("QDialog { background:#111927; color:white; font-family:'Segoe UI'; }"
                         "QLabel { color:#F1F5F9; }"
                         "QLineEdit,QDoubleSpinBox,QComboBox,QTextEdit { background:#0D131F; color:white; border:1px solid #24344D; border-radius:6px; padding:8px; }"
                         "QLineEdit:focus,QDoubleSpinBox:focus,QComboBox:focus,QTextEdit:focus { border:1px solid #25C7E8; }"
                         "QPushButton { background:#25C7E8; color:#001018; padding:9px 18px; border-radius:6px; font-weight:bold; }"
                         "QPushButton:hover { background:#35D5F3; }");
    connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject);
    connect(save,&QPushButton::clicked,&dialog,[&](){
        if(idProjet->text().trimmed().isEmpty() || titre->text().trimmed().isEmpty() || idEquipe->text().trimmed().isEmpty()){ QMessageBox::warning(&dialog,"Erreur","ID_Projet, Titre et ID_Equipe sont obligatoires."); return; }
        for(int i=0;i<projets.size();++i) if(i!=index && projets[i].ID_Projet.compare(idProjet->text().trimmed(),Qt::CaseInsensitive)==0){ QMessageBox::warning(&dialog,"Erreur","Cet ID_Projet existe déjà."); return; }
        dialog.accept();
    });
    if(dialog.exec()!=QDialog::Accepted) return;
    QString ancienTitre=p.titre;
    p.ID_Projet=idProjet->text().trimmed(); p.titre=titre->text().trimmed(); p.description=description->toPlainText().trimmed();
    p.thematique=thematique->currentText(); p.noteFinale=note->value(); p.ID_Equipe=idEquipe->text().trimmed();
    if(g_gagnant==ancienTitre) g_gagnant=p.titre;
    afficherListe();
}

// ============================================================
// SUPPRIMER PROJET
// ============================================================

void MainWindow::supprimerProjet(int index)
{
    if (index < 0 || index >= projets.size())
        return;

    const QString nom = projets[index].titre;

    QMessageBox::StandardButton reponse =
        QMessageBox::question(
            this,
            "Supprimer",
            "Voulez-vous supprimer le projet :\n\n" + nom + " ?",
            QMessageBox::Yes | QMessageBox::No
            );

    if (reponse != QMessageBox::Yes)
        return;

    if (g_gagnant == nom)
        g_gagnant.clear();

    projets.removeAt(index);
    afficherListe();
}

// ============================================================
// DECLARER GAGNANT
// ============================================================

void MainWindow::definirGagnant(int /*index*/)
{
    if (projets.isEmpty())
    {
        g_gagnant.clear();
        afficherStatistiques();
        afficherListe();
        return;
    }

    // Le gagnant est automatiquement le projet ayant la Note_Finale la plus élevée.
    int meilleurIndex = 0;

    for (int i = 1; i < projets.size(); ++i)
    {
        if (projets[i].noteFinale > projets[meilleurIndex].noteFinale)
            meilleurIndex = i;
    }

    g_gagnant = projets[meilleurIndex].titre;

    afficherStatistiques();
    afficherListe();
    afficherCelebration(g_gagnant);
}

// ============================================================
// STATISTIQUES
// ============================================================

void MainWindow::afficherStatistiques()
{
    if(!statProjets || !statNombre || !statDomaines || !statGagnant) return;
    int total=projets.size(); double sommeNotes=0.0; QSet<QString> thematiques;
    for(const Projet &p: projets){ sommeNotes+=p.noteFinale; thematiques.insert(p.thematique); }
    double moyenne=total>0?sommeNotes/total:0.0;

    // Gagnant automatique : la meilleure Note_Finale.
    if (!projets.isEmpty())
    {
        int meilleurIndex = 0;

        for (int i = 1; i < projets.size(); ++i)
        {
            if (projets[i].noteFinale > projets[meilleurIndex].noteFinale)
                meilleurIndex = i;
        }

        g_gagnant = projets[meilleurIndex].titre;
    }
    else
    {
        g_gagnant.clear();
    }
    statProjets->setText("<span style='color:#8B9BB4;font-size:12px;'>Projets</span><br><span style='font-size:23px;color:#F1F5F9;'>"+QString::number(total)+"</span>");
    statNombre->setText("<span style='color:#8B9BB4;font-size:12px;'>Note moyenne</span><br><span style='font-size:23px;color:#F1F5F9;'>"+QString::number(moyenne,'f',1)+"/20</span>");
    statDomaines->setText("<span style='color:#8B9BB4;font-size:12px;'>Thématiques</span><br><span style='font-size:23px;color:#F1F5F9;'>"+QString::number(thematiques.size())+"</span>");
    statGagnant->setText(g_gagnant.isEmpty()?"<span style='color:#8B9BB4;font-size:12px;'>Gagnant</span><br><span style='font-size:18px;color:#F1F5F9;'>Aucun</span>":"<span style='color:#8B9BB4;font-size:12px;'>🏆 Gagnant</span><br><span style='font-size:18px;color:#F1F5F9;'>"+g_gagnant+"</span>");
    if (!g_statsChart)
        return;

    g_statsChart->valeurs.clear();
    g_statsChart->noms.clear();
    for(const Projet &p: projets){ g_statsChart->valeurs.append(qRound(p.noteFinale*10.0)); g_statsChart->noms.append(p.titre); }
    g_statsChart->update();
}

// ============================================================
// ANIMATION DE VICTOIRE
// ============================================================

void MainWindow::afficherCelebration(const QString &nomProjet)
{
    QWidget *overlay = new QWidget(this);

    overlay->setAttribute(Qt::WA_DeleteOnClose);
    overlay->setStyleSheet(
        "background-color:rgba(2,9,20,242);"
        );
    overlay->setGeometry(rect());

    QVBoxLayout *layout = new QVBoxLayout(overlay);
    layout->setAlignment(Qt::AlignCenter);

    QLabel *emoji = new QLabel("🎉   🏆   🎉");
    emoji->setAlignment(Qt::AlignCenter);
    emoji->setStyleSheet("font-size:55px;");
    layout->addWidget(emoji);

    QLabel *felicitation = new QLabel("FÉLICITATIONS !");
    felicitation->setAlignment(Qt::AlignCenter);
    felicitation->setStyleSheet(
        "color:#25C7E8;"
        "font-size:42px;"
        "font-weight:bold;"
        );
    layout->addWidget(felicitation);

    QLabel *winner = new QLabel(
        "🏆 PROJET GAGNANT 🏆\n\n" + nomProjet
        );
    winner->setAlignment(Qt::AlignCenter);
    winner->setStyleSheet(
        "color:white;"
        "font-size:30px;"
        "font-weight:bold;"
        "padding:25px;"
        );
    layout->addWidget(winner);

    QLabel *message = new QLabel(
        "Bravo à toute l'équipe ! 🎊"
        );
    message->setAlignment(Qt::AlignCenter);
    message->setStyleSheet(
        "color:#8B9BB4;"
        "font-size:20px;"
        );
    layout->addWidget(message);

    QPushButton *close = new QPushButton("Continuer");
    close->setFixedSize(160, 45);
    close->setStyleSheet(
        "QPushButton {"
        "background:#25C7E8;"
        "color:#001018;"
        "font-weight:bold;"
        "padding:10px;"
        "border-radius:8px;"
        "font-size:15px;"
        "}"
        "QPushButton:hover {"
        "background:#25C7E8;"
        "}"
        );

    layout->addWidget(
        close,
        0,
        Qt::AlignCenter
        );

    ConfettiWidget *confettis =
        new ConfettiWidget(overlay);

    confettis->setGeometry(overlay->rect());
    confettis->lower();

    QGraphicsOpacityEffect *effect =
        new QGraphicsOpacityEffect(overlay);

    overlay->setGraphicsEffect(effect);

    QPropertyAnimation *animation =
        new QPropertyAnimation(
            effect,
            "opacity",
            overlay
            );

    animation->setDuration(700);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);

    overlay->show();
    overlay->raise();

    confettis->raise();
    confettis->start();

    animation->start(
        QAbstractAnimation::DeleteWhenStopped
        );

    connect(
        close,
        &QPushButton::clicked,
        overlay,
        [overlay, confettis]()
        {
            confettis->stop();
            overlay->close();
        }
        );

    QTimer::singleShot(
        8000,
        overlay,
        [overlay, confettis]()
        {
            if (overlay->isVisible())
            {
                confettis->stop();
                overlay->close();
            }
        }
        );
}
