#include "discord_bot.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

DiscordBot::DiscordBot(const BotConfig& config,
                       std::shared_ptr<AuthManager> auth_manager,
                       std::shared_ptr<HttpClient> http_client)
    : config_(config), auth_manager_(auth_manager), http_client_(http_client) {
    
    bot_ = std::make_unique<dpp::cluster>(config_.token, dpp::i_all_intents);
}

void DiscordBot::start() {
    bot_->on_log(dpp::utility::cout_logger());
    
    bot_->on_ready([this](const dpp::ready_t& event) {
        std::cout << "Connected as: " << bot_->me.username << std::endl;
        setup_commands();
    });
    
    bot_->on_slashcommand([this](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "count") {
            handle_count(event);
        } else if (event.command.get_command_name() == "refresh") {
            handle_refresh(event);
        } else if (event.command.get_command_name() == "pull") {
            handle_pull(event);
        } else if (event.command.get_command_name() == "auth_link") {
            handle_auth_link(event);
        } else if (event.command.get_command_name() == "help") {
            handle_help(event);
        }
    });
    
    bot_->start(dpp::st_wait);
}

void DiscordBot::stop() {
    if (bot_) {
        bot_->shutdown();
    }
}

void DiscordBot::setup_commands() {
    if (dpp::run_once<struct register_bot_commands>()) {
        std::vector<dpp::slashcommand> commands = {
            dpp::slashcommand("count", "Display total number of unique auths", bot_->me.id),
            dpp::slashcommand("refresh", "Refresh tokens for all users", bot_->me.id),
            dpp::slashcommand("pull", "Pull users into the server", bot_->me.id)
                .add_option(dpp::command_option(dpp::co_integer, "amount", "Number of users to pull", true)),
            dpp::slashcommand("auth_link", "Generate authentication link", bot_->me.id),
            dpp::slashcommand("help", "Show help information", bot_->me.id)
        };
        
        bot_->global_bulk_command_create(commands);
    }
}

void DiscordBot::handle_count(const dpp::slashcommand_t& event) {
    size_t count = auth_manager_->get_unique_count();
    
    dpp::embed embed = create_embed(
        "🔢 auth count",
        "total unique auths: " + std::to_string(count),
        0x00ff00
    );
    embed.set_thumbnail("https://img.icons8.com/color/48/000000/counter.png");
    embed.set_footer(dpp::embed_footer().set_text("authix bot • premium authentication service"));
    
    dpp::message msg(event.command.channel_id, embed);
    event.reply(msg);
}

void DiscordBot::handle_refresh(const dpp::slashcommand_t& event) {
    event.thinking();
    
    auto start_time = std::chrono::steady_clock::now();
    auto tokens = auth_manager_->get_all_tokens();
    
    if (tokens.empty()) {
        dpp::embed embed = create_embed(
            "⚠️ no auths found",
            "the database is empty. nothing to refresh.",
            0xffa500
        );
        embed.set_thumbnail("https://img.icons8.com/color/48/000000/empty-box.png");
        event.edit_response(dpp::message(event.command.channel_id, embed));
        return;
    }
    
    int total = tokens.size();
    int refreshed = 0;
    int failed = 0;
    std::vector<AuthToken> new_tokens;
    
    for (int i = 0; i < total; ++i) {
        const auto& token = tokens[i];
        
        auto response = http_client_->refresh_token(
            token.refresh_token,
            config_.client_id,
            config_.client_secret
        );
        
        if (response) {
            new_tokens.push_back({
                token.user_id,
                response->access_token,
                response->refresh_token
            });
            refreshed++;
        } else {
            failed++;
        }
        
        // Update progress every 5 tokens
        if ((i + 1) % 5 == 0 || (i + 1) == total) {
            std::string progress = create_progress_bar(i + 1, total);
            dpp::embed embed = create_embed(
                "🔄 refreshing tokens",
                "progress: " + progress + "\nprocessed: " + std::to_string(i + 1) + "/" + std::to_string(total),
                0xffff00
            );
            embed.set_thumbnail("https://img.icons8.com/color/48/000000/refresh.png");
            event.edit_response(dpp::message(event.command.channel_id, embed));
        }
    }
    
    auth_manager_->update_tokens(new_tokens);
    auth_manager_->save();
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    int mins = duration / 60;
    int secs = duration % 60;
    
    std::ostringstream time_str;
    time_str << mins << "m " << secs << "s";
    
    dpp::embed embed = create_embed(
        "✅ token refresh complete",
        "refreshed tokens for " + std::to_string(refreshed) + " users out of " + 
        std::to_string(total) + " in " + time_str.str() + "\nfailed refreshes: " + std::to_string(failed),
        0x00ff00
    );
    embed.set_thumbnail("https://img.icons8.com/color/48/000000/checked.png");
    event.edit_response(dpp::message(event.command.channel_id, embed));
}

