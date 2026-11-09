# Evals for the support bot in src/support.cpp. The worker builds its client once and answers every test case.
# Build first: cmake -B build && cmake --build build
worker build/evalworker

test_case "refund after the window" "Can I still return my shoes? They arrived 40 days ago."
  target src/support.cpp#answer
  matches "30[- ]day"   # "30 days" and "30-day window" are both right
  judge "Says a refund isn't possible anymore, politely"

test_case "missing package" "My package never showed up and I'm getting really annoyed."
  target src/support.cpp#answer
  matches "order (number|id|#)"
  judge "Asks for the order number before promising a refund or resend"

test_case "refund inside the window" "My shoes arrived last week and they don't fit. Can I send them back?"
  target src/support.cpp#answer
  judge "Says they can return them for a refund"
