// A support bot for a web shop. `support::answer` sends the customer's message to Claude
// with the prompt in prompts/support.md and returns the reply.
#pragma once

#include <curl/curl.h>

#include <memory>
#include <stdexcept>
#include <string>

namespace support {

// What answer throws. status is the API's HTTP status, like 400, 401, 429 or 529,
// or 0 when no answer came back: no key, no network, a timeout or a broken response.
struct Error : std::runtime_error {
    Error(long status, const std::string& message) : std::runtime_error(message), status(status) {}
    long status;
};

// The HTTP client. Build one per thread and reuse it: it keeps the connection to the API open.
class Client {
public:
    Client();  // reads ANTHROPIC_API_KEY

private:
    friend std::string answer(Client& client, const std::string& message);
    bool has_key_;
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_;
    std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> headers_;
};

std::string answer(Client& client, const std::string& message);

}  // namespace support