void DiscordBot::handle_pull(const dpp::slashcommand_t& event) {
    int amount = std::get<int64_t>(event.get_parameter("amount"));
    
    event.thinking();
    
    auto start_time = std::chrono::steady_clock::now();
    auto tokens = auth_manager_->get_all_tokens();
    
    if (tokens.empty()) {
        dpp::embed embed = create_embed(
            "⚠️ no auths found",
            "the database is empty. nothing to pull.",
            0xffa500
        );
        embed.set_thumbnail("https://img.icons8.com/color/48/000000/empty-box.png");
        event.edit_response(dpp::message(event.command.channel_id, embed));
        return;
    }
    
    // Shuffle tokens
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(tokens.begin(), tokens.end(), g);
    
    int tries = 0;
    int added = 0;
    int failed = 0;
    std::vector<std::string> last_users;
    
    dpp::snowflake guild_id = event.command.guild_id;
    
    for (size_t i = 0; i < tokens.size() && added < amount; ++i) {
        tries++;
        const auto& token = tokens[i];
        
        bool success = http_client_->add_member_to_guild(
            std::to_string(guild_id),
            token.user_id,
            token.access_token,
            config_.token
        );
        
        if (success) {
            auto user_info = http_client_->get_user_info(token.access_token);
            if (user_info) {
                last_users.push_back(user_info->username + " (" + token.user_id + ")");
                if (last_users.size() > 5) {
                    last_users.erase(last_users.begin());
                }
            }
            added++;
        } else {
            failed++;
        }
        
        // Update progress every 5 tries
        if (tries % 5 == 0 || added == amount) {
            std::string progress = create_progress_bar(added, amount);
            std::string last_added = last_users.empty() ? "none" : "";
            for (const auto& user : last_users) {
                if (!last_added.empty()) last_added += ", ";
                last_added += user;
            }
            
            dpp::embed embed = create_embed(
                "🚀 pulling users",
                "progress: " + progress + "\ntries: " + std::to_string(tries) + 
                " | added: " + std::to_string(added) + " | failed: " + std::to_string(failed) +
                "\nlast added: " + last_added,
                0x0000ff
            );
            embed.set_thumbnail("https://img.icons8.com/color/48/000000/download.png");
            event.edit_response(dpp::message(event.command.channel_id, embed));
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    int mins = duration / 60;
    int secs = duration % 60;
    
    std::ostringstream time_str;
    time_str << mins << "m " << secs << "s";
    
    dpp::embed embed = create_embed(
        "✅ pull operation complete",
        "pulled " + std::to_string(added) + " users with " + std::to_string(failed) + 
        " failures after " + std::to_string(tries) + " tries in " + time_str.str(),
        0x00ff00
    );
    embed.set_thumbnail("https://img.icons8.com/color/48/000000/checked.png");
    event.edit_response(dpp::message(event.command.channel_id, embed));
}

void DiscordBot::handle_auth_link(const dpp::slashcommand_t& event) {
    std::ostringstream url;
    url << "https://discord.com/oauth2/authorize?"
        << "client_id=" << config_.client_id
        << "&response_type=code"
        << "&redirect_uri=" << config_.redirect_uri
        << "&scope=identify%20guilds.join";
    
    dpp::embed embed = create_embed(
        "🔗 authentication link",
        "click the link below to authenticate and join the premium authix service:",
        0x0000ff
    );
    embed.add_field("authenticate here", "[click to authenticate](" + url.str() + ")");
    embed.set_thumbnail("https://img.icons8.com/color/48/000000/link.png");
    
    dpp::message msg(event.command.channel_id, embed);
    event.reply(msg);
}

void DiscordBot::handle_help(const dpp::slashcommand_t& event) {
    dpp::embed embed = create_embed(
        "📚 authix bot commands help",
        "here's a premium list of available commands for authix bot:",
        0x800080
    );
    
    embed.add_field("/count", "displays the total number of unique auths in the database.", false);
    embed.add_field("/refresh", "refreshes tokens for all unique users in the database with progress updates.", false);
    embed.add_field("/pull <amount>", "pulls a specified amount of users into your server with live progress.", false);
    embed.add_field("/auth_link", "generates the authentication link for users to join.", false);
    embed.set_thumbnail("https://img.icons8.com/color/48/000000/help.png");
    
    dpp::message msg(event.command.channel_id, embed);
    event.reply(msg);
}

dpp::embed DiscordBot::create_embed(const std::string& title, const std::string& description, uint32_t color) {
    dpp::embed embed;
    embed.set_title(title);
    embed.set_description(description);
    embed.set_color(color);
    embed.set_timestamp(time(nullptr));
    return embed;
}

std::string DiscordBot::create_progress_bar(int current, int total, int length) {
    int filled = (length * current) / total;
    std::string bar = std::string(filled, '█') + std::string(length - filled, '—');
    return "[" + bar + "] " + std::to_string(current) + "/" + std::to_string(total);
}

std::string DiscordBot::get_random_webhook() {
    if (config_.webhook_urls.empty()) return "";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, config_.webhook_urls.size() - 1);
    
    return config_.webhook_urls[dis(gen)];
}
