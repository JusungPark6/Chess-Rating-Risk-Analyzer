
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QString>
#include <iostream>
#include <cmath>
#include <vector>
#include <tuple>
#include <string>
#include <curl/curl.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;

// Constants for the Glicko-1 system
const double q = 0.0057565;
const double pi = 3.14159265358979323846;

class Player {
public:
    int Rating, RD;
    std::string username;
    std::string gamemode;

    Player(std::string mode) : gamemode(mode) {}

    void stats(std::string gamemode) {
        fetchPlayerData(username, gamemode);
    }

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalSize = size * nmemb;
        userp->append((char*)contents, totalSize);
        return totalSize;
    }

    json getPlayerStats(const std::string& username) {
        std::string readBuffer;
        CURL* curl;
        CURLcode res;
        std::string url = "https://api.chess.com/pub/player/" + username + "/stats";

        curl = curl_easy_init();
        if (curl) {
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);

            if (res != CURLE_OK) {
                throw std::runtime_error("Failed to fetch data from Chess.com API");
            }
        }

        return json::parse(readBuffer);
    }

    void fetchPlayerData(std::string user, std::string gamemode) {
        try {
            json stats = getPlayerStats(user);
            if (stats.contains(gamemode) && stats[gamemode].contains("last")) {
                int rating = stats[gamemode]["last"]["rating"];
                int rd = stats[gamemode]["last"]["rd"];
                Rating = rating;
                RD = rd;
            }
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }
};

class Game {
    double r, RD, r_j, RD_j;

public:
    Game(double playerRating, double playerRD, double oppRating, double oppRD) {
        r = playerRating;
        RD = playerRD;
        r_j = oppRating;
        RD_j = oppRD;
    }

    double calculate_new_rating(double r, double RD, double r_j, double RD_j, double s) {
        double gRD_j = 1 / sqrt(1 + 3 * pow(q, 2) * pow(RD_j, 2) / pow(pi, 2));
        double E = 1 / (1 + pow(10, -gRD_j * (r - r_j) / 400));
        double sum = pow(gRD_j, 2) * E * (1 - E);
        double sum_s_minus_E = gRD_j * (s - E);

        double d_2 = 1 / (q * q * sum);
        double denom = 1 / pow(RD, 2) + 1 / d_2;

        double new_r = r + (q / denom) * sum_s_minus_E;
        return new_r;
    }

    std::tuple<double, double, double> calculateRatingRes() {
        if (RD<55){RD=55;}
        if (RD_j<55){RD_j=55;}
        double win = calculate_new_rating(r, RD, r_j, RD_j, 1);
        double lose = calculate_new_rating(r, RD, r_j, RD_j, 0);
        double draw = calculate_new_rating(r, RD, r_j, RD_j, 0.5);
        return std::make_tuple(win, lose, draw);
    }
};

class ChessRatingApp : public QWidget {
public:
    ChessRatingApp(QWidget* parent = 0) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QLabel* playerLabel = new QLabel("Enter Player Chess.com Username:", this);
        playerEdit = new QLineEdit(this);

        QLabel* opponentLabel = new QLabel("Enter Opponent Chess.com Username:", this);
        opponentEdit = new QLineEdit(this);

        calculateButton = new QPushButton("Calculate", this);
        connect(calculateButton, &QPushButton::clicked, this, &ChessRatingApp::onCalculate);

        outputLabel = new QLabel("", this);

        mainLayout->addWidget(playerLabel);
        mainLayout->addWidget(playerEdit);
        mainLayout->addWidget(opponentLabel);
        mainLayout->addWidget(opponentEdit);
        mainLayout->addWidget(calculateButton);
        mainLayout->addWidget(outputLabel);

        setLayout(mainLayout);
        setWindowTitle("Chess Rating Calculator");
        resize(400, 200);
    }

private slots:
    void onCalculate() {
        QString playerUsername = playerEdit->text();
        QString opponentUsername = opponentEdit->text();

        Player player("chess_bullet");
        player.username = playerUsername.toStdString();
        player.stats("chess_bullet");

        Player opponent("chess_bullet");
        opponent.username = opponentUsername.toStdString();
        opponent.stats("chess_bullet");

        if (player.Rating == 0 || opponent.Rating == 0) {
            QMessageBox::warning(this, "Error", "Failed to fetch player data. Please check the usernames and try again.");
            return;
        }

        Game game(player.Rating, player.RD, opponent.Rating, opponent.RD);
        auto results = game.calculateRatingRes();

        QString resultText = QString("If you win: %1\nIf you lose: %2\nIf you draw: %3")
                             .arg(std::get<0>(results))
                             .arg(std::get<1>(results))
                             .arg(std::get<2>(results));

        outputLabel->setText(resultText);

    }

private:
    QLineEdit* playerEdit;
    QLineEdit* opponentEdit;
    QPushButton* calculateButton;
    QLabel* outputLabel;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ChessRatingApp window;
    window.show();
    return app.exec();
}