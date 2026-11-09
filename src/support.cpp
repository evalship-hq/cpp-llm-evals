// C++ has no official Anthropic SDK, so this calls the Messages API with libcurl and nlohmann/json.
#include "support.hpp"

#include <cstdlib>

#include <nlohmann/json.hpp>

#include "prompt.hpp"  // kPrompt, made by CMake from prompts/support.md

using nlohmann::json;

namespace support {

Client::Client()
    : has_key_(false), curl_(curl_easy_init(), curl_easy_cleanup), headers_(nullptr, curl_slist_free_all) {
    if (!curl_) throw std::runtime_error("curl_easy_init failed");
    const char* key = std::getenv("ANTHROPIC_API_KEY");
    has_key_ = key && *key;
    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("x-api-key: " + std::string(has_key_ ? key : "")).c_str());
    headers = curl_slist_append(headers, "anthropic-version: 2026-10-21");
    headers = curl_slist_append(headers, "content-type: application/json");
    headers_.reset(headers);

    CURL* curl = curl_.get();
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.anthropic.com/v1/messages");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char* data, size_t size, size_t n, void* out) {
        static_cast<std::string*>(out)->append(data, size * n);
        return size * n;
    });
}

std::string answer(Client& client, const std::string& message) {
    if (!client.has_key_) throw Error(0, "ANTHROPIC_API_KEY is not set");
    json request = {
        {"model", "claude-opus-5-5"},
        {"max_tokens", 1024},
        // Opus 5.5 always thinks; low effort keeps a short reply quick.
        {"output_config", {{"effort", "low"}}},
        {"system", kPrompt},
        {"messages", json::array({{{"role", "user"}, {"content", message}}})},
    };
    // replace: bytes that aren't UTF-8 become U+FFFD instead of throwing
    std::string body = request.dump(-1, ' ', false, json::error_handler_t::replace);
    std::string response;
    CURL* curl = client.curl_.get();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    if (CURLcode code = curl_easy_perform(curl); code != CURLE_OK) throw Error(0, curl_easy_strerror(code));

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    std::string error = std::to_string(status) + " ";
    try {
        json reply = json::parse(response);
        if (status == 200) {
            std::string text;
            for (const json& block : reply.at("content"))
                if (block.at("type") == "text") text += block.at("text").get<std::string>();
            return text;
        }
        const json& detail = reply.at("error");
        error += detail.at("type").get<std::string>() + ": " + detail.at("message").get<std::string>();
    } catch (const json::exception&) {
        error += "response that isn't the API's JSON";  // like a 502 page from a proxy
    }
    throw Error(status == 200 ? 0 : status, error);
}

}  // namespace support
