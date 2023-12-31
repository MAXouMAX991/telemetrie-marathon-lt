#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QPainter>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_connexionButton_clicked();

    void on_deconnexionButton_clicked();

    void on_envoiButton_clicked();

    void gerer_donnees();

    void afficher_erreur(QAbstractSocket::SocketError);

    void mettre_a_jour_ihm();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    QTimer *pTimer;
    QImage *pCarte;
    QImage *pCarte_Satellite;
    QImage *pCarte_Transparent;
    QSqlDatabase bdd;
    float lastpx;
    float lastpy;
    float lastlat_rad;
    float lat_rad;
    float lastlong_rad;
    float long_rad;
    float lastdistance;
    float distAB;
    float distance;
    float vitesse;
};

#endif // MAINWINDOW_H
