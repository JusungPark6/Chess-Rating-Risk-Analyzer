#include <iostream>
using namespace std;

class Player {
    int rating, RD; // rating deviation, low RD means more frequent playing
    char username[100]; //player chess.com username
    public:
        void calculateRD(char username, int rating);
        void calculateRatingRes(char username, int rating);
        void analyzeRisk(char username, int rating);
        bool abort();
    
    private:
        void fetchPlayerData(char username);
}

Player :: calculateRD(char username, int rating){

}
Player :: calculateRatingRes(){

}
Player :: analyzeRisk(){

}

class Game {
    int player1, player2;
    public:
        void calculateRatingRes();
        void analyzeRisk();
        void abort();

}


int main(){
    Player user;
    Player opponent;
    cout << "Enter Player Rating: " << endl;
    cin >> user.rating;
    cout << "Enter Opponent Rating: " << endl;
    cin >> oppoent.rating;

    //Make API Calls here for user and opponent to calculate RD

    //Run calculations for each player
    user.calculateRD();
    opponent.calculateRD();
    calculateRatingRes(char username, int rating);
    analyzeRisk(char username, int rating);
    abort();


    return 0;
}