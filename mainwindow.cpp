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
    ui->label_carte_transparent->setPixmap(QPixmap::fromImage(*pCarte_Transparent));

    //Connection à la base données
    bdd = QSqlDatabase::addDatabase("QSQLITE");
    bdd.setDatabaseName(QCoreApplication::applicationDirPath() + "/marathon.sqlite");
    if (!bdd.open())
    {
        qDebug() << "Error: connection with database fail";
    }
    else
    {
        qDebug() << "Database: connection ok";
    }

    lastpx = 0.0;
    lastpy = 0.0;
    distAB = 0.0;
    lastdistance = 0.0;
    lastlat_rad = 0.0;
    lastlong_rad = 0.0;
    vitesse = 0.0;
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

    //bdd.close;
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

//Conversion degré to radian
double degToRad(double degrees){
    return degrees * M_PI / 180.0;
}

void MainWindow::gerer_donnees()
{
    // Réception des données
    QByteArray reponse = tcpSocket->readAll();
    QString trame = QString(reponse);
    qDebug() << trame;

    //Séparation des données de la trame
    QStringList liste = trame.split(",");

    //Nombre de Satellites
    qDebug() << "Nombre de satellites :" << liste[7].mid(0,2);
    int satellites = liste[7].mid(0,2).toInt();
    ui->lineEdit_ip_12->setText(QString::number(satellites));

    //Condition pour le décodage de la trame
    if (satellites < 3)
    {
        ui->lineEdit_ip_12->setText("Nombre de satellites insuffisant");
    }
    else{
        // Réception des données
        QByteArray reponse = tcpSocket->readAll();
        QString trame = QString(reponse);
        qDebug() << trame;

        // Affichage de la trame complète
        ui->lineEdit_ip_15->setText(QString(reponse));

        //Décodage de l'heure
        int heures = liste[1].mid(0,2).toInt();
        int minutes = liste[1].mid(2,2).toInt();
        int secondes = liste[1].mid(4,2).toInt();
        int timestamp = (heures * 3600 + minutes * 60 + secondes);
        QString timestampQString = QString("%1").arg(timestamp);

        //Temps écoulé
        int temps_ecoule_heures = ((timestamp - 28957) / 3600);
        int temps_ecoule_minutes = (timestamp % 3600) / 60;
        int temps_ecoule_secondes = timestamp % 60;

        //Conversion du temps en chaîne de caractères + affichage
        QString temps_ecoule_heuresQString = QString("%1").arg(temps_ecoule_heures);
        QString temps_ecoule_minutesQString = QString("%1").arg(temps_ecoule_minutes);
        QString temps_ecoule_secondesQString = QString("%1").arg(temps_ecoule_secondes);
        ui->lineEdit_ip_8->setText(temps_ecoule_heuresQString + " h " + temps_ecoule_minutesQString + " min " + temps_ecoule_secondesQString + " s");

        //Décodage de la Latitude
        double latitude_degre = liste[2].mid(0,2).toDouble();
        double latitude_minutes = liste[2].mid(2,7).toDouble();

        //Nord ou Sud de l'Equateur
        QString N_ou_S = liste[3].mid(0,1);

        //Calcul Latitude
        double latitude = 0.0;
        if( N_ou_S == "S"){
            latitude = (latitude_degre + (latitude_minutes / 60))*(-1);
        }else {
            latitude = latitude_degre + (latitude_minutes / 60);
        }
        ui->lineEdit_ip_2->setText(QString::number(latitude));

        //Décodage de la Longitude
        double longitude_degre = liste[4].mid(0,3).toDouble();
        double longitude_minutes = liste[4].mid(3,7).toDouble();

        //Ouest ou Est de Greenwich
        QString W_ou_E = liste[5].mid(0,1);

        //Calcul Longitude
        double longitude = 0.0;
        if( W_ou_E == "W"){
            longitude = (longitude_degre + (longitude_minutes / 60))*(-1);
        }else {
            longitude = longitude_degre + (longitude_degre / 60);
        }
        ui->lineEdit_ip_3->setText(QString::number(longitude));

        //Type de positionnement
        int positionnement = liste[6].mid(0,1).toInt();

        //Précision horizontale
        float précision = liste[8].mid(0,3).toFloat();

        //Décodage de l'Altitude
        float altitude = liste[9].mid(0,3).toFloat();
        ui->lineEdit_ip_4->setText(QString::number(altitude));

        //Unité altitude
        QString unite_altitude = liste[10].mid(0,1);

        //Hauteur de la géodésique au dessus de l'ellipsoïde WGS84
        float hauteur_geodesique = liste[11].mid(0,3).toFloat();

        //Unité hauteur
        QString unite_hauteur = liste[12].mid(0,1);

        //Temps depuis la dernière mise à jour DGPS
        float temps_maj = liste[13].mid(0,3).toFloat();

        //Décodage de la Fréquence cardiaque
        int frequence_cardiaque = liste[14].mid(0,4).toInt();
        ui->lineEdit_ip_5->setText(QString::number(frequence_cardiaque));

        // Préparation du contexte de dessin sur une image existante
        float px = 694 * ( (longitude - -1.195703 ) / (-1.136125 - -1.195703) );
        float py = 638 * ( 1.0 - (latitude - 46.135451) / (46.173311 - 46.135451) );

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

        //Calcul de FCmax
        int age = ui->spinBox->value();
        int FCmax = 220 - age;
        ui->lineEdit_ip_6->setText(QString::number(FCmax));

        //Calcul Intensité de l'effort
        double Intensite = (frequence_cardiaque * 100)/FCmax;
        ui->progressBar->setValue(Intensite);

        //Distance parcourue
        if(lastlat_rad != 0.0 && lastlong_rad != 0.0){
            distAB = 6378.0 * acos(sin(lastlat_rad)*sin(lat_rad) + cos(lastlat_rad)* cos(lat_rad)* cos(lastlong_rad - long_rad));
            distance = distAB +lastdistance;
            QString distAB_string = QString("%1").arg(distance);
            ui->lineEdit_ip_9->setText(distAB_string);
        }else{

        }
        //Calcul des calories dépensées
        int poids = ui->spinBox_3->value();
        double calories = (distAB * poids * 1.036);
        ui->lineEdit_ip_11->setText(QString::number(calories));

        //Calcul vitesse
        vitesse = distAB / temps_ecoule_heures;
        ui->lineEdit_ip_10->setText(QString::number(temps_ecoule_heures));

        //Affichage des cartes plan ou satellites
        if(ui->checkBoxCarte->isChecked()){
            ui->label_carte->setPixmap(QPixmap::fromImage(*pCarte));
        }else{
            ui->label_carte->setPixmap(QPixmap::fromImage(*pCarte_Satellite));
        }
    }
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
