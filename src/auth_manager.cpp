#include "auth_manager.h"
#include <fstream>
#include <sstream>
#include <algorithm>

AuthManager::AuthManager(const std::string& filename) : filename_(filename) {}

bool AuthManager::load() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ifstream file(filename_);
    if (!file.is_open()) {
        return false;
    }
    
    tokens_.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        std::string user_id, access_token, refresh_token;
        
        if (std::getline(iss, user_id, ',') &&
            std::getline(iss, access_token, ',') &&
            std::getline(iss, refresh_token)) {
            
            AuthToken token{user_id, access_token, refresh_token};
            tokens_[user_id] = token;
        }
    }
    
    file.close();
    return true;
}

bool AuthManager::save() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ofstream file(filename_);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& [user_id, token] : tokens_) {
        file << token.user_id << "," 
             << token.access_token << "," 
             << token.refresh_token << "\n";
    }
    
    file.close();
    return true;
}

void AuthManager::upsert_token(const std::string& user_id, 
                                const std::string& access_token, 
                                const std::string& refresh_token) {
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_[user_id] = AuthToken{user_id, access_token, refresh_token};
}

std::vector<AuthToken> AuthManager::get_all_tokens() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<AuthToken> result;
    result.reserve(tokens_.size());
    
    for (const auto& [user_id, token] : tokens_) {
        result.push_back(token);
    }
    
    return result;
}

size_t AuthManager::get_unique_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tokens_.size();
}

std::optional<AuthToken> AuthManager::get_token(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = tokens_.find(user_id);
    if (it != tokens_.end()) {
        return it->second;
    }
    return std::nullopt;
}

void AuthManager::update_tokens(const std::vector<AuthToken>& tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    tokens_.clear();
    for (const auto& token : tokens) {
        tokens_[token.user_id] = token;
    }
}

void AuthManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_.clear();
}
