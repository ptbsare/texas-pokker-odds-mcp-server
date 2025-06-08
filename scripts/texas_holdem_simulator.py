
import random
from collections import Counter

# 定义牌面和花色
RANKS = '23456789TJQKA'
SUITS = 'hdsc' # hearts, diamonds, spades, clubs

def create_deck():
    return [r + s for r in RANKS for s in SUITS]

def get_hand_rank(hand):
    """
    评估一手牌的牌型和大小
    hand: 5张牌的列表，例如 ['As', 'Ks', 'Qs', 'Js', 'Ts']
    返回: (牌型等级, 相关牌面值)
    牌型等级:
        9: 同花顺 (Straight Flush)
        8: 四条 (Four of a Kind)
        7: 葫芦 (Full House)
        6: 同花 (Flush)
        5: 顺子 (Straight)
        4: 三条 (Three of a Kind)
        3: 两对 (Two Pair)
        2: 一对 (One Pair)
        1: 高牌 (High Card)
    相关牌面值用于区分相同牌型的大小
    """
    # 将牌面转换为数字，A为12，K为11，以此类推
    rank_values = sorted([RANKS.index(c[0]) for c in hand], reverse=True)
    suits = [c[1] for c in hand]

    # 检查同花
    is_flush = len(set(suits)) == 1

    # 检查顺子
    is_straight = False
    if len(set(rank_values)) == 5: # 确保没有重复牌面
        # 普通顺子
        if rank_values[0] - rank_values[-1] == 4:
            is_straight = True
        # A-5顺子 (A, 2, 3, 4, 5) 特殊处理，A作为最低牌
        elif rank_values == [12, 3, 2, 1, 0]: # A, 5, 4, 3, 2
            is_straight = True
            # 为了比较大小，将A-5顺子的A视为-1 (或更小的数字)以便正确排序
            # 但这里我们只关心最高牌，所以对于A-5顺子，最高牌是5 (index 3)
            # 在返回时，我们通常用最高牌来表示顺子，所以需要特殊处理A-5顺子的值
            # 如果是A-5顺子，其“最高牌”实际是5，而不是A
            # rank_values = [3, 2, 1, 0, -1] # 这样可以确保A-5顺子小于2-6顺子

    # 统计牌面出现次数
    counts = Counter(rank_values)
    # 按出现次数降序，然后按牌面值降序排序
    sorted_counts = sorted(counts.items(), key=lambda item: (item[1], item[0]), reverse=True)

    # 提取牌型信息
    if sorted_counts[0][1] == 4: # 四条
        quad_rank = sorted_counts[0][0]
        kicker = sorted_counts[1][0]
        return (8, quad_rank, kicker)
    elif sorted_counts[0][1] == 3 and sorted_counts[1][1] == 2: # 葫芦
        trip_rank = sorted_counts[0][0]
        pair_rank = sorted_counts[1][0]
        return (7, trip_rank, pair_rank)
    elif is_flush and is_straight: # 同花顺
        return (9, rank_values[0] if not (rank_values == [12, 3, 2, 1, 0]) else 3) # A-5顺子最高牌是5
    elif is_flush: # 同花
        return (6, *rank_values)
    elif is_straight: # 顺子
        return (5, rank_values[0] if not (rank_values == [12, 3, 2, 1, 0]) else 3) # A-5顺子最高牌是5
    elif sorted_counts[0][1] == 3: # 三条
        trip_rank = sorted_counts[0][0]
        kickers = sorted([item[0] for item in sorted_counts[1:]], reverse=True)
        return (4, trip_rank, *kickers)
    elif sorted_counts[0][1] == 2 and sorted_counts[1][1] == 2: # 两对
        pair1_rank = sorted_counts[0][0]
        pair2_rank = sorted_counts[1][0]
        kicker = sorted_counts[2][0]
        return (3, pair1_rank, pair2_rank, kicker)
    elif sorted_counts[0][1] == 2: # 一对
        pair_rank = sorted_counts[0][0]
        kickers = sorted([item[0] for item in sorted_counts[1:]], reverse=True)
        return (2, pair_rank, *kickers)
    else: # 高牌
        return (1, *rank_values)

def get_best_5_card_hand(seven_cards):
    """
    从7张牌中选出最佳的5张牌组合
    """
    from itertools import combinations
    best_rank = (0,) # 初始最低牌型
    for combo in combinations(seven_cards, 5):
        current_rank = get_hand_rank(list(combo))
        if current_rank > best_rank:
            best_rank = current_rank
    return best_rank

def simulate_texas_holdem(num_simulations=100000):
    aa_wins = 0
    kk_wins = 0
    ties = 0

    # 预设AA和KK的底牌
    # 实际花色不影响胜率，只要是两张同等级牌即可
    player_aa_hand = ['Ad', 'Ah']
    player_kk_hand = ['Kd', 'Kh']

    for _ in range(num_simulations):
        deck = create_deck()
        # 移除已发出的底牌
        for card in player_aa_hand + player_kk_hand:
            if card in deck: # 确保牌在牌组中才移除
                deck.remove(card)

        # 随机抽取5张公共牌
        community_cards = random.sample(deck, 5)

        # 组合玩家手牌和公共牌
        aa_total_cards = player_aa_hand + community_cards
        kk_total_cards = player_kk_hand + community_cards

        # 评估最佳牌型
        aa_best_hand_rank = get_best_5_card_hand(aa_total_cards)
        kk_best_hand_rank = get_best_5_card_hand(kk_total_cards)

        # 比较胜负
        if aa_best_hand_rank > kk_best_hand_rank:
            aa_wins += 1
        elif kk_best_hand_rank > aa_best_hand_rank:
            kk_wins += 1
        else:
            ties += 1

    total_results = aa_wins + kk_wins + ties
    aa_win_percentage = (aa_wins / total_results) * 100
    kk_win_percentage = (kk_wins / total_results) * 100
    tie_percentage = (ties / total_results) * 100

    print(f"模拟次数: {num_simulations}")
    print(f"AA 胜率: {aa_win_percentage:.2f}%")
    print(f"KK 胜率: {kk_win_percentage:.2f}%")
    print(f"平局率: {tie_percentage:.2f}%")

# 运行模拟
simulate_texas_holdem()
