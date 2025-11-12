#include <iostream>
#include <fstream>
#include <thread>
#include <memory>
#include <csignal>
#include <nlohmann/json.hpp>
#include "auth_manager.h"
#include "http_client.h"
#include "discord_bot.h"
#include "web_server.h"

using json = nlohmann::json;

// Global pointers for signal handling
std::unique_ptr<DiscordBot> g_bot;
std::unique_ptr<WebServer> g_server;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    
    if (g_server) {
        g_server->stop();
    }
    
    if (g_bot) {
        g_bot->stop();
    }
    
    exit(0);
}

json load_config(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + filename);
    }
    
    json config;
    file >> config;
    return config;
}

int main() {
    try {
        // Set up signal handlers
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        
        // Load configuration
        std::cout << "Loading configuration..." << std::endl;
        json config = load_config("config.json");
        
        std::string token = config["token"];
        std::string client_secret = config["secret"];
        std::string client_id = config["id"];
        std::string redirect_uri = config["redirect"];
        std::string api_endpoint = config["api_endpoint"];
        std::vector<std::string> webhook_urls = config["logs"].get<std::vector<std::string>>();
        
        // Validate configuration
        if (token == "YOUR_BOT_TOKEN" || client_id == "YOUR_BOT_CLIENT_ID") {
            std::cerr << "Error: Please configure config.json with your bot credentials!" << std::endl;
            return 1;
        }
        
        // Initialize shared components
        auto auth_manager = std::make_shared<AuthManager>("auths.txt");
        auto http_client = std::make_shared<HttpClient>(api_endpoint);
        
        // Load existing tokens
        std::cout << "Loading authentication tokens..." << std::endl;
        auth_manager->load();
        std::cout << "Loaded " << auth_manager->get_unique_count() << " unique tokens" << std::endl;
        
        // Create bot configuration
        BotConfig bot_config;
        bot_config.token = token;
        bot_config.client_id = client_id;
        bot_config.client_secret = client_secret;
        bot_config.redirect_uri = redirect_uri;
        bot_config.api_endpoint = api_endpoint;
        bot_config.webhook_urls = webhook_urls;
        
        // Create server configuration
        ServerConfig server_config;
        server_config.client_id = client_id;
        server_config.client_secret = client_secret;
        server_config.redirect_uri = redirect_uri;
        server_config.api_endpoint = api_endpoint;
        server_config.webhook_urls = webhook_urls;
        server_config.port = 8000;
        
        // Create bot and server instances
        g_bot = std::make_unique<DiscordBot>(bot_config, auth_manager, http_client);
        g_server = std::make_unique<WebServer>(server_config, auth_manager, http_client);
        
        // Start web server in a separate thread
        std::cout << "Starting web server..." << std::endl;
        std::thread server_thread([&]() {
            g_server->start();
        });
        
        // Give the server a moment to start
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Start Discord bot (blocking)
        std::cout << "Starting Discord bot..." << std::endl;
        g_bot->start();
        
        // Wait for server thread
        if (server_thread.joinable()) {
            server_thread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
