#include <iostream>
#include <cmath>
#include <vector>
#include "LRUCache.h"
#include <tuple>

using namespace std;
using std::tuple;

class Player {
    int rating, RD; // rating deviation, low RD means more frequent playing
    char username[100]; //player chess.com username
    // LRUCache oppList(10);
    public:
        void calculateRD(char username, int rating);
        void calculateRatingRes(char username, int rating);
        void analyzeRisk(char username, int rating);
        bool abort();
    
    private:
        void fetchPlayerData(char username);
        void fetchRecentOpps(char username);
};

// Player :: calculateRD(char username, int rating){
//     double q = 0.0057565;
//     double pi = 2*asin(1.0);
//     double gRD = 1/(sqrt((1+3*pow(q,2)*(pow(RD,2))/pow(pi, 2))));
//     double r;
//     //opponent
//     double RD_j;
//     double r_j;
//     double E = 1/(1+pow(10,(-g*(RD_j)*(r-r_j)/400)));


// }   
// Player :: calculateRatingRes(){

// }
// Player :: analyzeRisk(){

// }

class Game {
    int player1, player2;
    public:
        void calculateRatingRes();
        void analyzeRisk();
        void abort();

};


int main(){
    // Player user;
    // Player opponent;
    // cout << "Enter Player Rating: " << endl;
    // cin >> user.rating;
    // cout << "Enter Opponent Rating: " << endl;
    // cin >> oppoent.rating;
    LRUCache list(10);
    list.insert(std::make_tuple(1400, 30));
    list.insert(std::make_tuple(1550, 100));
    list.insert(std::make_tuple(1700, 300));
    int r = 1500;
    int RD = 200;
    double q = 0.0057565;
    double pi = 2*asin(1.0);
    double gRD = 1/(sqrt((1+3*pow(q,2)*(pow(RD,2))/pow(pi, 2))));


    double sum;
    double sum_s_minus_E = 0.0;
    

    for (const auto& elem : list.getCache()) {
        int r_j = std::get<0>(elem);
        int RD_j = std::get<1>(elem);
        double gRD_j = 1/(sqrt((1+3*pow(q,2)*(pow(RD_j,2))/pow(pi, 2)))); 
        double E = 1/(1+pow(10,(-gRD_j)*(r-r_j)/400)); 

        double s;
        if (r_j == 1400) s = 1;
        else s = 0;

        sum += + pow(gRD_j, 2) * E * (1 - E); 
        sum_s_minus_E += gRD_j * (s - E);
    }

    double d_2 = pow(pow(q, 2) * sum, -1);
    double new_r = r + q / ((1 / pow(RD, 2)) + (1 / d_2)) * sum_s_minus_E;
    double new_RD = sqrt(pow((1 / pow(RD, 2)) + (1 / d_2), -1));



    cout << "d_2 = " << d_2 << endl;
    cout << "new_r = " << new_r << endl;
    cout << "new_RD = " << new_RD << endl;


    // Request user input for the next opponent's rating and RD
    int opponent_rating, opponent_RD;
    cout << "Enter the opponent's rating: ";
    cin >> opponent_rating;
    cout << "Enter the opponent's RD: ";
    cin >> opponent_RD;

    // Perform calculations for win, loss, and draw
    double g_opponent_RD = 1 / (sqrt(1 + 3 * pow(q, 2) * pow(opponent_RD, 2) / pow(pi, 2))); 
    double E_opponent = 1 / (1 + pow(10, -g_opponent_RD * (r - opponent_rating) / 400)); 

    double sum_opponent = sum + pow(g_opponent_RD, 2) * E_opponent * (1 - E_opponent);

    double d_2_opponent = pow(pow(q, 2) * sum_opponent, -1);

    auto calculate_new_rating = [&](double s) {
        double sum_s_minus_E_opponent = sum_s_minus_E + g_opponent_RD * (s - E_opponent);
        double new_r_opponent = r + q / ((1 / pow(RD, 2)) + (1 / d_2_opponent)) * sum_s_minus_E_opponent;
        return new_r_opponent;
    };

    cout << "If you win: " << calculate_new_rating(1) << endl;
    cout << "If you lose: " << calculate_new_rating(0) << endl;
    cout << "If you draw: " << calculate_new_rating(0.5) << endl;


    //Make API Calls here for user and opponent to calculate RD

    // //Run calculations for each player
    // user.calculateRD();
    // opponent.calculateRD();
    // calculateRatingRes(char username, int rating);
    // analyzeRisk(char username, int rating);
    // abort();


    return 0;
}