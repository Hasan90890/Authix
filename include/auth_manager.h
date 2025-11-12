#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <optional>

struct AuthToken {
    std::string user_id;
    std::string access_token;
    std::string refresh_token;
};

class AuthManager {
public:
    AuthManager(const std::string& filename = "auths.txt");
    
    // Load tokens from file
    bool load();
    
    // Save tokens to file
    bool save();
    
    // Add or update a token
    void upsert_token(const std::string& user_id, const std::string& access_token, const std::string& refresh_token);
    
    // Get all tokens
    std::vector<AuthToken> get_all_tokens();
    
    // Get unique user count
    size_t get_unique_count() const;
    
    // Get token by user_id
    std::optional<AuthToken> get_token(const std::string& user_id);
    
    // Update all tokens (for refresh operation)
    void update_tokens(const std::vector<AuthToken>& tokens);
    
    // Clear all tokens
    void clear();

private:
    std::string filename_;
    std::unordered_map<std::string, AuthToken> tokens_;
    mutable std::mutex mutex_;
};
