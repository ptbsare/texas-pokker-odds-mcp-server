# 德州扑克胜率 MCP 服务器

本项目提供了一个计算德州扑克两手牌在给定公共牌（可选）情况下的胜负平概率的工具，并将其封装为一个 MCP (Model Context Protocol) 服务器。

**核心亮点：**

*   **精确计算**：通过大量模拟，提供两手牌在不同公共牌情况下的胜负平概率，帮助玩家做出更明智的决策。
*   **灵活输入**：支持传入两张手牌，以及可选的3张（翻牌）或4张（转牌）公共牌，并可指定模拟次数。
*   **多核优化**：利用多核 CPU 并行计算，显著提高模拟速度，尤其适用于大量模拟次数的场景。
*   **MCP 服务器集成**：将概率计算功能封装为 MCP 工具，方便其他 AI 代理或系统通过 MCP 协议调用。

## 如何使用

### 1. 环境准备

确保您的系统安装了 Python 3.10 或更高版本。

推荐使用 `uv` 进行依赖管理：

```bash
# 安装 uv (如果尚未安装)
curl -LsSf https://astral.sh/uv/install.sh | sh

# 进入项目目录
cd texas-pokker-odds-mcp-server

# 使用 uv 安装依赖
uv sync
```

### 2. 作为 MCP 服务器使用

您可以将本项目作为 MCP 服务器运行，以便其他 AI 代理或系统通过 MCP 协议调用 `calculate_poker_odds` 工具。

```bash
uv run server.py
```

服务器启动后，它将监听 MCP 客户端的请求。

### 3. Claude MCP 服务器配置示例

以下是一个 `mcp_server.json` 配置文件示例，用于将此 MCP 服务器连接到 Claude：

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
          "description": "计算德州扑克两手牌在给定公共牌（可选）情况下的胜负平概率。模拟次数越多，结果越精确。",
          "input_schema": {
            "type": "object",
            "properties": {
              "player1_hand": {
                "type": "string",
                "description": "玩家1的两张手牌，例如 'AsKd' (A黑桃, K方块)。牌面: 2-9, T(10), J, Q, K, A。花色: h(红心), d(方块), s(黑桃), c(梅花)。"
              },
              "player2_hand": {
                "type": "string",
                "description": "玩家2的两张手牌，例如 '7c8h' (7梅花, 8红心)。格式同玩家1手牌。"
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
            "required": ["player1_hand", "player2_hand"]
          }
        }
      ]
    }
  ]
}