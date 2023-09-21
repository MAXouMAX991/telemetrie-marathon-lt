#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Initialisation de l'interface graphique
    ui->setupUi(this);

    // Instanciation de la socket
    tcpSocket = new QTcpSocket(this);

    // Attachement d'un slot qui sera appelé à chaque fois que des données arrivent (mode asynchrone)
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(gerer_donnees()));

    // Idem pour les erreurs
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(afficher_erreur(QAbstractSocket::SocketError)));

    //Instanciation du timer
    pTimer = new QTimer();

    // Association du "tick" du timer à l'appel d'une méthode SLOT faire_qqchose()
        connect(pTimer, SIGNAL(timeout()), this, SLOT(mettre_a_jour_ihm()));

    // Lancement du timer avec un tick toutes les 1000 ms
    pTimer->start(1000);

    // Instanciation de l'image
    pCarte = new QImage();
    pCarte_Satellite = new QImage();
    pCarte_Transparent = new QImage();

    // Chargement depuis un fichier
    pCarte->load(":/carte_la_rochelle_plan.png");
    pCarte_Satellite->load(":/carte_la_rochelle_satellite.png");
    pCarte_Transparent->load(":/carte_la_rochelle_transparent.png");

    // Affichage dans un QLabel, ici label_carte
    ui->label_carte->setPixmap(QPixmap::fromImage(*pCarte));
    ui->label_carte_satellite->setPixmap(QPixmap::fromImage(*pCarte_Satellite));
    ui->label_carte_transparent->setPixmap(QPixmap::fromImage(*pCarte_Transparent));

}

MainWindow::~MainWindow()
{
    // Destruction de la socket
    tcpSocket->abort();
    delete tcpSocket;

    // Arrêt du timer
    pTimer->stop();

    // Destruction de l'objet
    delete pTimer;

    // Destruction de l'interface graphique
    delete ui;

    // Suppression de l'image
    delete pCarte;
    delete pCarte_Satellite;
    delete pCarte_Transparent;
}

void MainWindow::on_connexionButton_clicked()
{
    // Récupération des paramètres
    QString adresse_ip = ui->lineEdit_ip->text();
    unsigned short port_tcp = ui->lineEdit_port->text().toInt();

    // Connexion au serveur
    tcpSocket->connectToHost(adresse_ip, port_tcp);
}

void MainWindow::on_deconnexionButton_clicked()
{
    // Déconnexion du serveur
    tcpSocket->close();
}

void MainWindow::on_envoiButton_clicked()
{
    // Préparation de la requête
    QByteArray requete;
    requete = "RETR\r\n";

    // Envoi de la requête
    tcpSocket->write(requete);
}

