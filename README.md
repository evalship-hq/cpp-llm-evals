# LLM evals for C++

A C++ support bot whose AI call is tested on every pull request with [tinyeval](https://tinyeval.com).

## Run it

Clone this repo, or use it as a template. You need CMake, a C++20 compiler, libcurl and nlohmann/json (`brew install cmake nlohmann-json` on macOS). Then:

```sh
export ANTHROPIC_API_KEY=sk-ant-...
cmake -B build && cmake --build build
./tinyeval
```

```
support
  ✓ refund after the window 3.9s
  ✓ missing package 3.5s
  ✓ refund inside the window 3.1s

All 3 evals passed (eval-results.json)
```

## What's here

- `src/support.cpp`: the bot. `support::answer(client, message)` sends the message to Claude and returns the reply. C++ has no official Anthropic SDK, so it calls the Messages API with libcurl and nlohmann/json.
- `src/support.hpp`: the bot's `Client` and `Error` types.
- `prompts/support.md`: the system prompt. CMake embeds it in the library.
- `src/evalworker.cpp`: the tinyeval worker. It builds the `Client` once, then answers each test case.
- `CMakeLists.txt`: the `support` library and the `evalworker` executable.
- `evals/support.sh`: three test cases, each checked by a regex, a judge or both.
- `tinyeval`: the eval runner, one bash file.
- `.github/workflows/evals.yml`: runs the evals on every pull request and push to main.

## Use it for your app

1. Replace `answer` with your AI call. Keep the worker, so your app starts once per worker and not once per test case.
2. Write your test cases in `evals/`. The [check lines](https://github.com/evalship-hq/tinyeval#writing-evals) cover regexes, JSON, your own graders and a model judge.
3. Keep the AI call in a library that your app and the worker both link. Build its HTTP client once in the worker's `main`.

When the API call fails for a reason that isn't your code, such as a missing key, a rate limit or a timeout, the worker replies `#tinyeval couldnt_run`. tinyeval then counts the test case as couldn't run, not as failed.

## In CI

Add `ANTHROPIC_API_KEY` as a repository secret under **Settings** > **Secrets and variables** > **Actions**. The bot and tinyeval's judge both use it.

The workflow installs libcurl and nlohmann/json with apt, then builds with CMake. It runs each test case 3 times, and a test case passes when at least 2 of the 3 pass. A failing eval fails the job, so you can require it before merging. The results upload as the `eval-results` artifact, also when an eval fails.

Without the secret, the workflow skips the evals and prints a notice. Pull requests from forks get no secrets, so their evals skip too.

## Compare pull requests with main

A fresh CI run has no earlier run to compare with. [evalship](https://evalship.com) reads `eval-results.json`, compares each pull request with main, tells flaky evals from real drops and comments on the pull request.

## License

MIT. See [LICENSE](LICENSE).
