# bbb.osc — TODO

## Setup
- [x] Create SPEC.md
- [x] Create README.md
- [x] Create PLAN.md
- [x] Create TODO.md
- [ ] Initialize git submodules (min-api, bbb-osc)
- [ ] Create root CMakeLists.txt
- [ ] Create cmake/generate_version.cmake
- [ ] Create package-info.json

## bbb.osc.send
- [ ] Create source/projects/bbb.osc.send/CMakeLists.txt
- [ ] Implement bbb.osc.send.cpp
  - [ ] NIL macro workaround
  - [ ] host/port attributes with setter
  - [ ] long/double attributes
  - [ ] send message handler
  - [ ] anything message handler
  - [ ] Type coercion logic (atoms → bbb::osc::message)

## bbb.osc.receive
- [ ] Create source/projects/bbb.osc.receive/CMakeLists.txt
- [ ] Implement bbb.osc.receive.cpp
  - [ ] NIL macro workaround
  - [ ] broadcast_receiver class
  - [ ] receiver_registry singleton
  - [ ] port/bind_ip attributes with setter
  - [ ] Init timer pattern
  - [ ] Periodic update timer
  - [ ] Message → Max atoms conversion
  - [ ] Left outlet output
  - [ ] Right outlet output
  - [ ] close message handler
  - [ ] Destructor cleanup

## Verification
- [ ] Build succeeds
- [ ] Manual test in Max
