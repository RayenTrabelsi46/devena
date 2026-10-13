#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>

#include "projet.h"

class QListWidget;
class QLineEdit;
class QLabel;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void creerInterface();
    void afficherListe();
    void ajouterProjet();
    void afficherProjet(int index);
    void modifierProjet(int index);
    void supprimerProjet(int index);
    void definirGagnant(int index);
    void afficherStatistiques();
    void afficherCelebration(const QString &nomProjet);

private:
    QList<Projet> projets;

    QLineEdit *recherche = nullptr;
    QListWidget *listeProjets = nullptr;

    QLabel *statProjets = nullptr;
    QLabel *statNombre = nullptr;
    QLabel *statDomaines = nullptr;
    QLabel *statGagnant = nullptr;
};

#endif // MAINWINDOW_H
