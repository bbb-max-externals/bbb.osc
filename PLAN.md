# bbb.osc — Implementation Plan

## Architecture

```
bbb.osc/
├── CMakeLists.txt                  # Root CMake: add_subdirectory for each external
├── cmake/
│   └── generate_version.cmake      # Git commit count → version.h
├── deps/
│   ├── min-api/                    # git submodule (max-sdk-base included)
│   └── bbb-osc/                    # git submodule (--recursive)
├── source/
│   ├── bbb/
│   │   └── version.h               # Generated
│   └── projects/
│       ├── bbb.osc.send/
│       │   ├── CMakeLists.txt
│       │   └── bbb.osc.send.cpp
│       └── bbb.osc.receive/
│           ├── CMakeLists.txt
│           └── bbb.osc.receive.cpp
├── externals/                      # Build output (.mxo)
├── help/
└── package-info.json
```

## Implementation Steps

### Phase 1: Project Setup

1. Initialize git repo (already done)
2. Add submodules: `deps/min-api`, `deps/bbb-osc`
3. Create root `CMakeLists.txt`
4. Create `cmake/generate_version.cmake`
5. Create `package-info.json`

### Phase 2: bbb.osc.send

1. Create `source/projects/bbb.osc.send/CMakeLists.txt`
2. Implement `bbb.osc.send.cpp`:
   - NIL macro workaround (`#pragma push_macro("NIL")` / `#undef NIL` before bbb-osc includes)
   - `host` / `port` attributes with setter to re-setup sender
   - `long` / `double` attributes
   - `send` message handler: parse atoms → build `bbb::osc::message` with type coercion
   - `anything` message handler: selector as OSC address
   - Use `bbb::osc::sender` directly (no shared socket needed for sending)

### Phase 3: bbb.osc.receive

1. Create `source/projects/bbb.osc.receive/CMakeLists.txt`
2. Implement `bbb.osc.receive.cpp`:
   - NIL macro workaround
   - `broadcast_receiver` class extending `bbb::osc::receiver`
   - `receiver_registry` singleton managing shared receivers by `(port, bind_ip)`
   - `port` / `bind_ip` attributes with setter callbacks
   - Init timer pattern (SKILL.md: attributes not set during constructor)
   - Periodic update timer for polling `broadcast_receiver` on main thread
   - Callback: convert `bbb::osc::message` → Max atoms with type coercion (int→long, float→double)
   - Left outlet: OSC address as selector + converted args
   - Right outlet: `[ sender_ip, sender_port, receiver_ip, receiver_port ]` as list
   - `close` message handler
   - Destructor: unregister callback, close shared receiver if last user

### Phase 4: Build & Verify

1. `cmake .. && cmake --build .`
2. Fix any compilation errors
3. Manual test in Max

## Key Technical Notes

### NIL Macro Conflict

Max SDK defines `#define NIL ((void*)0)` in `ext_mess.h`. OSCpp uses `NIL` as an enum value.
Workaround:
```cpp
#include "c74_min.h"
#pragma push_macro("NIL")
#undef NIL
#include <bbb/osc.hpp>
```

### CMake: add_library between pre/post target

min-api's `min-pretarget.cmake` sets up project and flags but does NOT create the target.
`min-posttarget.cmake` expects the target to exist. Must call `add_library()` between them:
```cmake
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)
add_library(${PROJECT_NAME} MODULE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${C74_INCLUDES})
# ... other target settings ...
include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)
```

### attribute<symbol> → std::string

`attribute<symbol>` cannot be directly converted to `std::string`. Use a helper:
```cpp
static std::string to_string(const symbol& s) {
    return std::string((const char*)s);
}
```

### Atom type dispatch

min-api atoms use Max SDK types directly:
```cpp
if(arg.a_type == c74::max::A_LONG) { ... }
else if(arg.a_type == c74::max::A_FLOAT) { ... }
else if(arg.a_type == c74::max::A_SYM) { ... }
```

### Main Thread Only for Outlets

bbb-osc's receiver runs a background thread. Outlet output must happen on Max's main thread.
Use `c74::min::timer<defer_delivery>` to poll and output.

### Attribute Initialization Order

Attributes are set after construction. Use init timer:
```cpp
c74::min::timer<> m_init_timer{this, MIN_FUNCTION { init(); return {}; }};
// In constructor: m_init_timer.delay(0);
```

### Thread Safety

- `bbb::osc::receiver::queued_messages` is a `threaded_queue` (mutex-based, NOT lock-free)
- Use `receive()` (non-blocking) instead of `try_receive(msg, 0)` to avoid condition_variable overhead
- Background thread only touches `queued_messages` — never acquires registry or callback mutexes
- All Max object operations (timer callbacks, destructors, attribute setters) run on the main thread

### Shutdown / Close considerations

- `close()` must call `m_poll_timer.stop()` to prevent timer firing after receiver is null
- `rebind()` must restart poll timer with `m_poll_timer.delay(1)` after setting up new receiver
- `receiver_registry::release()` must NOT hold registry mutex during thread join:
  ```cpp
  // Extract shared_ptr from map → release mutex → shared_ptr goes out of scope → close/join
  std::shared_ptr<broadcast_receiver> to_destroy;
  {
      auto lock = std::lock_guard<std::mutex>(mtx_);
      // find and extract...
      receivers_.erase(it);
  }
  // to_destroy destroyed here, outside mutex: close() + join()
  ```
- `bbb::udp::receiver::close()` closes socket then joins background thread. Since socket is non-blocking,
  the thread exits within ~1μs after `sock.close()` (loop checks `sock.is_open()`)
- `bbb::udp::receiver::~receiver()` handles double-close safely (checks `is_open()` and `joinable()`)

### min-api submodules

min-api itself has submodules (max-sdk-base, readerwriterqueue, mock). Must run:
```bash
cd deps/min-api && git submodule update --init --recursive
```
