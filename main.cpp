#include <iostream>
#include <cmath>
#include <vector>
#include "LRUCache.h"
#include <tuple>
#include <string>
#include <curl/curl.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;
using std::tuple;

class Player {
    public:
        int Rating, RD; // rating deviation, low RD means more frequent playing
        std::string username; //player chess.com username
        std::string gamemode;
        Player(std::string mode){
            gamemode = mode;
        }
        void stats(std::string gamemode){
            fetchPlayerData(username, gamemode);
        };
    
    private:

        // Callback function to handle the response data
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
            size_t totalSize = size * nmemb;
            userp->append((char*)contents, totalSize);
            return totalSize;
        };


        // Function to get player stats from Chess.com API
        json getPlayerStats(const std::string& username) {
            std::string readBuffer;
            CURL* curl;
            CURLcode res;
            std::string url = "https://api.chess.com/pub/player/" + username + "/stats";

            curl = curl_easy_init();
            if (curl) {
                struct curl_slist *headers = NULL;
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
            // std::cout << "Raw response: " << readBuffer << std::endl;

            return json::parse(readBuffer);
        }

        void fetchPlayerData(std::string user, std::string gamemode){
            try {
                std::cout << "Fetching " << user << "data";
                json stats = getPlayerStats(user);

                // Retrieve the rating and RD for the selected game mode
                if (stats.contains(gamemode) && stats[gamemode].contains("last")) {
                    int rating = stats[gamemode]["last"]["rating"];
                    int rd = stats[gamemode]["last"]["rd"];
                    std::cout << "Rating: " << rating << std::endl;
                    std::cout << "RD: " << rd << std::endl;
                    Rating = rating;
                    RD = rd;
                } else {
                    std::cout << "No stats available for the selected game mode." << std::endl;
                }
            } catch (const std::exception& ex) {
                std::cerr << "Error: " << ex.what() << std::endl;
            }
            
        };
        // void fetchRecentOpps(char username);
};


class Game {
    std::string player, opponent;
    double r, RD, r_j, RD_j;
    static constexpr double q = 0.0057565;
    static constexpr double pi = 3.14159265358979323846;
    
    public:
        Game(std::string player1, std::string player2, double playerRating, double playerRD, double oppRating, double oppRD){
            player = player1;
            opponent = player2;
            r = playerRating;
            RD = playerRD;
            oppRating = r_j;
            oppRD = RD_j;
        }
        void calculateRatingRes(double r, double RD, double r_j, double RD_j){

            


            auto calculate_new_rating = [&](double s){
                double sum;
                double sum_s_minus_E = 0.0;
                double gRD = 1/(sqrt((1+3*pow(q,2)*(pow(RD,2))/pow(pi, 2))));

                double gRD_j = 1/(sqrt((1+3*pow(q,2)*(pow(RD_j,2))/pow(pi, 2)))); 
                double E = 1/(1+pow(10,(-gRD_j)*(r-r_j)/400)); 
                sum += + pow(gRD_j, 2) * E * (1 - E); 
                sum_s_minus_E += gRD_j * (s - E);

                double d_2 = pow(pow(q, 2) * sum, -1);
                double new_r = r + q / ((1 / pow(RD, 2)) + (1 / d_2)) * sum_s_minus_E;
                double new_RD = sqrt(pow((1 / pow(RD, 2)) + (1 / d_2), -1));
                double E_j = 1 / (1 + pow(10, -gRD_j * (r - r_j) / 400)); 
                double sum_j = sum + pow(gRD_j, 2) * E_j * (1 - E_j);
                double d_2_j = pow(pow(q, 2) * sum_j, -1);
                double sum_s_minus_E_j = sum_s_minus_E + gRD_j * (s - E_j);
                double new_r_opponent = r + q / ((1 / pow(RD, 2)) + (1 / d_2_j)) * sum_s_minus_E_j;
                return new_r_opponent;
            };

            // int r = 1500;
            // int RD = 200;
            // double q = 0.0057565;
            // double pi = 2*asin(1.0);
            // double gRD = 1/(sqrt((1+3*pow(q,2)*(pow(RD,2))/pow(pi, 2))));


            // double sum;
            // double sum_s_minus_E = 0.0;
            

            // for (const auto& elem : list.getCache()) {
            //     int r_j = std::get<0>(elem);
            //     int RD_j = std::get<1>(elem);
            //     double gRD_j = 1/(sqrt((1+3*pow(q,2)*(pow(RD_j,2))/pow(pi, 2)))); 
            //     double E = 1/(1+pow(10,(-gRD_j)*(r-r_j)/400)); 

            //     double s;
            //     if (r_j == 1400) s = 1;
            //     else s = 0;

            //     sum += + pow(gRD_j, 2) * E * (1 - E); 
            //     sum_s_minus_E += gRD_j * (s - E);
            // }

            // double d_2 = pow(pow(q, 2) * sum, -1);
            // double new_r = r + q / ((1 / pow(RD, 2)) + (1 / d_2)) * sum_s_minus_E;
            // double new_RD = sqrt(pow((1 / pow(RD, 2)) + (1 / d_2), -1));


            // Perform calculations for win, loss, and draw
            // double g_opponent_RD = 1 / (sqrt(1 + 3 * pow(q, 2) * pow(opponent_RD, 2) / pow(pi, 2))); 
            // double E_opponent = 1 / (1 + pow(10, -g_opponent_RD * (r - opponent_rating) / 400)); 

            // double sum_opponent = sum + pow(g_opponent_RD, 2) * E_opponent * (1 - E_opponent);

            // double d_2_opponent = pow(pow(q, 2) * sum_opponent, -1);

            // auto calculate_new_rating = [&](double s) {
            //     double sum_s_minus_E_opponent = sum_s_minus_E + g_opponent_RD * (s - E_opponent);
            //     double new_r_opponent = r + q / ((1 / pow(RD, 2)) + (1 / d_2_opponent)) * sum_s_minus_E_opponent;
            //     return new_r_opponent;
            // };
            std::cout << "If you win: " << calculate_new_rating(1) << endl;
            std::cout << "If you lose: " << calculate_new_rating(0) << endl;
            std::cout << "If you draw: " << calculate_new_rating(0.5) << endl;
        };
        // void analyzeRisk();
        // void abort();

};





