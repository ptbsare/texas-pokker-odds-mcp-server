import random
from collections import Counter
from typing import List, Dict, Any, Optional, Annotated
from pydantic import Field, ValidationError
import multiprocessing
import os
import sys

from mcp.server.fastmcp import FastMCP

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

def parse_card_string(card_string: str) -> List[str]:
    """
    解析牌面字符串，例如 "AsKd" -> ["As", "Kd"]
    """
    if len(card_string) % 2 != 0:
        raise ValueError("牌面字符串长度必须是偶数，每张牌由2个字符组成。")
    
    cards = []
    for i in range(0, len(card_string), 2):
        card = card_string[i:i+2]
        rank, suit = card[0].upper(), card[1].lower()
        if rank not in RANKS or suit not in SUITS:
            raise ValueError(f"无效的牌面: {card}。牌面应为 '2-9TJQKA'，花色为 'h,d,s,c'。")
        cards.append(rank + suit)
    return cards

def _simulate_single_process(player1_hand: List[str], player2_hand: List[str], initial_community_cards: List[str], num_simulations_per_process: int):
    """
    单个进程的模拟函数。
    """
    player1_wins = 0
    player2_wins = 0
    ties = 0

    all_known_cards = set(player1_hand + player2_hand + initial_community_cards)

    for _ in range(num_simulations_per_process):
        deck = create_deck()
        # 移除已知牌
        for card in all_known_cards:
            if card in deck:
                deck.remove(card)

        # 补充公共牌到5张
        remaining_community_cards_needed = 5 - len(initial_community_cards)
        if remaining_community_cards_needed > 0:
            simulated_community_cards = random.sample(deck, remaining_community_cards_needed)
        else:
            simulated_community_cards = []

        final_community_cards = initial_community_cards + simulated_community_cards

        # 组合玩家手牌和公共牌
        player1_total_cards = player1_hand + final_community_cards
        player2_total_cards = player2_hand + final_community_cards

        # 评估最佳牌型
        player1_best_hand_rank = get_best_5_card_hand(player1_total_cards)
        player2_best_hand_rank = get_best_5_card_hand(player2_total_cards)

        # 比较胜负
        if player1_best_hand_rank > player2_best_hand_rank:
            player1_wins += 1
        elif player2_best_hand_rank > player1_best_hand_rank:
            player2_wins += 1
        else:
            ties += 1
    
    return player1_wins, player2_wins, ties

def calculate_odds_core(player1_hand_str: str, player2_hand_str: str, community_cards_str: Optional[str] = None, num_simulations: int = 10000):
    """
    核心计算函数，不直接作为MCP工具，方便多进程调用。
    """
    try:
        player1_hand = parse_card_string(player1_hand_str)
        player2_hand = parse_card_string(player2_hand_str)
        
        if len(player1_hand) != 2 or len(player2_hand) != 2:
            raise ValueError("每手牌必须包含2张牌。")
        
        initial_community_cards = []
        if community_cards_str:
            initial_community_cards = parse_card_string(community_cards_str)
            if not (3 <= len(initial_community_cards) <= 4):
                raise ValueError("公共牌必须是3张（翻牌）或4张（转牌）。")

        all_known_cards = set(player1_hand + player2_hand + initial_community_cards)
        if len(all_known_cards) != len(player1_hand) + len(player2_hand) + len(initial_community_cards):
            raise ValueError("手牌和公共牌中存在重复的牌。")

    except ValueError as e:
        return {"error": str(e)}

    # 确定使用的进程数
    num_processes = os.cpu_count() or 1 # 获取CPU核心数，如果获取不到则默认为1
    if num_simulations < num_processes:
        num_processes = 1 # 如果模拟次数小于核心数，则只用一个进程

    # 计算每个进程需要执行的模拟次数
    num_simulations_per_process = num_simulations // num_processes
    remaining_simulations = num_simulations % num_processes

    total_player1_wins = 0
    total_player2_wins = 0
    total_ties = 0

    # 使用 multiprocessing.Pool 进行并行计算
    with multiprocessing.Pool(processes=num_processes) as pool:
        results = []
        for i in range(num_processes):
            sims_for_this_process = num_simulations_per_process
            if i < remaining_simulations: # 将余数分配给前几个进程
                sims_for_this_process += 1
            
            if sims_for_this_process > 0:
                results.append(pool.apply_async(_simulate_single_process, (player1_hand, player2_hand, initial_community_cards, sims_for_this_process)))
        
        for res in results:
            p1_wins, p2_wins, ties_count = res.get()
            total_player1_wins += p1_wins
            total_player2_wins += p2_wins
            total_ties += ties_count

    total_results = total_player1_wins + total_player2_wins + total_ties
    if total_results == 0: # 避免除以零
        return {"error": "模拟次数为0，无法计算概率。"}

    player1_win_percentage = (total_player1_wins / total_results) * 100
    player2_win_percentage = (total_player2_wins / total_results) * 100
    tie_percentage = (total_ties / total_results) * 100

    return {
        "player1_hand": player1_hand_str,
        "player2_hand": player2_hand_str,
        "community_cards": community_cards_str if community_cards_str else "None",
        "num_simulations": num_simulations,
        "player1_win_percentage": f"{player1_win_percentage:.2f}%",
        "player2_win_percentage": f"{player2_win_percentage:.2f}%",
        "tie_percentage": f"{tie_percentage:.2f}%"
    }

# 创建 MCP 服务器实例
mcp = FastMCP("texas-poker-odds-generator")

# 定义 calculate_poker_odds 工具
@mcp.tool(
    description="计算德州扑克两手牌在给定公共牌（可选）情况下的胜负平概率。模拟次数越多，结果越精确。"
)
def calculate_poker_odds(
    player1_hand: Annotated[str, Field(description="玩家1的两张手牌，例如 'AsKd' (A黑桃, K方块)。牌面: 2-9, T(10), J, Q, K, A。花色: h(红心), d(方块), s(黑桃), c(梅花)。")],
    player2_hand: Annotated[str, Field(description="玩家2的两张手牌，例如 '7c8h' (7梅花, 8红心)。格式同玩家1手牌。")],
    community_cards: Annotated[Optional[str], Field(None, description="可选的公共牌。可以是3张（翻牌，例如 '2h3d4s'）或4张（转牌，例如 '2h3d4s5c'）。")] = None,
    num_simulations: Annotated[int, Field(10000, description="模拟次数。默认为10000次，建议至少10000次以获得较准确结果。", ge=100)] = 10000
) -> Dict[str, Any]:
    """
    计算德州扑克两手牌的胜负平概率。
    """
    return calculate_odds_core(player1_hand, player2_hand, community_cards, num_simulations)

if __name__ == "__main__":
    multiprocessing.set_start_method('spawn', force=True)
    multiprocessing.freeze_support()
    mcp.run(transport='stdio')