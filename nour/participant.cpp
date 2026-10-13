#include "participant.h"

#include <QAction>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#ifdef HAS_QTPDF
#include <QPdfDocument>
#endif
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPrinter>
#include <QPushButton>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QTableView>
#include <QTextDocument>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QtCharts>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

        // ================================================================ Données
        QString Participant::pole() const {
    auto has = [this](const QStringList& keys) {
        for (const QString& comp : competences)
            for (const QString& k : keys)
                if (comp.compare(k, Qt::CaseInsensitive) == 0) return true;
        return false;
    };
    if (has({"Figma", "UI", "UX", "UI/UX", "Design", "Design system", "Prototypage", "Scrum", "Gestion"}))
        return "Designers & Leads";
    if (has({"IA", "LLM", "Pandas", "Data", "TensorFlow", "Machine Learning"}))
        return "IA & Data Specialists";
    return "Developers";
}

QString makeStyle(bool dark) {
    const QString bg   = dark ? "#0d1420" : "#ffffff";
    const QString card = dark ? "#152033" : "#f5f7fa";
    const QString tx   = dark ? "#e6edf6" : "#1b2430";
    const QString tx2  = dark ? "#8fa0b6" : "#5b6675";
    const QString bd   = dark ? "#243350" : "#dde3ea";
    const QString ac   = dark ? "#22d3ee" : "#1f6feb";
    const QString onAc = dark ? "#04232b" : "#ffffff";
    const QString acBg = dark ? "#0e3a4a" : "#e3eefe";

    return QString(R"(
QMainWindow, QDialog, QWidget#root { background:%1; }
QWidget { color:%3; font-size:13px; }
QFrame#kpi { background:%2; border-radius:8px; }
QLabel#title { font-size:22px; font-weight:500; }
QLabel#sub, QLabel#kpiTitle { color:%4; }
QLabel#kpiVal { font-size:22px; font-weight:500; }
QLineEdit, QComboBox { background:%2; border:1px solid %5; border-radius:8px; padding:6px 10px; }
QPushButton { background:transparent; border:1px solid %5; border-radius:8px; padding:7px 14px; }
QPushButton:hover { background:%2; }
QPushButton#primary { background:%6; border-color:%6; color:%7; font-weight:500; }
QTableView { background:%1; border:1px solid %5; border-radius:8px; gridline-color:transparent;
             selection-background-color:%2; selection-color:%3; }
QHeaderView::section { background:%1; color:%6; border:none; border-bottom:1px solid %5;
                       padding:8px; font-weight:500; }
QLabel#avatar { background:%8; color:%6; border-radius:40px; font-size:26px; font-weight:500; }
QLabel#chip { background:%8; color:%6; border-radius:10px; padding:2px 10px; }
QLabel#section { font-size:16px; font-weight:500; }
QListWidget { background:%2; border:1px solid %5; border-radius:8px; }
QLabel#logo { font-size:20px; font-weight:500; color:%6; }
QFrame#sidebar { background:%2; border-right:1px solid %5; }
QPushButton#nav { border:1px solid transparent; text-align:left; padding:10px 14px; color:%4; }
QPushButton#nav:hover { background:%1; }
QPushButton#nav[active="true"] { background:%8; color:%6; border:1px solid %6; font-weight:500; }
)").arg(bg, card, tx, tx2, bd, ac, onAc, acBg);
}

// ================================================================ Modèle
ParticipantModel::ParticipantModel(QObject* parent) : QAbstractTableModel(parent) {}

int ParticipantModel::rowCount(const QModelIndex& p) const { return p.isValid() ? 0 : m_data.size(); }
int ParticipantModel::columnCount(const QModelIndex& p) const { return p.isValid() ? 0 : 5; }

QVariant ParticipantModel::data(const QModelIndex& i, int role) const {
    if (!i.isValid()) return {};
    const Participant& p = m_data.at(i.row());
    if (role == IdRole) return p.id;
    if (role != Qt::DisplayRole) return {};
    switch (i.column()) {
    case 0: return p.id;
    case 1: return p.prenom + " " + p.nom;
    case 2: return p.email;
    case 3: return p.competences.join(", ");
    default: return QString();
    }
}

QVariant ParticipantModel::headerData(int s, Qt::Orientation o, int role) const {
    static const QStringList h = {"ID", "Nom & Prénom", "Email", "Compétences", "Actions"};
    if (o == Qt::Horizontal && role == Qt::DisplayRole) return h.value(s);
    return {};
}

