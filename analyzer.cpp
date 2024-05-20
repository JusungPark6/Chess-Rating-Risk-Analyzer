#include <iostream>
#include <cmath>
#include <vector>

struct Player {
    double rating;      // Current rating of the player
    double rd;          // Rating deviation (measure of uncertainty)
    double volatility;  // Measure of expected fluctuation in rating
};

struct MatchResult {
    double opponent_rating;
    double opponent_rd;
    double score;  // 1 for win, 0.5 for draw, 0 for loss
};

// Function to calculate the expected score
double glickoExpectedScore(double rating, double opponent_rating, double opponent_rd) {
    const double q = 0.0057565;  // Scaling factor
    double g = 1.0 / std::sqrt(1 + 3 * q * q * opponent_rd * opponent_rd / (M_PI * M_PI));
    double E = 1.0 / (1.0 + std::pow(10, -g * (rating - opponent_rating) / 400.0));
    return E;
}

// Function to calculate the g(RD) function
double gFunction(double rd) {
    const double q = 0.0057565;  // Scaling factor
    return 1.0 / std::sqrt(1 + 3 * q * q * rd * rd / (M_PI * M_PI));
}

// Function to update the player's rating and RD based on match results
Player updateGlickoRating(Player player, const std::vector<MatchResult>& results, double c = 1.2) {
    const double q = 0.0057565;  // Scaling factor

    // Step 1: Update RD (Rating Deviation)
    double rd_prime = std::sqrt(player.rd * player.rd + c * c);

    // Initialize variables for calculating new rating
    double v_inv = 0.0;  // Inverse of the estimated variance
    double delta = 0.0;  // Intermediate variable for new rating calculation

    for (const auto& result : results) {
        double E = glickoExpectedScore(player.rating, result.opponent_rating, result.opponent_rd);
        double g = gFunction(result.opponent_rd);

        // Accumulate v_inv (sum of g^2 * E * (1 - E))
        v_inv += g * g * E * (1 - E);
        // Accumulate delta (sum of g * (S - E))
        delta += g * (result.score - E);
    }

    // Calculate v (estimated variance)
    double v = 1.0 / (q * q * v_inv);
    // Calculate new rating
    delta = (q / ((1 / (rd_prime * rd_prime)) + (1 / v))) * delta;
    player.rating += delta;

    // Step 2: Update RD
    player.rd = std::sqrt(1 / ((1 / (rd_prime * rd_prime)) + (1 / v)));

    return player;
}

void calculateRatingChanges(double player_rating, double opponent_rating, double player_rd = 200, double opponent_rd = 150) {
    Player player = {player_rating, player_rd, 0.06};  // Initial player with rating and RD
    Player win_player = player;
    Player draw_player = player;
    Player loss_player = player;

    std::vector<MatchResult> win_result = {{opponent_rating, opponent_rd, 1.0}};
    std::vector<MatchResult> draw_result = {{opponent_rating, opponent_rd, 0.5}};
    std::vector<MatchResult> loss_result = {{opponent_rating, opponent_rd, 0.0}};

    // Calculate new ratings for win, draw, and loss scenarios
    win_player = updateGlickoRating(win_player, win_result);
    draw_player = updateGlickoRating(draw_player, draw_result);
    loss_player = updateGlickoRating(loss_player, loss_result);

    // Calculate potential gains and losses
    double gain_if_win = win_player.rating - player.rating;
    double gain_if_draw = draw_player.rating - player.rating;
    double loss_if_lose = player.rating - loss_player.rating;

    // Print the potential rating changes
    std::cout << "Potential gain if win: " << gain_if_win << std::endl;
    std::cout << "Potential gain if draw: " << gain_if_draw << std::endl;
    std::cout << "Potential loss if lose: " << loss_if_lose << std::endl;
}

int main() {
    double player_rating, opponent_rating;
    std::cout << "Enter player's rating: ";
    std::cin >> player_rating;
    std::cout << "Enter player's rating deviation (RD): ";
    double player_rd;
    std::cin >> player_rd;
    std::cout << "Enter player's volatility: ";
    double player_volatility;
    std::cin >> player_volatility;
    std::cout << "Enter opponent's rating: ";
    std::cin >> opponent_rating;
    std::cout << "Enter opponent's rating deviation (RD): ";
    double opponent_rd;
    std::cin >> opponent_rd;

    Player player = {player_rating, player_rd, player_volatility};
    MatchResult opponent = {opponent_rating, opponent_rd, 0};  // Match result score is not used here

    calculateRatingChanges(player.rating, opponent.rating, player.rd, opponent.rd);

    return 0;
}
