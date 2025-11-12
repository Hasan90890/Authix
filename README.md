# Authix v3 - C++ Edition

**Authix v3 C++** is a **fast, async Discord bot** built with **DPP (D++)** and **Crow**, designed for **member backup and restoration using stored tokens (auths)**. This is a complete C++ rewrite of the original Python version, offering improved performance and lower resource usage.

---

## Features

* 💾 **Member Backup**: Save Discord members' tokens safely.
* 🔄 **Member Restoration**: Re-add backed-up members to any server.
* ⚡ **Fully Async**: Multi-threaded architecture for fast operations.
* 🛠 **Lightweight**: Minimal dependencies and efficient C++ implementation.
* 🚀 **High Performance**: Native C++ performance with modern libraries.

---

## Requirements

* **C++ Compiler**: GCC 9+ or Clang 10+ with C++17 support
* **CMake**: Version 3.15 or higher
* **OpenSSL**: For HTTPS support
* **libcurl**: For HTTP requests
* **Git**: For fetching dependencies

### System Dependencies (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git libssl-dev zlib1g-dev libcurl4-openssl-dev
```

### System Dependencies (Fedora/RHEL/Amazon Linux)

```bash
sudo dnf install -y gcc-c++ cmake git openssl-devel zlib-devel libcurl-devel
```

### System Dependencies (macOS)

```bash
brew install cmake openssl curl
```

---

## Installation

```bash
# Clone the repository
git clone https://github.com/pygod139/Authix.git
cd Authix

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project (this will download and build all dependencies)
cmake --build . -j$(nproc)

# The executable will be in the build directory
./authix
```

---

## Configuration

Fill out `config.json` with your bot details:

```json
{
  "token": "YOUR_BOT_TOKEN",
  "secret": "YOUR_BOT_SECRET",
  "id": "YOUR_BOT_CLIENT_ID",
  "redirect": "http://127.0.0.1:8000/callback",
  "api_endpoint": "https://canary.discord.com/api/v8",
  "logs": [
    "YOUR_WEBHOOK_LINK_1",
    "YOUR_WEBHOOK_LINK_2"
  ]
}
```

**Important**: Replace all placeholder values with your actual Discord bot credentials.

---

## Usage

```bash
# Run from the build directory
./authix

# Or run from the project root
./build/authix
```

The bot will:
1. Start the web server on port 8000 for OAuth2 callbacks
2. Connect to Discord and register slash commands
3. Load existing authentication tokens from `auths.txt`

### Bot Commands

All commands are slash commands:

| Command          | Description                      |
| ---------------- | -------------------------------- |
| `/help`          | Show help message                |
| `/pull <amount>` | Pull specified number of members |
| `/refresh`       | Refresh all stored tokens        |
| `/count`         | Show token count                 |
| `/auth_link`     | Generate authentication link     |

---

## Project Structure

```
Authix/
├── CMakeLists.txt          # Build configuration
├── config.json             # Bot configuration
├── auths.txt              # Stored authentication tokens
├── include/               # Header files
│   ├── auth_manager.h     # Token storage management
│   ├── http_client.h      # Discord API client
│   ├── discord_bot.h      # Discord bot implementation
│   └── web_server.h       # OAuth2 web server
├── src/                   # Source files
│   ├── main.cpp           # Application entry point
│   ├── auth_manager.cpp   # Token storage implementation
│   ├── http_client.cpp    # HTTP client implementation
│   ├── discord_bot.cpp    # Bot commands and handlers
│   └── web_server.cpp     # Web server implementation
└── README.md              # This file
```

---

## Dependencies

The project uses CMake's FetchContent to automatically download and build:

* **DPP (D++)**: Discord C++ library for bot functionality
* **Crow**: Lightweight C++ web framework for OAuth2 callbacks
* **nlohmann/json**: JSON parsing and serialization
* **cpp-httplib**: HTTP client for Discord API requests

All dependencies are fetched and built automatically during the CMake build process.

---

## Building for Production

For optimized production builds:

```bash
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

---

## Troubleshooting

### Build Errors

If you encounter build errors:

1. Ensure all system dependencies are installed
2. Clear the build directory: `rm -rf build && mkdir build`
3. Try building with verbose output: `cmake --build . --verbose`

### Runtime Errors

* **"Failed to open config file"**: Ensure `config.json` exists in the working directory
* **"Please configure config.json"**: Update config.json with your actual bot credentials
* **Connection errors**: Check your bot token and internet connection

---

## Performance Notes

The C++ version offers significant performance improvements over the Python version:

* **Lower Memory Usage**: ~50-70% less memory consumption
* **Faster Startup**: Near-instant startup time
* **Better Concurrency**: Native multi-threading support
* **Reduced CPU Usage**: More efficient event handling

---

## Contributing

1. Fork the repository.
2. Create a branch (`git checkout -b feature-name`).
3. Commit your changes (`git commit -am 'Add new feature'`).
4. Push branch (`git push origin feature-name`).
5. Open a Pull Request.

---

## Disclaimer

**Authix v3** is intended solely for **controlled member backup and restoration**. Misuse for spamming or unauthorized member adding may violate Discord's Terms of Service.

---

## Credits

* **Original Python Version**: Rubin B (pygod7 / rubinexe)
* **C++ Port**: Converted to modern C++ with DPP and Crow
* **Libraries**: DPP, Crow, nlohmann/json, cpp-httplib

---

## License

This project is provided as-is for educational purposes. Use responsibly and in accordance with Discord's Terms of Service.
