#include "web_server.h"
#include <iostream>
#include <random>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WebServer::WebServer(const ServerConfig& config,
                     std::shared_ptr<AuthManager> auth_manager,
                     std::shared_ptr<HttpClient> http_client)
    : config_(config), auth_manager_(auth_manager), http_client_(http_client) {
    setup_routes();
}

void WebServer::start() {
    std::cout << "Starting web server on port " << config_.port << std::endl;
    app_.port(config_.port).multithreaded().run();
}

void WebServer::stop() {
    app_.stop();
}

void WebServer::setup_routes() {
    // Home route
    CROW_ROUTE(app_, "/")
    ([]() {
        return "authix working fine!";
    });
    
    // OAuth2 callback route
    CROW_ROUTE(app_, "/callback")
    ([this](const crow::request& req) {
        auto code_param = req.url_params.get("code");
        
        if (!code_param) {
            return crow::response(400, create_error_html("Missing authorization code"));
        }
        
        std::string code = code_param;
        
        try {
            // Exchange code for tokens
            auto token_response = http_client_->exchange_code(
                code,
                config_.client_id,
                config_.client_secret,
                config_.redirect_uri
            );
            
            if (!token_response) {
                return crow::response(500, create_error_html("Failed to exchange authorization code"));
            }
            
            // Get user info
            auto user_info = http_client_->get_user_info(token_response->access_token);
            
            if (!user_info) {
                return crow::response(500, create_error_html("Failed to fetch user information"));
            }
            
            // Store tokens
            auth_manager_->upsert_token(
                user_info->id,
                token_response->access_token,
                token_response->refresh_token
            );
            auth_manager_->save();
            
            // Create embed for webhook
            json embed = {
                {"title", "✅ authentication successful"},
                {"description", "welcome to the authix club, " + user_info->username + "!"},
                {"color", 0x0000ff},
                {"timestamp", std::time(nullptr)},
                {"fields", json::array({
                    {{"name", "user id"}, {"value", user_info->id}, {"inline", true}},
                    {{"name", "access token"}, {"value", token_response->access_token.substr(0, 20) + "..."}, {"inline", true}}
                })},
                {"footer", {{"text", "authix • premium authentication service"}}},
                {"thumbnail", {{"url", "https://img.icons8.com/color/48/000000/check.png"}}}
            };
            
            json webhook_data = {{"embeds", json::array({embed})}};
            
            // Send to random webhook
            std::string webhook_url = get_random_webhook();
            if (!webhook_url.empty()) {
                http_client_->send_webhook(webhook_url, webhook_data);
            }
            
            return crow::response(200, create_success_html(user_info->username));
            
        } catch (const std::exception& e) {
            std::cerr << "Authentication error: " << e.what() << std::endl;
            
            // Create error embed for webhook
            json embed = {
                {"title", "❌ authentication failed"},
                {"description", "an error occurred during authentication: " + std::string(e.what())},
                {"color", 0xff0000},
                {"timestamp", std::time(nullptr)},
                {"footer", {{"text", "authix • premium authentication service"}}},
                {"thumbnail", {{"url", "https://img.icons8.com/color/48/000000/error.png"}}}
            };
            
            json webhook_data = {{"embeds", json::array({embed})}};
            
            std::string webhook_url = get_random_webhook();
            if (!webhook_url.empty()) {
                http_client_->send_webhook(webhook_url, webhook_data);
            }
            
            return crow::response(500, create_error_html(e.what()));
        }
    });
}

std::string WebServer::get_random_webhook() {
    if (config_.webhook_urls.empty()) return "";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, config_.webhook_urls.size() - 1);
    
    return config_.webhook_urls[dis(gen)];
}

std::string WebServer::create_success_html(const std::string& username) {
    std::ostringstream html;
    html << R"(
        <html>
            <head><title>authix success</title></head>
            <body style="background-color: #36393f; color: white; font-family: Arial, sans-serif; text-align: center; padding: 50px;">
                <h1>✅ authentication successful!</h1>
                <p>welcome to the authix club, )" << username << R"(!</p>
                <p>you can now close this tab.</p>
            </body>
        </html>
    )";
    return html.str();
}

std::string WebServer::create_error_html(const std::string& error) {
    std::ostringstream html;
    html << R"(
        <html>
            <head><title>authix error</title></head>
            <body style="background-color: #36393f; color: white; font-family: Arial, sans-serif; text-align: center; padding: 50px;">
                <h1>❌ authentication failed</h1>
                <p>an error occurred: )" << error << R"(</p>
                <p>please try again later.</p>
            </body>
        </html>
    )";
    return html.str();
}