int main(){
    // Player user;
    // Player opponent;
    // cout << "Enter Player Rating: " << endl;
    // cin >> user.rating;
    // cout << "Enter Opponent Rating: " << endl;
    // cin >> oppoent.rating;
    // LRUCache list(10);
    // list.insert(std::make_tuple(1400, 30));
    // list.insert(std::make_tuple(1550, 100));
    // list.insert(std::make_tuple(1700, 300));
    // int r = 1500;
    // int RD = 200;
    // double q = 0.0057565;
    // double pi = 2*asin(1.0);
    // double gRD = 1/(sqrt((1+3*pow(q,2)*(pow(RD,2))/pow(pi, 2))));


    // double sum;
    // double sum_s_minus_E = 0.0;
    

    // for (const auto& elem : list.getCache()) {
    //     int r_j = std::get<0>(elem);
    //     int RD_j = std::get<1>(elem);
    //     double gRD_j = 1/(sqrt((1+3*pow(q,2)*(pow(RD_j,2))/pow(pi, 2)))); 
    //     double E = 1/(1+pow(10,(-gRD_j)*(r-r_j)/400)); 

    //     double s;
    //     if (r_j == 1400) s = 1;
    //     else s = 0;

    //     sum += + pow(gRD_j, 2) * E * (1 - E); 
    //     sum_s_minus_E += gRD_j * (s - E);
    // }

    // double d_2 = pow(pow(q, 2) * sum, -1);
    // double new_r = r + q / ((1 / pow(RD, 2)) + (1 / d_2)) * sum_s_minus_E;
    // double new_RD = sqrt(pow((1 / pow(RD, 2)) + (1 / d_2), -1));



    // cout << "d_2 = " << d_2 << endl;
    // cout << "new_r = " << new_r << endl;
    // cout << "new_RD = " << new_RD << endl;


    // // Request user input for the next opponent's rating and RD
    // int opponent_rating, opponent_RD;
    // cout << "Enter the opponent's rating: ";
    // cin >> opponent_rating;
    // cout << "Enter the opponent's RD: ";
    // cin >> opponent_RD;

   


    std::string username;
    std::cout << "Enter the Chess.com username: ";
    std::cin >> username;

    Player player("chess_bullet");
    player.username = username;
    player.stats("chess_bullet");

    std::string opponent;
    std::cout << "Enter the Opponent Chess.com username: ";
    std::cin >> opponent;

    Player opp("chess_bullet");
    opp.username = opponent;
    opp.stats("chess_bullet");

    Game game(player.username, opp.username, player.Rating, opp.Rating, player.RD, opp.RD);
    game.calculateRatingRes(player.Rating, opp.Rating, player.RD, opp.RD);

    return 0;
}