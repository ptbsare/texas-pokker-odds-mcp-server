# 德州扑克胜率 MCP 服务器

本项目提供了一个计算德州扑克多手牌在给定公共牌（可选）情况下的胜负平概率的工具，并将其封装为一个 MCP (Model Context Protocol) 服务器。

**核心亮点：**

*   **精确计算**：通过大量模拟，提供多手牌在不同公共牌情况下的胜负平概率，帮助玩家做出更明智的决策。
*   **灵活输入**：支持传入多组手牌（对应不同玩家），以及可选的3张（翻牌）或4张（转牌）公共牌，并可指定模拟次数。
*   **多核优化**：利用多核 CPU 并行计算，显著提高模拟速度，尤其适用于大量模拟次数的场景。
*   **MCP 服务器集成**：将概率计算功能封装为 MCP 工具，方便其他 AI 代理或系统通过 MCP 协议调用。

## 如何使用

### 快速开始 (uvx 一行命令)

你可以直接从 git 仓库运行此 MCP 服务器，无需克隆：

```bash
uvx --from git+https://github.com/ptbsare/texas-pokker-odds-mcp-server.git texas-pokker-odds-mcp-server
```

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

**使用 uvx (推荐):**

```json
{
  "mcp_servers": [
    {
      "name": "texas-poker-odds-generator",
      "type": "stdio",
      "command": ["uvx", "--from", "git+https://github.com/ptbsare/texas-pokker-odds-mcp-server.git", "texas-pokker-odds-mcp-server"]
    }
  ]
}
```

**使用本地路径:**

```json
{
  "mcp_servers": [
    {
      "name": "texas-poker-odds-generator",
      "type": "stdio",
      "command": ["uv", "--directory", "/path/to/texas-pokker-odds-mcp-server/", "run", "server.py"]
    }
  ]
}
```