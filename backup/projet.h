#ifndef PROJET_H
#define PROJET_H

#include <QString>

struct Projet
{
    QString ID_Projet;
    QString titre;
    QString description;
    QString thematique;
    double noteFinale = 0.0;
    QString ID_Equipe;
};

#endif // PROJET_H