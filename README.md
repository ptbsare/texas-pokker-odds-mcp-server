# Texas Poker Odds MCP Server
[简体中文 README](README_CN.md)

This project provides a tool to calculate the win, loss, and tie probabilities for multiple hands in Texas Hold'em poker, given optional community cards, and encapsulates it as an MCP (Model Context Protocol) server.

**Key Highlights:**

*   **Accurate Calculation**: Provides win, loss, and tie probabilities for multiple hands under different community card scenarios through extensive simulations, helping players make more informed decisions.
*   **Flexible Input**: Supports inputting multiple hole card sets (for different players), optional 3 (flop) or 4 (turn) community cards, and specifying the number of simulations.
*   **Multi-core Optimization**: Utilizes multi-core CPUs for parallel computation, significantly increasing simulation speed, especially for a large number of simulations.
*   **MCP Server Integration**: Encapsulates the probability calculation functionality as an MCP tool, allowing other AI agents or systems to call it via the MCP protocol.

## How to Use

### 1. Environment Setup

Ensure your system has Python 3.10 or higher installed.

It is recommended to use `uv` for dependency management:

```bash
# Install uv (if not already installed)
curl -LsSf https://astral.sh/uv/install.sh | sh

# Navigate to the project directory
cd texas-pokker-odds-mcp-server

# Install dependencies using uv
uv sync
```

### 2. Usage as an MCP Server

You can run this project as an MCP server, allowing other AI agents or systems to call the `calculate_poker_odds` tool via the MCP protocol.

```bash
uv run server.py
```

Once the server starts, it will listen for MCP client requests.

### 3. Claude MCP Server Configuration Example

Below is an example `mcp_server.json` configuration file to connect this MCP server to Claude:

```json
{
  "mcp_servers": [
    {
      "name": "texas-poker-odds-generator",
      "type": "stdio",
      "command": ["uv", "--directory", "/space/texas-pokker-odds-mcp-server/", "run", "server.py"],
      "tools": [
        {
          "name": "calculate_poker_odds",
          "description": "计算德州扑克多手牌在给定公共牌（可选）情况下的胜负平概率。模拟次数越多，结果越精确。",
          "input_schema": {
            "type": "object",
            "properties": {
              "player_hands": {
                "type": "array",
                "items": {
                  "type": "string"
                },
                "description": "一个包含所有玩家手牌字符串的列表。每手牌由两张牌组成，例如 ['AsKd', '7c8h']。牌面: 2-9, T(10), J, Q, K, A。花色: h(红心), d(方块), s(黑桃), c(梅花)。支持2到9个玩家。"
              },
              "community_cards": {
                "type": "string",
                "description": "可选的公共牌。可以是3张（翻牌，例如 '2h3d4s'）或4张（转牌，例如 '2h3d4s5c'）。",
                "nullable": true
              },
              "num_simulations": {
                "type": "integer",
                "description": "模拟次数。默认为10000次，建议至少10000次以获得较准确结果。",
                "default": 10000,
                "minimum": 100
              }
            },
            "required": ["player_hands"]
          }
        }
      ]
    }
  ]
}