void MainWindow::gerer_donnees()
{
    // Réception des données
    QByteArray reponse = tcpSocket->readAll();
    QString trame = QString(reponse);
    qDebug() << trame;

    // Affichage
    ui->lineEdit_ip_15->setText(QString(reponse));

    //Décodage
    QStringList liste = trame.split(",");
    qDebug() << "Heures :" << liste[1].mid(0,2);
    qDebug() << "Minutes :" << liste[1].mid(2,2);
    qDebug() << "Secondes :" << liste[1].mid(4,2);

    //Date
    int heures = liste[1].mid(0,2).toInt();
    int minutes = liste[1].mid(2,2).toInt();
    int secondes = liste[1].mid(4,2).toInt();
    int timestamp = (heures * 3600 + minutes * 60 + secondes);
    qDebug() << "Timestamp : " << timestamp;
    QString timestampQString = QString("%1").arg(timestamp);
    //ui->lineEdit_Heure->setText(timestampQString);

    //Latitude
    double latitude_degre = liste[2].mid(0,2).toDouble();
    qDebug() << "Degrés :" << liste[2].mid(0,2);
    double latitude_minutes = liste[2].mid(2,7).toDouble();
    qDebug() << "Minutes :" << liste[2].mid(2,7);

    //Nord ou Sud de l'Equateur
    QString N_ou_S = liste[3].mid(0,1);
    qDebug() << "N/S :" << liste[3].mid(0,1);

    double latitude = 0.0;
    //Calcul Latitude
    if( N_ou_S == "S"){
        latitude = (latitude_degre + (latitude_minutes / 60))*(-1);
        qDebug() << "Latitude :" << latitude;
    }else {
        latitude = latitude_degre + (latitude_minutes / 60);
        qDebug() << "Latitude :" << latitude;
    }

    //Longitude
    double longitude_degre = liste[4].mid(0,3).toDouble();
    qDebug() << "Degrés :" << liste[4].mid(0,3);
    double longitude_minutes = liste[4].mid(3,7).toDouble();
    qDebug() << "Minutes :" << liste[4].mid(3,7);

    //Ouest ou Est de Greenwich
    QString W_ou_E = liste[5].mid(0,1);
    qDebug() << "W/E :" << liste[5].mid(0,1);

    double longitude = 0.0;
    //Calcul Longitude
    if( W_ou_E == "W"){
        longitude = (longitude_degre + (longitude_minutes / 60))*(-1);
        qDebug() << "Longitude :" << longitude;
    }else {
        longitude = longitude_degre + (longitude_degre / 60);
        qDebug() << "Longitude :" << longitude;
    }

    //Type de positionnement
    int positionnement = liste[6].mid(0,1).toInt();
    qDebug() << "Type de positionnement :" << liste[6].mid(0,1);

    //Nombre de Satellites
    int satellites = liste[7].mid(0,2).toInt();
    qDebug() << "Nombre de satellites :" << liste[7].mid(0,2);

    //Précision horizontale
    float précision = liste[8].mid(0,3).toFloat();
    qDebug() << "Précision horizontale :" << liste[8].mid(0,3);

    //Altitude
    float altitude = liste[9].mid(0,3).toFloat();
    qDebug() << "Altitude :" << liste[9].mid(0,3);

    //Unité altitude
    QString unite_altitude = liste[10].mid(0,1);
    qDebug() << "Unité :" << liste[10].mid(0,1);

    //Hauteur de la géodésique au dessus de l'ellipsoïde WGS84
    float hauteur_geodesique = liste[11].mid(0,3).toFloat();
    qDebug() << "Hauteur géodésique :" << liste[11].mid(0,3);

    //Unité hauteur
    QString unite_hauteur = liste[12].mid(0,1);
    qDebug() << "Unité :" << liste[12].mid(0,1);

    //Temps depuis la dernière mise à jour DGPS
    float temps_maj = liste[13].mid(0,3).toFloat();
    qDebug() << "Temps depuis la dernière maj :" << liste[13].mid(0,3);

    //Fréquence cardiaque
    int frequence_cardiaque = liste[14].mid(0,4).toInt();
    qDebug() << "Fréquence Cardiaque :" << liste[14].mid(0,4);

    float px = 694 * ( (longitude - -1.195703 ) / (-1.136125 - -1.195703) );
    float py = 638 * ( 1.0 - (latitude - 46.135451) / (46.173311 - 46.135451) );

    // Préparation du contexte de dessin sur une image existante
    QPainter p(pCarte_Transparent);
    // Choix de la couleur
    if ((lastpx != 0.0) && (lastpy != 0.0)){
        p.setPen(Qt::red);
        // Dessin d'une ligne
        p.drawLine(lastpx, lastpy, px, py);
        p.end();
        ui->label_carte_transparent->setPixmap(QPixmap::fromImage(*pCarte_Transparent));
    }
    else {
    }
    lastpx = px;
    lastpy = py;
    qDebug()<< "px:"<<px;
    qDebug()<< "py:"<<px;
    qDebug()<< "lastpx:"<<lastpx;
    qDebug()<< "lastpy:"<<lastpy;
}

void MainWindow::mettre_a_jour_ihm()
{
    // Préparation de la requête
    QByteArray requete;
    requete = "RETR\r\n";

    // Envoi de la requête
    tcpSocket->write(requete);
}

void MainWindow::afficher_erreur(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Hôte introuvable"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Connexion refusée"));
            break;
        default:
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Erreur : %1.")
                                     .arg(tcpSocket->errorString()));
    }
}