int ParticipantModel::rowOf(const QString& id) const {
    for (int i = 0; i < m_data.size(); ++i)
        if (m_data[i].id == id) return i;
    return -1;
}

QString ParticipantModel::nextId() { return QString("P%1").arg(m_next++, 3, 10, QChar('0')); }

void ParticipantModel::insertAt(int row, const Participant& p) {
    beginInsertRows({}, row, row);
    m_data.insert(row, p);
    endInsertRows();
    emit changed();
}

void ParticipantModel::removeAt(int row) {
    if (row < 0 || row >= m_data.size()) return;
    beginRemoveRows({}, row, row);
    m_data.removeAt(row);
    endRemoveRows();
    emit changed();
}

void ParticipantModel::replace(int row, const Participant& p) {
    if (row < 0 || row >= m_data.size()) return;
    m_data[row] = p;
    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
    emit changed();
}

// ================================================================ Commandes d'annulation
AddCmd::AddCmd(ParticipantModel* m, const Participant& p) : m_m(m), m_p(p), m_row(m->rowCount()) {
    setText(QObject::tr("Ajouter %1 %2").arg(p.prenom, p.nom));
}
void AddCmd::redo() { m_m->insertAt(m_row, m_p); }
void AddCmd::undo() { m_m->removeAt(m_m->rowOf(m_p.id)); }

DeleteCmd::DeleteCmd(ParticipantModel* m, const Participant& p)
    : m_m(m), m_p(p), m_row(m->rowOf(p.id)) {
    setText(QObject::tr("Supprimer %1 %2").arg(p.prenom, p.nom));
}
void DeleteCmd::redo() { m_m->removeAt(m_m->rowOf(m_p.id)); }
void DeleteCmd::undo() { m_m->insertAt(m_row, m_p); }   // retour à la même position

EditCmd::EditCmd(ParticipantModel* m, const Participant& oldP, const Participant& newP)
    : m_m(m), m_old(oldP), m_new(newP) {
    setText(QObject::tr("Modifier %1 %2").arg(oldP.prenom, oldP.nom));
}
void EditCmd::redo() { m_m->replace(m_m->rowOf(m_new.id), m_new); }
void EditCmd::undo() { m_m->replace(m_m->rowOf(m_old.id), m_old); }

// ================================================================ Delegates
void CellDelegate::paintBase(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex& i) const {
    QStyleOptionViewItem opt = o;
    initStyleOption(&opt, i);
    opt.text.clear();
    QStyle* s = opt.widget ? opt.widget->style() : QApplication::style();
    s->drawControl(QStyle::CE_ItemViewItem, &opt, p, opt.widget);
}

void BadgeDelegate::paint(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex& i) const {
    paintBase(p, o, i);
    const QStringList tags = i.data().toString().split(", ", Qt::SkipEmptyParts);
    QFontMetrics fm(o.font);
    const int right = o.rect.right() - 4;
    int x = o.rect.left() + 4;
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    for (int k = 0; k < tags.size(); ++k) {
        const int w = fm.horizontalAdvance(tags[k]) + 16;
        if (x + w > right) {                       // plus de place : "+N"
            p->setPen(Theme::accent);
            p->drawText(QRect(x, o.rect.top(), qMax(0, right - x), o.rect.height()),
                        Qt::AlignVCenter, QString("+%1").arg(tags.size() - k));
            break;
        }
        QRect r(x, o.rect.center().y() - 11, w, 22);
        p->setPen(Qt::NoPen);
        p->setBrush(Theme::accentBg);
        p->drawRoundedRect(r, 11, 11);
        p->setPen(Theme::accent);
        p->drawText(r, Qt::AlignCenter, tags[k]);
        x += w + 6;
    }
    p->restore();
}

QSize BadgeDelegate::sizeHint(const QStyleOptionViewItem& o, const QModelIndex& i) const {
    return QStyledItemDelegate::sizeHint(o, i) + QSize(0, 12);
}

QRect ActionsDelegate::btn(const QRect& cell, int k) {
    return QRect(cell.left() + 6 + k * 34, cell.center().y() - 13, 28, 26);
}

void ActionsDelegate::paint(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex& i) const {
    paintBase(p, o, i);
    static const char* glyph[3] = {"👁", "✏", "🗑"};
    p->save();
    for (int k = 0; k < 3; ++k)
        p->drawText(btn(o.rect, k), Qt::AlignCenter, QString::fromUtf8(glyph[k]));
    p->restore();
}

