# bbb.osc — TODO

## Setup
- [x] Create SPEC.md
- [x] Create README.md
- [x] Create PLAN.md
- [x] Create TODO.md
- [x] Initialize git submodules (min-api, bbb-osc)
- [x] Create root CMakeLists.txt
- [x] Create cmake/generate_version.cmake
- [x] Create package-info.json

## bbb.osc.send
- [x] Create source/projects/bbb.osc.send/CMakeLists.txt
- [x] Implement bbb.osc.send.cpp
  - [x] NIL macro workaround
  - [x] host/port attributes with setter
  - [x] long/double attributes
  - [x] send message handler
  - [x] anything message handler
  - [x] Type coercion logic (atoms → bbb::osc::message)

## bbb.osc.receive
- [x] Create source/projects/bbb.osc.receive/CMakeLists.txt
- [x] Implement bbb.osc.receive.cpp
  - [x] NIL macro workaround
  - [x] broadcast_receiver class
  - [x] receiver_registry singleton
  - [x] port/bind_ip attributes with setter
  - [x] Init timer pattern
  - [x] Periodic update timer
  - [x] Message → Max atoms conversion
  - [x] Left outlet output
  - [x] Right outlet output
  - [x] close message handler
  - [x] Destructor cleanup

## Verification
- [x] Build succeeds (both .mxo generated)
- [ ] Manual test in Max
