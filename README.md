# bbb.osc

OSC send/receive externals for Max/MSP.

Built with [min-api](https://github.com/Cycling74/min-api) and [bbb-osc](https://github.com/2bbb/bbb-osc).

## Externals

### bbb.osc.send

Sends OSC messages over UDP.

```
[send /foo 1 2.5 hello]
|                |
|   bbb.osc.send @host 192.168.1.10 @port 9000 @long 1 @double 1
```

- `@host` — destination IP (default: `127.0.0.1`)
- `@port` — destination port (default: `9000`)
- `@long` — send integers as OSC Int64 (default: `0`)
- `@double` — send floats as OSC Double (default: `0`)

### bbb.osc.receive

Receives OSC messages over UDP. Left outlet outputs received messages, right outlet outputs source/destination info.

```
bbb.osc.receive @port 9000 @bind_ip 0.0.0.0
|                |
|  /foo 1 2.5    |  list 192.168.1.10 12345 0.0.0.0 9000
```

- `@port` — bind port (default: `9000`)
- `@bind_ip` — bind interface IP (default: `0.0.0.0`)
- Left outlet: OSC address as selector, arguments follow
- Right outlet: `[ sender_ip, sender_port, receiver_ip, receiver_port ]`

## Build

```bash
git clone --recursive https://github.com/bbb-max-externals/bbb.osc.git
cd bbb.osc
mkdir build && cd build
cmake ..
cmake --build .
```

Output: `externals/bbb.osc.send.mxo` and `externals/bbb.osc.receive.mxo`

## Install

Copy the entire `bbb.osc` directory to `~/Documents/Max 8/Packages/` (or your preferred Packages folder).

## License

MIT License

## Third-party Libraries

| Library | License | Author |
|---------|---------|--------|
| [min-api](https://github.com/Cycling74/min-api) | MIT | Cycling74 |
| [bbb-osc](https://github.com/2bbb/bbb-osc) | MIT | ISHII 2bit |
| [Asio](https://github.com/chriskohlhoff/asio) (standalone) | BSL-1.0 | Christopher Kohlhoff |
| [oscpp](https://github.com/kronihk/oscpp) | MIT | Stefan Kersten |
| [bit_by_bit](https://github.com/2bbb/bit_by_bit) | MIT | ISHII 2bit |