bool ActionsDelegate::editorEvent(QEvent* e, QAbstractItemModel*, const QStyleOptionViewItem& o,
                                  const QModelIndex& i) {
    if (e->type() != QEvent::MouseButtonRelease) return false;
    const QPoint pos = static_cast<QMouseEvent*>(e)->pos();
    const QString id = i.data(ParticipantModel::IdRole).toString();
    if (btn(o.rect, 0).contains(pos)) emit viewClicked(id);
    else if (btn(o.rect, 1).contains(pos)) emit editClicked(id);
    else if (btn(o.rect, 2).contains(pos)) emit deleteClicked(id);
    return true;
}

// ================================================================ Dialogues
ProfileDialog::ProfileDialog(const Participant& p, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Profil du participant");
    setMinimumWidth(340);
    auto* v = new QVBoxLayout(this);
    v->setSpacing(8);

    auto* av = new QLabel((p.prenom.left(1) + p.nom.left(1)).toUpper());
    av->setObjectName("avatar");
    av->setFixedSize(80, 80);
    av->setAlignment(Qt::AlignCenter);
    v->addWidget(av, 0, Qt::AlignHCenter);

    auto* name = new QLabel(p.prenom + " " + p.nom);
    name->setObjectName("title");
    v->addWidget(name, 0, Qt::AlignHCenter);

    auto* id = new QLabel(p.id);
    id->setObjectName("sub");
    v->addWidget(id, 0, Qt::AlignHCenter);

    auto* lbl = new QLabel("Compétences clés");
    lbl->setObjectName("sub");
    v->addSpacing(8);
    v->addWidget(lbl);
    auto* skills = new QHBoxLayout;
    for (const QString& c : p.competences) {
        auto* chip = new QLabel(c);
        chip->setObjectName("chip");
        skills->addWidget(chip);
    }
    skills->addStretch();
    v->addLayout(skills);

    v->addSpacing(8);
    v->addWidget(new QLabel("✉  " + p.email));

    auto* close = new QPushButton("Fermer");
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
    v->addSpacing(8);
    v->addWidget(close);
}

ParticipantFormDialog::ParticipantFormDialog(QWidget* parent, const Participant* p) : QDialog(parent) {
    setWindowTitle(p ? "Modifier le participant" : "Ajouter un participant");
    setMinimumWidth(360);
    m_nom = new QLineEdit;
    m_prenom = new QLineEdit;
    m_email = new QLineEdit;
    m_comp = new QLineEdit;
    m_comp->setPlaceholderText("C++, Qt, Python");
    m_err = new QLabel;
    m_err->setStyleSheet("color:#d03a3a;");
    m_info = new QLabel;
    m_info->setObjectName("sub");
    m_cvBtn = new QPushButton("Importer un CV");

    auto* form = new QFormLayout;
    form->addRow("Nom", m_nom);
    form->addRow("Prénom", m_prenom);
    form->addRow("Email", m_email);
    auto* rowComp = new QHBoxLayout;
    rowComp->addWidget(m_comp, 1);
    rowComp->addWidget(m_cvBtn);
    form->addRow("Compétences", rowComp);

    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, this, [this] { validate(); });
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_cvBtn, &QPushButton::clicked, this, [this] { importerCv(); });

    auto* v = new QVBoxLayout(this);
    v->addLayout(form);
    v->addWidget(m_info);
    v->addWidget(m_err);
    v->addWidget(bb);

    if (p) {
        m_nom->setText(p->nom);
        m_prenom->setText(p->prenom);
        m_email->setText(p->email);
        m_comp->setText(p->competences.join(", "));
    }
}

Participant ParticipantFormDialog::result(const QString& id) const {
    Participant p;
    p.id = id;
    p.nom = m_nom->text().trimmed();
    p.prenom = m_prenom->text().trimmed();
    p.email = m_email->text().trimmed();
    for (const QString& c : m_comp->text().split(',', Qt::SkipEmptyParts))
        p.competences << c.trimmed();
    return p;
}

void ParticipantFormDialog::validate() {
    if (m_nom->text().trimmed().isEmpty() || m_prenom->text().trimmed().isEmpty()) {
        m_err->setText("Renseignez le nom et le prénom.");
        return;
    }
    static const QRegularExpression re(R"(^\S+@\S+\.\S+$)");
    if (!re.match(m_email->text().trimmed()).hasMatch()) {
        m_err->setText("Saisissez une adresse e-mail valide.");
        return;
    }
    accept();
}

