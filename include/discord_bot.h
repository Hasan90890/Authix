#pragma once

#include <dpp/dpp.h>
#include <memory>
#include <string>
#include <vector>
#include "auth_manager.h"
#include "http_client.h"

struct BotConfig {
    std::string token;
    std::string client_id;
    std::string client_secret;
    std::string redirect_uri;
    std::string api_endpoint;
    std::vector<std::string> webhook_urls;
};

class DiscordBot {
public:
    DiscordBot(const BotConfig& config, 
               std::shared_ptr<AuthManager> auth_manager,
               std::shared_ptr<HttpClient> http_client);
    
    void start();
    void stop();
    
private:
    BotConfig config_;
    std::shared_ptr<AuthManager> auth_manager_;
    std::shared_ptr<HttpClient> http_client_;
    std::unique_ptr<dpp::cluster> bot_;
    
    // Command handlers
    void setup_commands();
    void handle_count(const dpp::slashcommand_t& event);
    void handle_refresh(const dpp::slashcommand_t& event);
    void handle_pull(const dpp::slashcommand_t& event);
    void handle_auth_link(const dpp::slashcommand_t& event);
    void handle_help(const dpp::slashcommand_t& event);
    
    // Helper functions
    dpp::embed create_embed(const std::string& title, const std::string& description, uint32_t color);
    std::string create_progress_bar(int current, int total, int length = 20);
    std::string get_random_webhook();
};
