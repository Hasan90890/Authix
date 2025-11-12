#pragma once

#include <crow.h>
#include <memory>
#include <string>
#include <vector>
#include "auth_manager.h"
#include "http_client.h"

struct ServerConfig {
    std::string client_id;
    std::string client_secret;
    std::string redirect_uri;
    std::string api_endpoint;
    std::vector<std::string> webhook_urls;
    int port;
};

class WebServer {
public:
    WebServer(const ServerConfig& config,
              std::shared_ptr<AuthManager> auth_manager,
              std::shared_ptr<HttpClient> http_client);
    
    void start();
    void stop();

private:
    ServerConfig config_;
    std::shared_ptr<AuthManager> auth_manager_;
    std::shared_ptr<HttpClient> http_client_;
    crow::SimpleApp app_;
    
    void setup_routes();
    std::string get_random_webhook();
    std::string create_success_html(const std::string& username);
    std::string create_error_html(const std::string& error);
};
