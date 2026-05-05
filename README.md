# Texas Poker Odds MCP Server
[简体中文 README](README_CN.md)

This project provides a tool to calculate the win, loss, and tie probabilities for multiple hands in Texas Hold'em poker, given optional community cards, and encapsulates it as an MCP (Model Context Protocol) server.

**Key Highlights:**

*   **Accurate Calculation**: Provides win, loss, and tie probabilities for multiple hands under different community card scenarios through extensive simulations, helping players make more informed decisions.
*   **Flexible Input**: Supports inputting multiple hole card sets (for different players), optional 3 (flop) or 4 (turn) community cards, and specifying the number of simulations.
*   **Multi-core Optimization**: Utilizes multi-core CPUs for parallel computation, significantly increasing simulation speed, especially for a large number of simulations.
*   **MCP Server Integration**: Encapsulates the probability calculation functionality as an MCP tool, allowing other AI agents or systems to call it via the MCP protocol.

## How to Use

### Quick Start (uvx one-liner)

You can run this MCP server directly from the git repository without cloning:

```bash
uvx --from git+https://github.com/ptbsare/texas-pokker-odds-mcp-server.git texas-pokker-odds-mcp-server
```

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

**Using uvx (recommended):**

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

**Using local path:**

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