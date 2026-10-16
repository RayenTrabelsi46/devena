#include "connection.h"

// Ctor
// Initialise l'attribut `db` avec le driver ODBC (Oracle).
// Utilise un nom de connexion unique "devena_conn" pour éviter les conflits
// si Qt détecte une tentative d'enregistrement d'une connexion déjà existante.
// On le fait dans le constructeur pour que `db` soit stocké dans le Singleton.
// Cela signifie que la même instance de `QSqlDatabase` sera utilisée partout dans l'application
// via le Singleton, au lieu de créer une nouvelle connexion à chaque fois.
Connection::Connection() {
    // Vérifie si une connexion nommée existe déjà pour éviter les duplicatas
    if (QSqlDatabase::contains("devena_conn")) {
        db = QSqlDatabase::database("devena_conn");
    } else {
        db = QSqlDatabase::addDatabase("QODBC", "devena_conn");
    }
}

//Dtor
// Ferme la connexion si elle est encore ouverte lorsque l'objet est détruit
Connection::~Connection() {
    if (db.isOpen())
        db.close(); // Libère proprement les ressources
}

// --- Accès à l'instance unique du Singleton ---
// Utilise la technique "Meyers Singleton" : instance statique locale
// Elle est créée la première fois qu'on appelle createInstance()
// et reste vivante pendant toute la durée du programme (thread-safe en C++11+)
Connection& Connection::createInstance() {
    static Connection instance;  // Instance unique du Singleton
    return instance; // Retourne une référence à cette instance
}


bool Connection::createConnection() {
    db.setDatabaseName("devena");           // Nom du DSN ODBC configuré dans Windows
    db.setUserName("devena");               // Nom d'utilisateur Oracle
    db.setPassword("esprit2006");           // Mot de passe Oracle

    if (db.open()) {
        qDebug() << "[Devena] Connexion Oracle/ODBC établie via DSN : Source_ProjetC++";
        return true;
    }

    qDebug() << "[Devena] Échec de la connexion Oracle/ODBC :" << db.lastError().text();
    return false;
}

