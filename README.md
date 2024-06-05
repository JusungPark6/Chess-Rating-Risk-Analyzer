# Chess Rating Risk Analyzer
### Jusung Park
### Purpose:
---
This program utilizes the Glicko Rating System formula to predict the rating changes that would happen for each game of chess played on Chess.com. The intent is to maximize reward and minimize risk when playing high-speed chess games such as bullet or hyperbullet to decide if the opponent the Player is matched up with is worth playing or whether they should abort. 

### Glicko Rating System
---
[Article on Glicko System](./Glicko%20System.pdf)
The Glicko Rating System is a chess rating system used by Chess.com. It works by providing a more accurate measure of a player's skill by accounting for the rating deviation (RD) which indicates the reliability of a player's rating. The system adjusts a player's rating based on the results of games played and the ratings and RDs of the opponents.


### Things I've done
---
- installed libcurl4-openssl-dev for making HTTP requests.
- installed nlohmann/json for parsing JSON responses.
- 




### How to Use
---
1. **Input Player's Rating:**
   - The program prompts the user to input the player's current rating, rating deviation (RD), and volatility.
   
2. **Input Opponent's Details:**
   - The program then prompts the user to input the opponent's rating and RD.

3. **Calculate Potential Rating Changes:**
   - The program calculates and prints the potential rating changes for win, draw, and loss scenarios using the Glicko Rating System formulas.

### Explanation of the Code
#### Struct Definitions:
- **Player Struct:**
  - Contains the player's rating, rating deviation (RD), and volatility.
   ```cpp
    struct Player {
        double rating;
        double RD;
        double volatility;
    };
- **MatchResult Struct:**
  - Contains the opponent's rating, opponent's RD, and the match result score.
  - Contains the opponent's rating, opponent's RD, and the match result score.
    ```cpp
    struct MatchResult {
        double opponent_rating;
        double opponent_RD;
        double match_result;
    };
#### Functions:
- ***glickoExpectedScore Function:***
    -  Calculates the expected score for a player against an opponent using the Glicko formula.
    - ```cpp
        double glickoExpectedScore(double player_rating, double opponent_rating, double opponent_RD) {
            const double q = 0.0057565;  // constant
            double g_RD = 1.0 / sqrt(1.0 + 3.0 * pow(q, 2) * pow(opponent_RD, 2) / pow(M_PI, 2));
            double E = 1.0 / (1.0 + pow(10, -g_RD * (player_rating - opponent_rating) / 400.0));
            return E;
        }
- ***updateGlickoRating Function:***
    - Updates the player's rating and RD based on the match results using the Glicko rating system formulas.
        - Step 1: Update RD:
             ```cpp
                double update_RD(double RD, double c) {
                    return sqrt(pow(RD, 2) + pow(c, 2));
                }
        - Step 2: Calculate v (estimated variance):
             ```cpp
                 double calculate_v(double RD, double g_RD, double E) {
                    const double q = 0.0057565;  // constant
                    return 1.0 / (pow(q, 2) * pow(g_RD, 2) * E * (1 - E));
                }
        - Step 3: Calculate delta (rating change):
             ```cpp
                double calculate_delta(double RD_prime, double v, double g_RD, double S, double E) {
                    const double q = 0.0057565;  // constant
                    return (q / ((1.0 / pow(RD_prime, 2)) + (1.0 / v))) * g_RD * (S - E);
                }
    - ***calculateRatingChanges Function:***
        - Calculates the potential rating changes for win, draw, and loss scenarios.
        - Calls the updateGlickoRating function to calculate the new ratings for win, draw, and loss results.
        - Prints the potential rating changes.
         ```cpp
            void calculateRatingChanges(Player player, MatchResult opponent) {
                std::vector<std::string> results = {"win", "draw", "loss"};
                for (const auto& result : results) {
                    double S;
                    if (result == "win") {
                        S = 1;
                    } else if (result == "draw") {
                        S = 0.5;
                    } else {
                        S = 0;
                    }

                    double expected_score = glickoExpectedScore(player.rating, opponent.opponent_rating, opponent.opponent_RD);
                    double g_RD = 1.0 / sqrt(1.0 + 3.0 * pow(0.0057565, 2) * pow(opponent.opponent_RD, 2) / pow(M_PI, 2));
                    double v = calculate_v(opponent.opponent_RD, g_RD, expected_score);
                    double delta = calculate_delta(player.RD, v, g_RD, S, expected_score);
                    double new_rating = update_rating(player.rating, delta);
                    double new_RD = update_RD(player.RD, player.volatility);

                    std::cout << "After a " << result << ", the new rating would be: " << new_rating << " and the new RD would be: " << new_RD << std::endl;
                    }
                }
        
    - ***Main Function:***
        - Prompts the user to input the player's rating and the opponent's rating.
        - Calls the calculateRatingChanges function to calculate and print the rating changes for different outcomes.
         ```cpp
            int main() {
                double player_rating;
                double player_RD;
                double player_volatility;
                double opponent_rating;
                double opponent_RD;

                std::cout << "Enter player's rating: ";
                std::cin >> player_rating;
                std::cout << "Enter player's rating deviation (RD): ";
                std::cin >> player_RD;
                std::cout << "Enter player's volatility: ";
                std::cin >> player_volatility;
                std::cout << "Enter opponent's rating: ";
                std::cin >> opponent_rating;
                std::cout << "Enter opponent's rating deviation (RD): ";
                std::cin >> opponent_RD;

                Player player = {player_rating, player_RD, player_volatility};
                MatchResult opponent = {opponent_rating, opponent_RD, 0};  // Match result score is not used here

                calculateRatingChanges(player, opponent);

                return 0;
            }
            
### To Do:
- ***test it***
- Figure out how to get the RD value for player and opponent (some estimation calculator)
- add calculations for the program to output whether to play the game or abort
- add a more User-friendly and usable interface GUI
    - ideally a program that can read the screen of the game playing and instantly calculate risk without any user inputs
- add backtesting to make sure the Glick calculations align with the Chess.com calculations
