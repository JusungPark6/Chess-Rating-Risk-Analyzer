
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
    std::vector<double> abortsProb;

public:
    Game(double playerRating, double playerRD, double oppRating, double oppRD) {
        r = playerRating;
        RD = playerRD;
        r_j = oppRating;
        RD_j = oppRD;
        abortsProb = std::vector<double>(6, 1.0 / 6.0); // Uniform prior over [5, 10]
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
        if (RD < 55) { RD = 55; }
        if (RD_j < 55) { RD_j = 55; }
        double win = calculate_new_rating(r, RD, r_j, RD_j, 1);
        double lose = calculate_new_rating(r, RD, r_j, RD_j, 0);
        double draw = calculate_new_rating(r, RD, r_j, RD_j, 0.5);
        return std::make_tuple(win, lose, draw);
    }

    std::string analyzeRisk(double playerRating, double oppRating, double win, double lose, double draw) {
        // Calculate the rating difference
        double ratingDifference = oppRating - playerRating;
        
        // Adjust the scaling factor based on the rating difference
        double scalingFactor = 250.0 / std::abs(ratingDifference);
        
        // Bradley-Terry model for probability estimation
        double win_prob = 1 / (1 + pow(10, ratingDifference / 400));
        double lose_prob = 1 / (1 + pow(10, -ratingDifference / 400));
        double draw_prob = 1 - win_prob - lose_prob;


        // Normalize probabilities to sum to 1
        double sum_probs = win_prob + lose_prob + draw_prob;
        win_prob /= sum_probs;
        lose_prob /= sum_probs;
        draw_prob /= sum_probs;

        win_prob *= scalingFactor;
        lose_prob /= scalingFactor;

        // Calculate the expected value (EV)
        double expected_value = (win - playerRating) * win_prob +
                                (lose - playerRating) * lose_prob +
                                (draw - playerRating) * draw_prob;

        // Risk-reward analysis
        double risk_reward_ratio = (win - playerRating) / (playerRating - lose);

        // Volatility adjustment
        double volatility = sqrt(pow(RD, 2) + pow(RD_j, 2));
        double adjusted_expected_value = expected_value / volatility;
        
        // Scarcity factor adjustment
        double scarcity_factor = 1.0;
        if (expected_value < 0) {
            scarcity_factor = 1.0 / (std::count_if(abortsProb.begin(), abortsProb.end(), [](double p) { return p > 0; }));
        }

        // Adjust decision thresholds dynamically based on scarcity factor
        double min_acceptable_gain = 5.0 * scarcity_factor;
        double max_acceptable_loss = -10.0 / scarcity_factor;

        // Determine the number of aborts left (based on the current belief)
        double expected_aborts_left = 0.0;
        for (int i = 0; i < abortsProb.size(); ++i) {
            expected_aborts_left += abortsProb[i] * (5 + i);
        }

        std::string decision;

        if (adjusted_expected_value > 0 && risk_reward_ratio > 1.0 && win - playerRating > min_acceptable_gain) {
            decision = "Play on: Positive expected value with favorable risk-reward ratio.";
        } else if (lose - playerRating < max_acceptable_loss) {
            decision = "Abort: High risk with significant potential loss.";
        } else if (expected_aborts_left > 0) {
            decision = "Abort: Negative or insufficient expected value.";
        } else {
            decision = "Play on: Limited aborts left.";
        }

        // Update the belief about the number of aborts left if decision is to abort
        if (decision.find("Abort") != std::string::npos) {
            std::vector<double> updatedAbortsProb(6, 0.0);
            for (int i = 0; i < abortsProb.size(); ++i) {
                if (i < abortsProb.size() - 1) {
                    updatedAbortsProb[i] = abortsProb[i + 1];
                }
            }
            abortsProb = updatedAbortsProb;
        }

        return decision;
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

        newGameButton = new QPushButton("New Game", this);
        connect(newGameButton, &QPushButton::clicked, this, &ChessRatingApp::onNewGame);
        outputLabel = new QLabel("", this);
        
        mainLayout->addWidget(playerLabel);
        mainLayout->addWidget(playerEdit);
        mainLayout->addWidget(opponentLabel);
        mainLayout->addWidget(opponentEdit);
        mainLayout->addWidget(calculateButton);
        mainLayout->addWidget(newGameButton);
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
        
        // Analyze risk and provide recommendation
        std::string riskAnalysis = game.analyzeRisk(player.Rating, opponent.Rating, std::get<0>(results), std::get<1>(results), std::get<2>(results));
        QString riskAnalysisQString = QString::fromStdString(riskAnalysis);

        resultText.append("\n\n" + riskAnalysisQString);

        outputLabel->setText(resultText);

    }
    void onNewGame() {
        opponentEdit->clear();
        outputLabel->clear();
    }

private:
    QLineEdit* playerEdit;
    QLineEdit* opponentEdit;
    QPushButton* calculateButton;
    QPushButton* newGameButton;
    QLabel* outputLabel;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ChessRatingApp window;
    window.show();
    return app.exec();
}