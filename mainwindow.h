#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QImage>
#include <QPixmap>

struct Participant
{
    int id;
    QString nom;
    QString prenom;
    QString sexe;
    QString competence;
    int age;
};

class Equipe
{
public:
    Equipe(int id = 0, const QString &nom = "");

    int id() const
    {
        return m_id;
    }

    QString nom() const
    {
        return m_nom;
    }

    void setNom(const QString &nom)
    {
        m_nom = nom;
    }

    bool ajouter(const Participant &p);
    bool supprimer(int id);
    void vider();

    int taille() const
    {
        return m_membres.size();
    }

    bool pleine() const
    {
        return m_membres.size() >= 6;
    }

    int hommes() const;
    int femmes() const;
    double ageMoyen() const;
    bool aCompetence(const QString &c) const;
    QString statut() const;

    const QVector<Participant> &membres() const
    {
        return m_membres;
    }

private:
    int m_id;
    QString m_nom;
    QVector<Participant> m_membres;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void creerInterface();
    void creerDonnees();

    int prochainIdEquipe() const;
    int prochainIdParticipant() const;

    int meilleureEquipe(const Participant &p) const;

    void affecterAutomatiquement();

    void ajouterEquipe();
    void modifierEquipe(int index);
    void supprimerEquipe(int index);

    void ajouterMembre(bool automatique, int indexEquipe = -1);

    void voirMembres(int index);
    void supprimerMembre(int index);

    QImage creerQR(const QString &code) const;
    void afficherQR(int index);

    QImage creerBadge(const Equipe &e) const;
    void afficherBadge(int index);

    void afficherEquipes();

    QPixmap creerCourbe() const;
    void afficherStatistiques();

    void filtrer();

    void afficherNotification(
        const QString &texte,
        bool succes = true
        );

    void rafraichir();

    // NOUVEAUX
    void exporterPDF();
    void changerModeClair();

private:
    class QTableWidget *m_table;
    class QLineEdit *m_recherche;
    class QComboBox *m_filtre;
    class QLabel *m_notification;
    class QLabel *m_courbe;

    class QLabel *m_totalEquipes;
    class QLabel *m_totalMembres;
    class QLabel *m_totalHommes;
    class QLabel *m_totalFemmes;

    class QWidget *m_side;

    QVector<Equipe> m_equipes;
    QVector<Participant> m_participants;

    bool m_modeClair = false;
};

#endif // MAINWINDOW_H