// tinyeval worker: evals/support.sh starts it with `worker build/evalworker`.
// It reads each input up to a NUL byte on stdin, and prints the reply and a NUL byte.
#include <algorithm>
#include <iostream>
#include <string>

#include "support.hpp"

// Turns an error into a reply. When the cause isn't the app (no key, a bad key, a rate limit,
// a server error, a timeout or the network), tinyeval counts it as couldn't run.
// Any other error, like a bad model name, is the reply, so the eval fails.
static std::string failure(const support::Error& e) {
    if (e.status == 0 || e.status == 401 || e.status == 429 || e.status >= 500) {
        std::string reason = e.what();
        std::ranges::replace(reason, '\n', ' ');
        return "#tinyeval couldnt_run " + reason + "\n";
    }
    return std::string("error: ") + e.what();
}

int main() {
    support::Client client;  // once per worker, not once per test case
    std::string input;
    while (std::getline(std::cin, input, '\0')) {
        std::string reply;
        try {
            reply = support::answer(client, input);
        } catch (const support::Error& e) {
            reply = failure(e);
        }
        std::cout << reply << '\0' << std::flush;
    }
}
