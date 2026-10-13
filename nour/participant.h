#pragma once
#include <QAbstractTableModel>
#include <QColor>
#include <QDialog>
#include <QList>
#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QUndoCommand>

class QComboBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QNetworkAccessManager;
class QPushButton;
class QSortFilterProxyModel;
class QTableView;
class QUndoStack;

// ---------------------------------------------------------------- Données
QString makeStyle(bool dark);              // feuille de style QSS clair / sombre

struct Theme {
    static inline QColor accent{"#1f6feb"};
    static inline QColor accentBg{"#e3eefe"};
};

struct Participant {
    QString id, nom, prenom, email;
    QStringList competences;
    QString pole() const;
};

// ---------------------------------------------------------------- Modèle
class ParticipantModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Role { IdRole = Qt::UserRole + 1 };
    explicit ParticipantModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation o, int role) const override;

    const QList<Participant>& all() const { return m_data; }
    const Participant& at(int row) const { return m_data.at(row); }
    int rowOf(const QString& id) const;
    QString nextId();
    void insertAt(int row, const Participant& p);
    void removeAt(int row);
    void replace(int row, const Participant& p);

signals:
    void changed();

private:
    QList<Participant> m_data;
    int m_next = 1;
};

// ---------------------------------------------------------------- Annulation (QUndoStack)
class AddCmd : public QUndoCommand {
public:
    AddCmd(ParticipantModel* m, const Participant& p);
    void redo() override;
    void undo() override;
private:
    ParticipantModel* m_m; Participant m_p; int m_row;
};

class DeleteCmd : public QUndoCommand {
public:
    DeleteCmd(ParticipantModel* m, const Participant& p);
    void redo() override;
    void undo() override;
private:
    ParticipantModel* m_m; Participant m_p; int m_row;
};

class EditCmd : public QUndoCommand {
public:
    EditCmd(ParticipantModel* m, const Participant& oldP, const Participant& newP);
    void redo() override;
    void undo() override;
private:
    ParticipantModel* m_m; Participant m_old, m_new;
};

// ---------------------------------------------------------------- Delegates
class CellDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
protected:
    void paintBase(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex& i) const;
};

class BadgeDelegate : public CellDelegate {
public:
    using CellDelegate::CellDelegate;
    void paint(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex& i) const override;
    QSize sizeHint(const QStyleOptionViewItem& o, const QModelIndex& i) const override;
};

class ActionsDelegate : public CellDelegate {
    Q_OBJECT
public:
    using CellDelegate::CellDelegate;
    void paint(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex& i) const override;
    bool editorEvent(QEvent* e, QAbstractItemModel* m, const QStyleOptionViewItem& o,
                     const QModelIndex& i) override;
signals:
    void viewClicked(const QString& id);
    void editClicked(const QString& id);
    void deleteClicked(const QString& id);
private:
    static QRect btn(const QRect& cell, int k);
};

// ---------------------------------------------------------------- Dialogues
class ProfileDialog : public QDialog {
public:
    ProfileDialog(const Participant& p, QWidget* parent);
};

class ParticipantFormDialog : public QDialog {
public:
    explicit ParticipantFormDialog(QWidget* parent, const Participant* p = nullptr);
    Participant result(const QString& id) const;
private:
    void validate();
    void importerCv();
    QLineEdit *m_nom, *m_prenom, *m_email, *m_comp;
    QLabel* m_err;
    QLabel* m_info;
    QPushButton* m_cvBtn;
};

// ---------------------------------------------------------------- Extraction des compétences d'un CV
struct ResultatCv {
    QStringList competences;
    QString niveau, domaine;
    bool viaIA = false;            // false = extraction locale (mode dégradé)
};

class CvExtracteur : public QObject {
    Q_OBJECT
public:
    explicit CvExtracteur(QObject* parent = nullptr);
    void analyser(const QString& texteCv);                       // IA, sinon repli local
    static QString lireFichier(const QString& chemin, QString* erreur);   // .txt ou .pdf
signals:
    void termine(const ResultatCv& r);
private:
    ResultatCv extractionLocale(const QString& texte) const;
    void appelIA(const QString& texte, const QString& cle);
    QNetworkAccessManager* m_net;
};

class CvPreviewDialog : public QDialog {
public:
    CvPreviewDialog(const ResultatCv& r, QWidget* parent);
    QStringList selection() const;
private:
    QListWidget* m_list;
};

// ---------------------------------------------------------------- Fenêtre principale
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onAdd();
    void onEdit(const QString& id);
    void onDelete(const QString& id);
    void onView(const QString& id);
    void exportPdf();
    void toggleTheme();
    void refreshKpis();

private:
    void applyTheme(bool dark);
    void loadSample();
    QLabel* addKpi(QHBoxLayout* layout, const QString& title);
    QWidget* buildSidebar();       // barre latérale gauche
    QWidget* buildStatsChart();    // courbe des statistiques

    ParticipantModel* m_model;
    QSortFilterProxyModel* m_proxy;
    QTableView* m_view;
    QUndoStack* m_stack;
    QLineEdit* m_search;
    QPushButton* m_themeBtn;
    QLabel *m_kTotal, *m_kIA, *m_kDev, *m_kDes;
    bool m_dark = false;
};