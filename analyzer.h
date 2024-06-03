/*
Rating Deviation (RD):
    - changes from both game outcomes and passage of time when not playing
    - Handles uncertainty of a player's true skill as the more time they spend not playing, 
        they might be better or worse than what their rating indicates

95% Confidence Interval:
    - The lowest value in the interval is the player's rating minus twice the player's RD.
    - The highest value in the interval is the player's rating plus twice the player's RD.
    - lower the RD (more frequent playing), the smaller the interval.   
Rating Period:
    - Average 5-10 games in the rating period, but dependent on adminstrator

*/

/*
Glicko System Formula:
    - Step 1:
        - Determine the rating and RD for each player at the onset of the rating period.
        - RD should never drop below 30
        - calculate new RD from old RD:
            RD = min(sqrt(oldRD^2+c^2(t)), 350)
            t = number of rating periods since last competition (default to 1 if recent)
            c = a constant that governs increase in uncertainty over time.
    - Step 2: 
        - r' and RD': post-period rating and rating deviation for a player
            - r' = r + q/(1/RD^2 + 1/d^2) \sum(gRD_j)(s_j - E(s | r, r_j, RD_j))
            - RD' = sqrt((1/RD^2 + 1/d^2)^-1)
        - where:
            - q = log10/400 = 0.0057565
            - g(RD) = 1/(sqrt(1+3q^2(RD^2)/pi^2))
            - E(s | r, r_j, RD_j)) = 1/(1+10^(-g(RD_j)(r-r_j)/400))
            - d^2 = (q^2 \sum(g(RD_j))^2(E(s | r, r_j, RD_j))(1-E(s | r, r_j, RD_j)))^-1

*/