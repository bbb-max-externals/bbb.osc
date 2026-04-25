# bbb.osc — Specification

OSC send/receive externals for Max/MSP using [bbb-osc](https://github.com/2bbb/bbb-osc).

## Externals

### bbb.osc.send

OSC message sender. API follows `udpsend` conventions.

#### Inlets

| # | Type | Description |
|---|------|-------------|
| 1 | anything | `send /address arg1 arg2 ...` or `/address arg1 arg2 ...` |

#### Outlets

None.

#### Attributes

| Name | Type | Default | Description |
|------|------|---------|-------------|
| `host` | symbol | `"127.0.0.1"` | Destination IP address |
| `port` | long | `9000` | Destination UDP port |
| `long` | bool | `false` | When ON: Max integers are sent as OSC Int64 (h). OFF: Int32 (i) |
| `double` | bool | `false` | When ON: Max floats are sent as OSC Double (d). OFF: Float (f) |

#### Messages

- `send /address args...` — Send OSC message with given address and arguments
- `anything` — Selector is treated as OSC address, rest as arguments (udpsend convention)

#### Type mapping (inlet → OSC)

| Max type | @long OFF | @long ON |
|----------|-----------|----------|
| long | Int32 (i) | Int64 (h) |

| Max type | @double OFF | @double ON |
|----------|-------------|------------|
| double | Float (f) | Double (d) |

Strings and symbols → OSC String (s).

---

### bbb.osc.receive

OSC message receiver. API follows `udpreceive` conventions.

#### Inlets

None. Parameters configured via attributes.

#### Outlets

| # | Type | Description |
|---|------|-------------|
| 1 | anything | OSC address as selector, arguments follow (udpreceive convention) |
| 2 | list | `[ sender_ip, sender_port, receiver_ip, receiver_port ]` on each received message |

#### Attributes

| Name | Type | Default | Description |
|------|------|---------|-------------|
| `port` | long | `9000` | UDP port to bind |
| `bind_ip` | symbol | `"0.0.0.0"` | Local IP address of interface to bind |

#### Messages

- `close` — Close the socket and stop receiving

#### Type mapping (OSC → Max)

| OSC type | Max output |
|----------|-----------|
| Int32 (i), Int64 (h), Char (c) | long |
| Float (f), Double (d) | double |
| True (T), False (F) | long (1/0) |
| String (s), Symbol (S) | symbol |
| Nil (N), Impulse (I) | selector only (no args) |
| Blob (b) | list of bytes |
| Midi4 (m) | list of 4 longs |
| RGBA (r) | list of 4 longs |

#### Right outlet format

```
list <sender_ip:symbol> <sender_port:long> <receiver_ip:symbol> <receiver_port:long>
```

---

## Socket sharing (bbb.osc.receive)

Multiple `bbb.osc.receive` instances sharing the same `(port, bind_ip)` pair share a single UDP socket internally.

- A `receiver_registry` singleton manages `shared_ptr<broadcast_receiver>` keyed by `(port, bind_ip)`
- Each Max instance registers a callback on the shared receiver
- Background thread receives into `queued_messages` (thread-safe)
- Any instance's timer triggers `broadcast_update()` → all registered callbacks receive all messages
- When all instances on a `(port, ip)` pair are destroyed, the socket is closed and removed from registry

## Dependencies

- [bbb-osc](https://github.com/2bbb/bbb-osc) (header-only, includes asio, oscpp, bit_by_bit)
- [min-api](https://github.com/Cycling74/min-api) (Max external development framework)