// ================================================================ Fenêtre principale
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Devena - Participants");
    resize(1150, 780);

    m_model = new ParticipantModel(this);
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1);
    m_stack = new QUndoStack(this);
    loadSample();

    auto* shell = new QWidget;                       // barre latérale + contenu
    shell->setObjectName("root");
    auto* h = new QHBoxLayout(shell);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    h->addWidget(buildSidebar());
    auto* root = new QWidget;
    h->addWidget(root, 1);
    setCentralWidget(shell);
    auto* v = new QVBoxLayout(root);
    v->setContentsMargins(20, 16, 20, 16);
    v->setSpacing(12);

    // A. En-tête
    auto* head = new QHBoxLayout;
    auto* titles = new QVBoxLayout;
    auto* title = new QLabel("Participants");
    title->setObjectName("title");
    auto* sub = new QLabel("Gérez les participants du hackathon");
    sub->setObjectName("sub");
    titles->addWidget(title);
    titles->addWidget(sub);
    head->addLayout(titles);
    head->addStretch();

    m_themeBtn = new QPushButton("🌙 Mode sombre");
    auto* pdfBtn = new QPushButton("Exporter en PDF");
    auto* addBtn = new QPushButton("+ Ajouter un participant");
    addBtn->setObjectName("primary");
    head->addWidget(m_themeBtn);
    head->addWidget(pdfBtn);
    head->addWidget(addBtn);
    v->addLayout(head);

    // Recherche + annulation
    auto* bar = new QHBoxLayout;
    m_search = new QLineEdit;
    m_search->setPlaceholderText("Rechercher un participant");
    m_search->setClearButtonEnabled(true);
    auto* undoBtn = new QPushButton("↶ Annuler (Ctrl+Z)");
    bar->addWidget(m_search, 1);
    bar->addWidget(undoBtn);
    v->addLayout(bar);

    // B. Tableau
    m_view = new QTableView;
    m_view->setModel(m_proxy);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setShowGrid(false);
    m_view->setSortingEnabled(true);
    m_view->verticalHeader()->hide();
    m_view->verticalHeader()->setDefaultSectionSize(42);
    m_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_view->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    m_view->setColumnWidth(0, 80);
    m_view->setColumnWidth(4, 120);
    m_view->sortByColumn(1, Qt::AscendingOrder);

    auto* badges = new BadgeDelegate(m_view);
    auto* actions = new ActionsDelegate(m_view);
    m_view->setItemDelegateForColumn(3, badges);
    m_view->setItemDelegateForColumn(4, actions);
    m_view->setMinimumHeight(200);
    v->addWidget(m_view, 1);
    v->addWidget(buildStatsChart());     // statistiques

    // D. KPIs
    auto* kpis = new QHBoxLayout;
    m_kTotal = addKpi(kpis, "Total participants");
    m_kIA    = addKpi(kpis, "IA & Data Specialists");
    m_kDev   = addKpi(kpis, "Developers");
    m_kDes   = addKpi(kpis, "Designers & Leads");
    v->addLayout(kpis);

    // Ctrl+Z / Ctrl+Y
    QAction* undoAct = m_stack->createUndoAction(this, "Annuler");
    undoAct->setShortcut(QKeySequence::Undo);
    QAction* redoAct = m_stack->createRedoAction(this, "Rétablir");
    redoAct->setShortcut(QKeySequence::Redo);
    addAction(undoAct);
    addAction(redoAct);

    // Connexions
    connect(undoBtn, &QPushButton::clicked, m_stack, &QUndoStack::undo);
    connect(m_search, &QLineEdit::textChanged, m_proxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(pdfBtn, &QPushButton::clicked, this, &MainWindow::exportPdf);
    connect(m_themeBtn, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    connect(m_model, &ParticipantModel::changed, this, &MainWindow::refreshKpis);

    connect(m_view, &QTableView::clicked, this, [this](const QModelIndex& i) {
        if (i.column() < 2)
            onView(i.data(ParticipantModel::IdRole).toString());
    });
    connect(actions, &ActionsDelegate::viewClicked,   this, &MainWindow::onView,   Qt::QueuedConnection);
    connect(actions, &ActionsDelegate::editClicked,   this, &MainWindow::onEdit,   Qt::QueuedConnection);
    connect(actions, &ActionsDelegate::deleteClicked, this, &MainWindow::onDelete, Qt::QueuedConnection);

    applyTheme(false);
    refreshKpis();
}

QLabel* MainWindow::addKpi(QHBoxLayout* layout, const QString& title) {
    auto* card = new QFrame;
    card->setObjectName("kpi");
    auto* l = new QVBoxLayout(card);
    auto* t = new QLabel(title);
    t->setObjectName("kpiTitle");
    auto* val = new QLabel("0");
    val->setObjectName("kpiVal");
    l->addWidget(t);
    l->addWidget(val);
    layout->addWidget(card);
    return val;
}

void MainWindow::loadSample() {
    struct S { const char *n, *p, *e, *c; };
    const S rows[] = {
                       {"Ben Ali", "Yassine", "yassine.benali@esprit.tn", "C++,Qt,Python"},
                       {"Trabelsi", "Aya", "aya.trabelsi@esprit.tn", "Figma,UI,Prototypage"},
                       {"Sassi", "Omar", "omar.sassi@esprit.tn", "Java,Spring,React"},
                       {"Kacem", "Salma", "salma.kacem@esprit.tn", "Python,Pandas,SQL"},
                       {"Jaziri", "Mohamed", "mohamed.jaziri@esprit.tn", "Java,Docker,SQL"},
                       {"Hammami", "Lina", "lina.hammami@esprit.tn", "Figma,Design system"},
                       {"Mejri", "Zeineb", "zeineb.mejri@esprit.tn", "Scrum,Gestion"},
                       {"Nasri", "Ali", "ali.nasri@esprit.tn", "LLM,Python"},
                       {"Fekih", "Rim", "rim.fekih@esprit.tn", "Python,Django,IA"},
                       {"Daly", "Iheb", "iheb.daly@esprit.tn", "Node.js,Redis"}};
    for (const S& r : rows) {
        Participant p;
        p.id = m_model->nextId();
        p.nom = r.n;
        p.prenom = r.p;
        p.email = r.e;
        p.competences = QString(r.c).split(',');
        m_model->insertAt(m_model->rowCount(), p);
    }
}

void MainWindow::onAdd() {
    ParticipantFormDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
        m_stack->push(new AddCmd(m_model, dlg.result(m_model->nextId())));
}

void MainWindow::onEdit(const QString& id) {
    int r = m_model->rowOf(id);
    if (r < 0) return;
    const Participant old = m_model->at(r);
    ParticipantFormDialog dlg(this, &old);
    if (dlg.exec() == QDialog::Accepted)
        m_stack->push(new EditCmd(m_model, old, dlg.result(old.id)));
}

void MainWindow::onDelete(const QString& id) {
    int r = m_model->rowOf(id);
    if (r < 0) return;
    m_stack->push(new DeleteCmd(m_model, m_model->at(r)));   // annulable avec Ctrl+Z
    statusBar()->showMessage("Participant supprimé. Ctrl+Z pour annuler.", 5000);
}

void MainWindow::onView(const QString& id) {
    int r = m_model->rowOf(id);
    if (r < 0) return;
    ProfileDialog(m_model->at(r), this).exec();
}

void MainWindow::exportPdf() {
    const QString file = QFileDialog::getSaveFileName(this, "Exporter en PDF",
                                                      "participants.pdf", "PDF (*.pdf)");
    if (file.isEmpty()) return;

    QString html = "<h2>Liste des participants</h2>"
                   "<table border='1' cellspacing='0' cellpadding='5' width='100%'>"
                   "<tr><th>ID</th><th>Nom et prénom</th><th>Email</th><th>Compétences</th></tr>";
    for (const Participant& p : m_model->all())
        html += QString("<tr><td>%1</td><td>%2 %3</td><td>%4</td><td>%5</td></tr>")
                    .arg(p.id, p.prenom, p.nom, p.email, p.competences.join(", "));
    html += "</table>";

    QTextDocument doc;
    doc.setHtml(html);
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(file);
    doc.print(&printer);
    statusBar()->showMessage("PDF exporté : " + file, 5000);
}

void MainWindow::toggleTheme() { applyTheme(!m_dark); }

static void styleChart(QChart* c, bool dark) {
    const QColor tx(dark ? "#8fa0b6" : "#5b6675");
    const QColor grid(dark ? "#243350" : "#dde3ea");
    c->setTitleBrush(QBrush(tx));
    for (QAbstractAxis* a : c->axes()) {
        a->setLabelsBrush(QBrush(tx));
        a->setGridLineColor(grid);
        a->setLinePenColor(grid);
    }
    for (QAbstractSeries* s : c->series())
        if (auto* ls = qobject_cast<QLineSeries*>(s)) {
            QPen pen(Theme::accent);
            pen.setWidth(3);
            ls->setPen(pen);
        }
}

void MainWindow::applyTheme(bool dark) {
    m_dark = dark;
    Theme::accent   = QColor(dark ? "#22d3ee" : "#1f6feb");
    Theme::accentBg = QColor(dark ? "#0e3a4a" : "#e3eefe");
    qApp->setStyleSheet(makeStyle(dark));
    if (auto* cv = findChild<QChartView*>()) styleChart(cv->chart(), dark);
    m_themeBtn->setText(dark ? "☀ Mode clair" : "🌙 Mode sombre");
    m_view->viewport()->update();
}

void MainWindow::refreshKpis() {
    int ia = 0, dev = 0, des = 0;
    for (const Participant& p : m_model->all()) {
        const QString pole = p.pole();
        if (pole.startsWith("IA")) ++ia;
        else if (pole == "Developers") ++dev;
        else ++des;
    }
    m_kTotal->setText(QString::number(m_model->rowCount()));
    m_kIA->setText(QString::number(ia));
    m_kDev->setText(QString::number(dev));
    m_kDes->setText(QString::number(des));
}

// ================================================================ Barre latérale
QWidget* MainWindow::buildSidebar() {
    auto* bar = new QFrame;
    bar->setObjectName("sidebar");
    bar->setFixedWidth(210);
    auto* l = new QVBoxLayout(bar);
    l->setContentsMargins(12, 16, 12, 16);
    l->setSpacing(6);

    auto* logo = new QLabel("◆ Devena");
    logo->setObjectName("logo");
    auto* slogan = new QLabel("Connecter les talents,\nconstruire l'avenir");
    slogan->setObjectName("sub");
    l->addWidget(logo);
    l->addWidget(slogan);
    l->addSpacing(16);

    const QStringList items = {"Tableau de bord", "Participants", "Équipes",
                               "Projets", "Évaluations", "Paramètres"};
    for (const QString& it : items) {
        auto* b = new QPushButton(it);
        b->setObjectName("nav");
        b->setCursor(Qt::PointingHandCursor);
        b->setProperty("active", it == "Participants");
        connect(b, &QPushButton::clicked, this, [this, it] {
            statusBar()->showMessage("Module « " + it + " » : bientôt disponible", 3000);
        });
        l->addWidget(b);
    }
    l->addStretch();

    auto* foot = new QLabel("Hack today\nBuild tomorrow");
    foot->setObjectName("sub");
    foot->setAlignment(Qt::AlignCenter);
    l->addWidget(foot);
    return bar;
}

// ================================================================ Statistiques (courbe)
QWidget* MainWindow::buildStatsChart() {
    auto* box = new QWidget;
    auto* l = new QVBoxLayout(box);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(4);

    auto* t = new QLabel("Statistiques");
    t->setObjectName("section");
    l->addWidget(t);

    const QStringList mois = {"Janv", "Févr", "Mars", "Avr", "Mai", "Juin", "Juil", "Août", "Sept"};
    const int valeurs[] = {2, 10, 10, 20, 20, 37, 31, 31, 43};   // exemple : à relier à vos données

    auto* serie = new QLineSeries;
    serie->setPointsVisible(true);
    for (int i = 0; i < mois.size(); ++i) serie->append(i, valeurs[i]);

    auto* chart = new QChart;
    chart->addSeries(serie);
    chart->legend()->hide();
    chart->setTitle("Membres inscrits");
    chart->setBackgroundVisible(false);

    auto* axeX = new QBarCategoryAxis;
    axeX->append(mois);
    chart->addAxis(axeX, Qt::AlignBottom);
    serie->attachAxis(axeX);

    auto* axeY = new QValueAxis;
    axeY->setRange(0, 50);
    axeY->setTickCount(6);
    axeY->setLabelFormat("%d");
    chart->addAxis(axeY, Qt::AlignLeft);
    serie->attachAxis(axeY);

    auto* view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background:transparent; border:none;");
    view->setMinimumHeight(190);
    view->setMaximumHeight(230);
    l->addWidget(view);
    return box;
}

// ================================================================ Import d'un CV
void ParticipantFormDialog::importerCv() {
    const QString f = QFileDialog::getOpenFileName(this, "Choisir un CV", QString(), "CV (*.pdf *.txt)");
    if (f.isEmpty()) return;

    QString erreur;
    const QString texte = CvExtracteur::lireFichier(f, &erreur);
    if (texte.trimmed().isEmpty()) {
        m_err->setText(erreur.isEmpty() ? "Aucun texte lisible dans ce fichier." : erreur);
        return;
    }
    m_err->clear();
    m_cvBtn->setEnabled(false);
    m_info->setText("Analyse du CV en cours...");

    auto* ex = new CvExtracteur(this);
    connect(ex, &CvExtracteur::termine, this, [this, ex](const ResultatCv& r) {
        ex->deleteLater();
        m_cvBtn->setEnabled(true);
        m_info->clear();
        CvPreviewDialog dlg(r, this);                      // l'humain valide
        if (dlg.exec() != QDialog::Accepted) return;
        QStringList all = m_comp->text().split(',', Qt::SkipEmptyParts);
        for (QString& s : all) s = s.trimmed();
        for (const QString& c : dlg.selection())
            if (!all.contains(c, Qt::CaseInsensitive)) all << c;
        m_comp->setText(all.join(", "));
    });
    ex->analyser(texte);
}

// ================================================================ CvExtracteur
CvExtracteur::CvExtracteur(QObject* parent) : QObject(parent), m_net(new QNetworkAccessManager(this)) {}

QString CvExtracteur::lireFichier(const QString& chemin, QString* erreur) {
    if (chemin.endsWith(".pdf", Qt::CaseInsensitive)) {
#ifdef HAS_QTPDF
        QPdfDocument doc;
        if (doc.load(chemin) != QPdfDocument::Error::None) {
            if (erreur) *erreur = "Impossible d'ouvrir ce PDF.";
            return {};
        }
        QString t;
        for (int i = 0; i < doc.pageCount(); ++i) t += doc.getAllText(i).text() + "\n";
        return t;
#else
        if (erreur) *erreur = "Lecture PDF indisponible (Qt 6 + module pdf requis). Utilisez un fichier .txt.";
        return {};
#endif
    }
    QFile f(chemin);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (erreur) *erreur = "Impossible d'ouvrir le fichier.";
        return {};
    }
    return QString::fromUtf8(f.readAll());
}

