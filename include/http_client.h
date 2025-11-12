#pragma once

#include <string>
#include <map>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct OAuthTokenResponse {
    std::string access_token;
    std::string refresh_token;
    std::string token_type;
    int expires_in;
};

struct DiscordUser {
    std::string id;
    std::string username;
    std::string discriminator;
    std::string avatar;
};

class HttpClient {
public:
    HttpClient(const std::string& api_endpoint);
    
    // Exchange OAuth2 code for tokens
    std::optional<OAuthTokenResponse> exchange_code(
        const std::string& code,
        const std::string& client_id,
        const std::string& client_secret,
        const std::string& redirect_uri
    );
    
    // Refresh OAuth2 token
    std::optional<OAuthTokenResponse> refresh_token(
        const std::string& refresh_token,
        const std::string& client_id,
        const std::string& client_secret
    );
    
    // Get user info from access token
    std::optional<DiscordUser> get_user_info(const std::string& access_token);
    
    // Add member to guild
    bool add_member_to_guild(
        const std::string& guild_id,
        const std::string& user_id,
        const std::string& access_token,
        const std::string& bot_token
    );
    
    // Send webhook message
    bool send_webhook(const std::string& webhook_url, const json& embed_data);

private:
    std::string api_endpoint_;
    
    // Helper for POST requests
    std::optional<json> post_request(
        const std::string& url,
        const std::map<std::string, std::string>& data,
        const std::map<std::string, std::string>& headers = {}
    );
    
    // Helper for GET requests
    std::optional<json> get_request(
        const std::string& url,
        const std::map<std::string, std::string>& headers = {}
    );
    
    // Helper for PUT requests
    bool put_request(
        const std::string& url,
        const json& data,
        const std::map<std::string, std::string>& headers = {}
    );
};
