#include "http_client.h"
#include <httplib.h>
#include <sstream>
#include <iostream>

HttpClient::HttpClient(const std::string& api_endpoint) : api_endpoint_(api_endpoint) {}

std::optional<OAuthTokenResponse> HttpClient::exchange_code(
    const std::string& code,
    const std::string& client_id,
    const std::string& client_secret,
    const std::string& redirect_uri) {
    
    std::map<std::string, std::string> data = {
        {"client_id", client_id},
        {"client_secret", client_secret},
        {"grant_type", "authorization_code"},
        {"code", code},
        {"redirect_uri", redirect_uri},
        {"scope", "identify guilds.join"}
    };
    
    auto response = post_request(api_endpoint_ + "/oauth2/token", data);
    if (!response) return std::nullopt;
    
    try {
        OAuthTokenResponse token_response;
        token_response.access_token = (*response)["access_token"];
        token_response.refresh_token = (*response)["refresh_token"];
        token_response.token_type = (*response)["token_type"];
        token_response.expires_in = (*response)["expires_in"];
        return token_response;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<OAuthTokenResponse> HttpClient::refresh_token(
    const std::string& refresh_token,
    const std::string& client_id,
    const std::string& client_secret) {
    
    std::map<std::string, std::string> data = {
        {"client_id", client_id},
        {"client_secret", client_secret},
        {"grant_type", "refresh_token"},
        {"refresh_token", refresh_token}
    };
    
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/x-www-form-urlencoded"}
    };
    
    auto response = post_request(api_endpoint_ + "/oauth2/token", data, headers);
    if (!response) return std::nullopt;
    
    try {
        OAuthTokenResponse token_response;
        token_response.access_token = (*response)["access_token"];
        token_response.refresh_token = (*response)["refresh_token"];
        token_response.token_type = (*response)["token_type"];
        token_response.expires_in = (*response)["expires_in"];
        return token_response;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<DiscordUser> HttpClient::get_user_info(const std::string& access_token) {
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + access_token}
    };
    
    auto response = get_request(api_endpoint_ + "/users/@me", headers);
    if (!response) return std::nullopt;
    
    try {
        DiscordUser user;
        user.id = (*response)["id"];
        user.username = (*response)["username"];
        user.discriminator = (*response).value("discriminator", "0");
        user.avatar = (*response).value("avatar", "");
        return user;
    } catch (...) {
        return std::nullopt;
    }
}

bool HttpClient::add_member_to_guild(
    const std::string& guild_id,
    const std::string& user_id,
    const std::string& access_token,
    const std::string& bot_token) {
    
    json data = {{"access_token", access_token}};
    
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bot " + bot_token},
        {"Content-Type", "application/json"}
    };
    
    std::string url = api_endpoint_ + "/guilds/" + guild_id + "/members/" + user_id;
    return put_request(url, data, headers);
}

bool HttpClient::send_webhook(const std::string& webhook_url, const json& embed_data) {
    try {
        httplib::Client cli(webhook_url.substr(0, webhook_url.find("/api")));
        std::string path = webhook_url.substr(webhook_url.find("/api"));
        
        httplib::Headers headers = {
            {"Content-Type", "application/json"}
        };
        
        auto res = cli.Post(path.c_str(), headers, embed_data.dump(), "application/json");
        return res && (res->status == 200 || res->status == 204);
    } catch (...) {
        return false;
    }
}

std::optional<json> HttpClient::post_request(
    const std::string& url,
    const std::map<std::string, std::string>& data,
    const std::map<std::string, std::string>& headers) {
    
    try {
        httplib::Client cli(api_endpoint_);
        cli.set_follow_location(true);
        
        std::string path = url.substr(api_endpoint_.length());
        
        // Build form data
        std::string body;
        for (const auto& [key, value] : data) {
            if (!body.empty()) body += "&";
            body += key + "=" + httplib::detail::encode_url(value);
        }
        
        httplib::Headers req_headers;
        for (const auto& [key, value] : headers) {
            req_headers.insert({key, value});
        }
        if (req_headers.find("Content-Type") == req_headers.end()) {
            req_headers.insert({"Content-Type", "application/x-www-form-urlencoded"});
        }
        
        auto res = cli.Post(path.c_str(), req_headers, body, "application/x-www-form-urlencoded");
        
        if (res && (res->status == 200 || res->status == 201)) {
            return json::parse(res->body);
        }
        
        return std::nullopt;
    } catch (const std::exception& e) {
        std::cerr << "POST request error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<json> HttpClient::get_request(
    const std::string& url,
    const std::map<std::string, std::string>& headers) {
    
    try {
        httplib::Client cli(api_endpoint_);
        cli.set_follow_location(true);
        
        std::string path = url.substr(api_endpoint_.length());
        
        httplib::Headers req_headers;
        for (const auto& [key, value] : headers) {
            req_headers.insert({key, value});
        }
        
        auto res = cli.Get(path.c_str(), req_headers);
        
        if (res && res->status == 200) {
            return json::parse(res->body);
        }
        
        return std::nullopt;
    } catch (const std::exception& e) {
        std::cerr << "GET request error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool HttpClient::put_request(
    const std::string& url,
    const json& data,
    const std::map<std::string, std::string>& headers) {
    
    try {
        httplib::Client cli(api_endpoint_);
        cli.set_follow_location(true);
        
        std::string path = url.substr(api_endpoint_.length());
        
        httplib::Headers req_headers;
        for (const auto& [key, value] : headers) {
            req_headers.insert({key, value});
        }
        
        auto res = cli.Put(path.c_str(), req_headers, data.dump(), "application/json");
        
        return res && (res->status == 201 || res->status == 204);
    } catch (const std::exception& e) {
        std::cerr << "PUT request error: " << e.what() << std::endl;
        return false;
    }
}
