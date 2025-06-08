#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Card representation: rank (0-12 for 2-A) and suit (0-3 for clubs, diamonds, hearts, spades)
typedef struct {
    int rank;
    int suit;
} Card;

// For sorting cards
int compare_cards(const void *a, const void *b) {
    Card *cardA = (Card *)a;
    Card *cardB = (Card *)b;
    return cardB->rank - cardA->rank; // Sort in descending order of rank
}

// Function to convert char rank to int rank
int rank_to_int(char r) {
    if (r >= '2' && r <= '9') return r - '2';
    if (r == 'T') return 8;
    if (r == 'J') return 9;
    if (r == 'Q') return 10;
    if (r == 'K') return 11;
    if (r == 'A') return 12;
    return -1; // Error
}

// Function to convert char suit to int suit
int suit_to_int(char s) {
    if (s == 'c') return 0;
    if (s == 'd') return 1;
    if (s == 'h') return 2;
    if (s == 's') return 3;
    return -1; // Error
}

// Function to convert int rank to char rank
char int_to_rank(int r) {
    if (r >= 0 && r <= 7) return r + '2';
    if (r == 8) return 'T';
    if (r == 9) return 'J';
    if (r == 10) return 'Q';
    if (r == 11) return 'K';
    if (r == 12) return 'A';
    return '?';
}

// Function to convert int suit to char suit
char int_to_suit(int s) {
    if (s == 0) return 'c';
    if (s == 1) return 'd';
    if (s == 2) return 'h';
    if (s == 3) return 's';
    return '?';
}

// Function to evaluate a 5-card poker hand
// Returns an integer representing hand strength (higher is better)
// Format: HandType * 10^10 + HighCardRank * 10^8 + Kicker1 * 10^6 + Kicker2 * 10^4 + ...
long long evaluate_5_card_hand(Card hand[5]) {
    qsort(hand, 5, sizeof(Card), compare_cards);

    // Check for flush
    bool is_flush = true;
    for (int i = 1; i < 5; i++) {
        if (hand[i].suit != hand[0].suit) {
            is_flush = false;
            break;
        }
    }

    // Check for straight
    bool is_straight = false;
    // Special case for A-5 straight (A, 2, 3, 4, 5)
    if (hand[0].rank == 12 && hand[1].rank == 3 && hand[2].rank == 2 && hand[3].rank == 1 && hand[4].rank == 0) {
        is_straight = true;
        // For A-5 straight, the highest card for value is '5' (rank 3)
        return (is_flush ? 9LL : 5LL) * 10000000000LL + 3LL; 
    } else {
        for (int i = 1; i < 5; i++) {
            if (hand[i].rank != hand[i-1].rank - 1) {
                is_straight = false;
                break;
            }
        }
    }

    // Straight Flush
    if (is_straight && is_flush) {
        return 9LL * 10000000000LL + hand[0].rank; // Rank based on highest card
    }

    // Count ranks for pairs, trips, quads
    int rank_counts[13] = {0}; // 0-12 for 2-A
    for (int i = 0; i < 5; i++) {
        rank_counts[hand[i].rank]++;
    }

    int pairs = 0;
    int trips = 0;
    int quads = 0;
    int pair_ranks[2]; // Store ranks of pairs
    int trip_rank = -1;
    int quad_rank = -1;

    for (int i = 12; i >= 0; i--) { // Iterate from A down to 2
        if (rank_counts[i] == 4) {
            quads = 1;
            quad_rank = i;
        } else if (rank_counts[i] == 3) {
            trips = 1;
            trip_rank = i;
        } else if (rank_counts[i] == 2) {
            pair_ranks[pairs++] = i;
        }
    }

    // Four of a Kind
    if (quads) {
        long long score = 8LL * 10000000000LL + quad_rank * 100000000LL;
        for (int i = 0; i < 5; i++) { // Find the kicker
            if (hand[i].rank != quad_rank) {
                score += hand[i].rank; // Only one kicker for quads
                break;
            }
        }
        return score;
    }

    // Full House
    if (trips && pairs == 1) {
        return 7LL * 10000000000LL + trip_rank * 100000000LL + pair_ranks[0] * 1000000LL;
    }

    // Flush (already checked straight flush)
    if (is_flush) {
        return 6LL * 10000000000LL + hand[0].rank * 100000000LL + hand[1].rank * 1000000LL + hand[2].rank * 10000LL + hand[3].rank * 100LL + hand[4].rank;
    }

    // Straight (already checked straight flush)
    if (is_straight) {
        return 5LL * 10000000000LL + hand[0].rank; // Rank based on highest card
    }

    // Three of a Kind
    if (trips) {
        long long score = 4LL * 10000000000LL + trip_rank * 100000000LL;
        int kicker_idx = 0;
        for (int i = 0; i < 5; i++) {
            if (hand[i].rank != trip_rank) {
                score += hand[i].rank * (kicker_idx == 0 ? 1000000LL : 10000LL);
                kicker_idx++;
            }
        }
        return score;
    }

    // Two Pair
    if (pairs == 2) {
        long long score = 3LL * 10000000000LL + pair_ranks[0] * 100000000LL + pair_ranks[1] * 1000000LL;
        for (int i = 0; i < 5; i++) { // Find the kicker
            if (hand[i].rank != pair_ranks[0] && hand[i].rank != pair_ranks[1]) {
                score += hand[i].rank;
                break;
            }
        }
        return score;
    }

    // One Pair
    if (pairs == 1) {
        long long score = 2LL * 10000000000LL + pair_ranks[0] * 100000000LL;
        int kicker_idx = 0;
        for (int i = 0; i < 5; i++) {
            if (hand[i].rank != pair_ranks[0]) {
                score += hand[i].rank * (kicker_idx == 0 ? 1000000LL : (kicker_idx == 1 ? 10000LL : 100LL));
                kicker_idx++;
            }
        }
        return score;
    }

    // High Card
    return 1LL * 10000000000LL + hand[0].rank * 100000000LL + hand[1].rank * 1000000LL + hand[2].rank * 10000LL + hand[3].rank * 100LL + hand[4].rank;
}

