#pragma once

#include <QWidget>
#include <QDate>

QT_BEGIN_NAMESPACE
namespace Ui { class EmployePage; }
QT_END_NAMESPACE

struct EmployeData
{
    QString id;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QDate dateEmbauche;
    double salaire = 0.0;
    QString poste;
    QString motDePasse;
    QString statut;
};

class EmployePage : public QWidget
{
    Q_OBJECT

public:
    explicit EmployePage(QWidget *parent = nullptr);
    ~EmployePage();

private slots:
    void ajouterEmploye();
    void modifierEmploye();
    void supprimerEmploye();
    void rechercherEmploye(const QString &texte);
    void exporterPDF();
    void changerTheme();
    void viderRecherche();
    void chargerSelection();

private:
    void chargerExemples();
    void ajouterLigne(const EmployeData &e);
    EmployeData lireLigne(int row) const;
    int ligneSelectionnee() const;
    void appliquerStyle(bool sombre);
    void ouvrirFormulaire(int row);

    Ui::EmployePage *ui;
    bool m_sombre = true;
    int m_nextId = 4;
};