void CvExtracteur::analyser(const QString& texteCv) {
    // La clé d'API est lue dans une variable d'environnement (jamais dans le code source).
    const QString cle = qEnvironmentVariable("DEVENA_AI_KEY");
    if (cle.isEmpty()) {                       // pas de clé : mode dégradé local
        emit termine(extractionLocale(texteCv));
        return;
    }
    appelIA(texteCv.left(8000), cle);
}

void CvExtracteur::appelIA(const QString& texte, const QString& cle) {
    const QString consigne =
        "Tu es un assistant de recrutement pour un hackathon. À partir du CV ci-dessous, "
        "réponds UNIQUEMENT en JSON avec les clés : "
        "competences (liste de technologies et compétences, noms normalisés), "
        "niveau (Débutant, Intermédiaire ou Expert), "
        "domaine (Dev, IA et Data, Design ou Gestion). "
        "N'invente aucune compétence absente du CV.\n\nCV :\n" + texte;

    QNetworkRequest req(QUrl("https://api.anthropic.com/v1/messages"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("x-api-key", cle.toUtf8());
    req.setRawHeader("anthropic-version", "2023-06-01");
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    req.setTransferTimeout(15000);             // au-delà : repli local
#endif

    QJsonObject msg{{"role", "user"}, {"content", consigne}};
    QJsonObject body{{"model", qEnvironmentVariable("DEVENA_AI_MODEL", "claude-haiku-4-5-20251001")},
                     {"max_tokens", 600},
                     {"messages", QJsonArray{msg}}};

    QNetworkReply* rep = m_net->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(rep, &QNetworkReply::finished, this, [this, rep, texte] {
        rep->deleteLater();
        ResultatCv r;
        if (rep->error() == QNetworkReply::NoError) {
            const QJsonObject root = QJsonDocument::fromJson(rep->readAll()).object();
            const QString txt = root.value("content").toArray().at(0).toObject().value("text").toString();
            const int a = txt.indexOf('{'), b = txt.lastIndexOf('}');
            if (a >= 0 && b > a) {
                const QJsonObject o = QJsonDocument::fromJson(txt.mid(a, b - a + 1).toUtf8()).object();
                for (const QJsonValue& v : o.value("competences").toArray())
                    if (!v.toString().isEmpty()) r.competences << v.toString();
                r.niveau = o.value("niveau").toString();
                r.domaine = o.value("domaine").toString();
                r.viaIA = !r.competences.isEmpty();
            }
        }
        emit termine(r.viaIA ? r : extractionLocale(texte));   // erreur réseau ou réponse invalide : repli local
    });
}

ResultatCv CvExtracteur::extractionLocale(const QString& texte) const {
    struct Entree { const char* nom; QStringList alias; };
    static const QList<Entree> dico = {
                                        {"C++", {"c++", "cpp"}}, {"Qt", {"qt"}}, {"Python", {"python"}}, {"Java", {"java"}},
                                        {"JavaScript", {"javascript", "js"}}, {"TypeScript", {"typescript"}},
                                        {"React", {"react", "reactjs"}}, {"Node.js", {"node.js", "nodejs"}},
                                        {"Django", {"django"}}, {"Spring", {"spring", "spring boot"}},
                                        {"SQL", {"sql", "mysql", "postgresql"}}, {"Docker", {"docker"}},
                                        {"Git", {"git", "github"}}, {"Redis", {"redis"}}, {"Figma", {"figma"}},
                                        {"UI/UX", {"ui/ux", "ux/ui", "ui", "ux"}}, {"Prototypage", {"prototypage", "prototyping"}},
                                        {"Design system", {"design system"}}, {"Scrum", {"scrum", "agile"}},
                                        {"Gestion", {"gestion de projet", "project management"}},
                                        {"Pandas", {"pandas"}}, {"LLM", {"llm", "gpt"}}, {"TensorFlow", {"tensorflow"}},
                                        {"Machine Learning", {"machine learning", "apprentissage automatique"}},
                                        {"IA", {"ia", "intelligence artificielle"}}};

    ResultatCv r;
    for (const Entree& e : dico)
        for (const QString& a : e.alias) {
            // mots entiers uniquement (évite "Java" dans "JavaScript", "C++" dans "C+++", etc.)
            const QRegularExpression re("(?<![A-Za-z0-9+#])" + QRegularExpression::escape(a) +
                                            "(?![A-Za-z0-9+#])",
                                        QRegularExpression::CaseInsensitiveOption);
            if (re.match(texte).hasMatch()) { r.competences << QString(e.nom); break; }
        }

    const int n = r.competences.size();
    r.niveau = n > 6 ? "Expert" : n > 3 ? "Intermédiaire" : "Débutant";

    static const QStringList design = {"Figma", "UI/UX", "Prototypage", "Design system"};
    static const QStringList ia = {"Pandas", "LLM", "TensorFlow", "Machine Learning", "IA"};
    static const QStringList gest = {"Scrum", "Gestion"};
    int d = 0, i = 0, g = 0, dev = 0;
    for (const QString& c : r.competences) {
        if (design.contains(c)) ++d;
        else if (ia.contains(c)) ++i;
        else if (gest.contains(c)) ++g;
        else ++dev;
    }
    int best = dev;
    r.domaine = "Dev";
    if (i > best) { best = i; r.domaine = "IA et Data"; }
    if (d > best) { best = d; r.domaine = "Design"; }
    if (g > best) { r.domaine = "Gestion"; }
    r.viaIA = false;
    return r;
}

// ================================================================ Aperçu de l'extraction
CvPreviewDialog::CvPreviewDialog(const ResultatCv& r, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Aperçu de l'extraction");
    setMinimumWidth(380);
    auto* v = new QVBoxLayout(this);

    auto* src = new QLabel(r.viaIA ? "Source : analyse par IA" : "Source : extraction locale");
    src->setObjectName("sub");
    v->addWidget(src);
    v->addWidget(new QLabel(QString("Niveau estimé : %1   |   Domaine : %2")
                                .arg(r.niveau.isEmpty() ? "-" : r.niveau,
                                     r.domaine.isEmpty() ? "-" : r.domaine)));
    v->addWidget(new QLabel(r.competences.isEmpty() ? "Aucune compétence détectée."
                                                    : "Cochez les compétences à conserver :"));
    m_list = new QListWidget;
    for (const QString& c : r.competences) {
        auto* it = new QListWidgetItem(c, m_list);
        it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
        it->setCheckState(Qt::Checked);
    }
    v->addWidget(m_list);

    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    v->addWidget(bb);
}

QStringList CvPreviewDialog::selection() const {
    QStringList out;
    for (int i = 0; i < m_list->count(); ++i)
        if (m_list->item(i)->checkState() == Qt::Checked) out << m_list->item(i)->text();
    return out;
}