// Function to find the best 5-card hand from 7 cards
long long get_best_7_card_hand(Card all_cards[7]) {
    long long best_score = 0;
    Card temp_hand[5];

    // Iterate through all combinations of 5 cards from 7
    // C(7,5) = 21 combinations
    int indices[5];
    for (indices[0] = 0; indices[0] < 7; indices[0]++) {
        for (indices[1] = indices[0] + 1; indices[1] < 7; indices[1]++) {
            for (indices[2] = indices[1] + 1; indices[2] < 7; indices[2]++) {
                for (indices[3] = indices[2] + 1; indices[3] < 7; indices[3]++) {
                    for (indices[4] = indices[3] + 1; indices[4] < 7; indices[4]++) {
                        temp_hand[0] = all_cards[indices[0]];
                        temp_hand[1] = all_cards[indices[1]];
                        temp_hand[2] = all_cards[indices[2]];
                        temp_hand[3] = all_cards[indices[3]];
                        temp_hand[4] = all_cards[indices[4]];
                        long long current_score = evaluate_5_card_hand(temp_hand);
                        if (current_score > best_score) {
                            best_score = current_score;
                        }
                    }
                }
            }
        }
    }
    return best_score;
}

// Function to create a shuffled deck
void create_shuffled_deck(Card deck[52]) {
    int k = 0;
    for (int s = 0; s < 4; s++) { // Suits
        for (int r = 0; r < 13; r++) { // Ranks
            deck[k].rank = r;
            deck[k].suit = s;
            k++;
        }
    }
    // Shuffle the deck (Fisher-Yates shuffle)
    for (int i = 51; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

// Function to parse user input for a hand
bool parse_hand(const char* input, Card* card1, Card* card2) {
    if (strlen(input) != 4) {
        fprintf(stderr, "Error: Hand input format is incorrect. Use format like 'AhKs'.\n");
        return false;
    }
    card1->rank = rank_to_int(input[0]);
    card1->suit = suit_to_int(input[1]);
    card2->rank = rank_to_int(input[2]);
    card2->suit = suit_to_int(input[3]);

    if (card1->rank == -1 || card1->suit == -1 || card2->rank == -1 || card2->suit == -1) {
        fprintf(stderr, "Error: Invalid rank or suit. Ranks: 2-9, T, J, Q, K, A; Suits: h, d, s, c.\n");
        return false;
    }
    if (card1->rank == card2->rank && card1->suit == card2->suit) {
        fprintf(stderr, "Error: Two hole cards cannot be identical.\n");
        return false;
    }
    return true;
}

// Function to parse community cards
bool parse_community_cards(const char* input, Card* community_cards_arr, int* num_parsed) {
    size_t len = strlen(input);
    if (len != 6 && len != 8) {
        fprintf(stderr, "Error: Community cards format is incorrect. Use 3 cards (e.g., 'AhKhJh') or 4 cards (e.g., 'AhKhJhTh').\n");
        return false;
    }

    *num_parsed = len / 2;
    for (int i = 0; i < *num_parsed; i++) {
        community_cards_arr[i].rank = rank_to_int(input[i*2]);
        community_cards_arr[i].suit = suit_to_int(input[i*2+1]);

        if (community_cards_arr[i].rank == -1 || community_cards_arr[i].suit == -1) {
            fprintf(stderr, "Error: Invalid rank or suit in community cards. Ranks: 2-9, T, J, Q, K, A; Suits: h, d, s, c.\n");
            return false;
        }
        // Check for duplicates within community cards themselves
        for (int j = 0; j < i; j++) {
            if (community_cards_arr[i].rank == community_cards_arr[j].rank && 
                community_cards_arr[i].suit == community_cards_arr[j].suit) {
                fprintf(stderr, "Error: Duplicate cards found in community cards.\n");
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    srand(time(NULL)); // Initialize random seed

    int num_simulations = 100000; // Default number of simulations
    Card player1_hole[2];
    Card player2_hole[2];
    Card community_cards[5]; // Max 5 community cards
    int num_community_cards_provided = 0;

    // Basic argument count check
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <player1_hand> <player2_hand> [-s <num_simulations>] [community_cards]\n", argv[0]);
        fprintf(stderr, "Example: %s AdAh KdKh\n", argv[0]);
        fprintf(stderr, "Example with flop: %s AdAh KdKh 2s3d4c\n", argv[0]);
        fprintf(stderr, "Example with turn: %s AdAh KdKh 2s3d4c5h\n", argv[0]);
        fprintf(stderr, "Example with custom simulations: %s AdAh KdKh -s 500000\n", argv[0]);
        fprintf(stderr, "Example with custom simulations and flop: %s AdAh KdKh -s 200000 2s3d4c\n", argv[0]);
        return 1;
    }

    // Parse player hands
    if (!parse_hand(argv[1], &player1_hole[0], &player1_hole[1]) ||
        !parse_hand(argv[2], &player2_hole[0], &player2_hole[1])) {
        return 1; // Exit on error
    }

    // Check for duplicate hole cards between players
    if ((player1_hole[0].rank == player2_hole[0].rank && player1_hole[0].suit == player2_hole[0].suit) ||
        (player1_hole[0].rank == player2_hole[1].rank && player1_hole[0].suit == player2_hole[1].suit) ||
        (player1_hole[1].rank == player2_hole[0].rank && player1_hole[1].suit == player2_hole[0].suit) ||
        (player1_hole[1].rank == player2_hole[1].rank && player1_hole[1].suit == player2_hole[1].suit)) {
        fprintf(stderr, "Error: Duplicate cards found in players' hands.\n");
        return 1;
    }

    // Iterate through remaining arguments to find -s and community cards
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) {
                num_simulations = atoi(argv[i+1]);
                if (num_simulations <= 0) {
                    fprintf(stderr, "Error: Number of simulations must be a positive integer.\n");
                    return 1;
                }
                i++; // Skip the next argument as it's the simulation count
            } else {
                fprintf(stderr, "Error: -s requires a number of simulations.\n");
                return 1;
            }
        } else {
            // Assume it's community cards if not -s and it looks like cards
            // Check if it's already parsed to avoid overwriting
            if (num_community_cards_provided > 0) {
                fprintf(stderr, "Error: Only one set of community cards can be provided.\n");
                return 1;
            }
            // Simple check if it might be a card string (length 6 or 8)
            if (strlen(argv[i]) == 6 || strlen(argv[i]) == 8) {
                if (!parse_community_cards(argv[i], community_cards, &num_community_cards_provided)) {
                    return 1; // Exit on error
                }
                // Check for duplicates between hole cards and community cards
                Card all_hole_cards[4] = {player1_hole[0], player1_hole[1], player2_hole[0], player2_hole[1]};
                for (int k = 0; k < num_community_cards_provided; k++) {
                    for (int j = 0; j < 4; j++) {
                        if (community_cards[k].rank == all_hole_cards[j].rank &&
                            community_cards[k].suit == all_hole_cards[j].suit) {
                            fprintf(stderr, "Error: Community card duplicates a hole card.\n");
                            return 1;
                        }
                    }
                }
            } else {
                fprintf(stderr, "Error: Unrecognized argument: %s\n", argv[i]);
                return 1;
            }
        }
    }

    int player1_wins = 0;
    int player2_wins = 0;
    int ties = 0;

    for (int i = 0; i < num_simulations; i++) {
        Card deck[52];
        create_shuffled_deck(deck);

        // Remove hole cards and provided community cards from the deck
        Card current_deck[52];
        int current_deck_size = 0;
        for (int j = 0; j < 52; j++) {
            bool is_excluded_card = false;
            // Check if current deck card is one of the players' hole cards
            if ((deck[j].rank == player1_hole[0].rank && deck[j].suit == player1_hole[0].suit) ||
                (deck[j].rank == player1_hole[1].rank && deck[j].suit == player1_hole[1].suit) ||
                (deck[j].rank == player2_hole[0].rank && deck[j].suit == player2_hole[0].suit) ||
                (deck[j].rank == player2_hole[1].rank && deck[j].suit == player2_hole[1].suit)) {
                is_excluded_card = true;
            }
            // Check if current deck card is one of the provided community cards
            for (int k = 0; k < num_community_cards_provided; k++) {
                if (deck[j].rank == community_cards[k].rank && deck[j].suit == community_cards[k].suit) {
                    is_excluded_card = true;
                    break;
                }
            }

            if (!is_excluded_card) {
                current_deck[current_deck_size++] = deck[j];
            }
        }

        // Draw remaining community cards
        // The first `num_community_cards_provided` slots of `community_cards` are already filled
        for (int j = num_community_cards_provided; j < 5; j++) {
            int rand_idx = rand() % current_deck_size;
            community_cards[j] = current_deck[rand_idx];
            // Remove drawn card from current_deck by swapping with last and decrementing size
            current_deck[rand_idx] = current_deck[current_deck_size - 1];
            current_deck_size--;
        }

        // Combine hole cards and community cards for each player
        Card player1_all_cards[7];
        player1_all_cards[0] = player1_hole[0];
        player1_all_cards[1] = player1_hole[1];
        for (int j = 0; j < 5; j++) {
            player1_all_cards[j+2] = community_cards[j];
        }

        Card player2_all_cards[7];
        player2_all_cards[0] = player2_hole[0];
        player2_all_cards[1] = player2_hole[1];
        for (int j = 0; j < 5; j++) {
            player2_all_cards[j+2] = community_cards[j];
        }

        // Evaluate best 5-card hand for each player
        long long score1 = get_best_7_card_hand(player1_all_cards);
        long long score2 = get_best_7_card_hand(player2_all_cards);

        if (score1 > score2) {
            player1_wins++;
        } else if (score2 > score1) {
            player2_wins++;
        } else {
            ties++;
        }
    }

    printf("\nSimulations: %d\n", num_simulations);
    printf("Player 1 (%c%c %c%c) win rate: %.2f%%\n",
           int_to_rank(player1_hole[0].rank), int_to_suit(player1_hole[0].suit),
           int_to_rank(player1_hole[1].rank), int_to_suit(player1_hole[1].suit),
           (double)player1_wins / num_simulations * 100.0);
    printf("Player 2 (%c%c %c%c) win rate: %.2f%%\n",
           int_to_rank(player2_hole[0].rank), int_to_suit(player2_hole[0].suit),
           int_to_rank(player2_hole[1].rank), int_to_suit(player2_hole[1].suit),
           (double)player2_wins / num_simulations * 100.0);
    
    if (num_community_cards_provided > 0) {
        printf("Community cards: ");
        for (int j = 0; j < num_community_cards_provided; j++) {
            printf("%c%c ", int_to_rank(community_cards[j].rank), int_to_suit(community_cards[j].suit));
        }
        printf("\n");
    }

    printf("Tie rate: %.2f%%\n", (double)ties / num_simulations * 100.0);

    return 0;
}
