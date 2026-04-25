#include "c74_min.h"

#pragma push_macro("NIL")
#undef NIL
#include <bbb/osc.hpp>

#include <memory>

using namespace c74::min;

static std::string to_string(const symbol& s) {
    return std::string((const char*)s);
}

class bbb_osc_send : public object<bbb_osc_send> {
public:
    MIN_DESCRIPTION{"Send OSC messages over UDP"};
    MIN_TAGS{"osc, udp, network"};
    MIN_AUTHOR{"ISHII 2bit"};

    inlet<> input{this, "(anything) send /address args..."};

    attribute<symbol> host{this, "host", "127.0.0.1",
        description{"Destination IP address"},
        setter{[this](const atoms& args, int) -> atoms {
            if(args.size() > 0) {
                setup_sender(to_string(symbol(args[0])), static_cast<int>(port));
            }
            return args;
        }}
    };

    attribute<int> port{this, "port", 9000,
        description{"Destination UDP port"},
        setter{[this](const atoms& args, int) -> atoms {
            if(args.size() > 0) {
                setup_sender(to_string(host), static_cast<int>(args[0]));
            }
            return args;
        }}
    };

    attribute<bool> use_long{this, "long", false,
        description{"When ON: send integers as OSC Int64 (h). OFF: Int32 (i)"}
    };

    attribute<bool> use_double{this, "double", false,
        description{"When ON: send floats as OSC Double (d). OFF: Float (f)"}
    };

    message<> send_msg{this, "send", "Send an OSC message: send /address args...",
        MIN_FUNCTION {
            if(args.size() < 1) return {};
            send_osc(args[0], atoms(args.begin() + 1, args.end()));
            return {};
        }
    };

    message<> anything_msg{this, "anything", "Send using selector as OSC address",
        MIN_FUNCTION {
            send_osc(args[0], atoms(args.begin() + 1, args.end()));
            return {};
        }
    };

    message<> dump_msg{this, "dump", "Print current status to console",
        MIN_FUNCTION {
            cout << "bbb.osc.send status:" << endl;
            cout << "  host: " << to_string(host) << endl;
            cout << "  port: " << port << endl;
            cout << "  @long: " << (use_long ? "on" : "off") << endl;
            cout << "  @double: " << (use_double ? "on" : "off") << endl;
            return {};
        }
    };

private:
    std::shared_ptr<bbb::osc::sender> sender_;

    void setup_sender(const std::string& h, int p) {
        try {
            sender_ = std::make_shared<bbb::osc::sender>();
            sender_->setup(h, static_cast<std::uint16_t>(p));
        } catch(std::exception& e) {
            cerr << "bbb.osc.send: setup error: " << e.what() << endl;
        }
    }

    void send_osc(const atom& addr_atom, const atoms& args) {
        if(!sender_) {
            setup_sender(to_string(host), static_cast<int>(port));
        }
        if(!sender_) return;

        std::string address = to_string(symbol(addr_atom));
        if(address.empty() || address[0] != '/') {
            cerr << "bbb.osc.send: invalid OSC address: " << address << endl;
            return;
        }

        bbb::osc::message mess(address);
        for(const auto& arg : args) {
            if(arg.a_type == c74::max::A_LONG) {
                if(use_long) {
                    mess.push(bbb::osc::Tag::Int64, static_cast<std::int64_t>(static_cast<int>(arg)));
                } else {
                    mess.push(bbb::osc::Tag::Int32, static_cast<std::int32_t>(static_cast<int>(arg)));
                }
            } else if(arg.a_type == c74::max::A_FLOAT) {
                if(use_double) {
                    mess.push(bbb::osc::Tag::Double, static_cast<double>(arg));
                } else {
                    mess.push(bbb::osc::Tag::Float, static_cast<float>(static_cast<double>(arg)));
                }
            } else if(arg.a_type == c74::max::A_SYM) {
                mess.push(bbb::osc::Tag::String, to_string(symbol(arg)));
            }
        }
        sender_->send(mess);
    }
};

MIN_EXTERNAL(bbb_osc_send);
