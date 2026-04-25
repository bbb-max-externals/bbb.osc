#include "c74_min.h"

#pragma push_macro("NIL")
#undef NIL
#include <bbb/osc.hpp>

#include <memory>
#include <mutex>
#include <map>
#include <vector>
#include <functional>
#include <utility>

using namespace c74::min;

class broadcast_receiver : public bbb::osc::receiver {
public:
    void add_callback(std::function<void(const bbb::osc::message&)> cb) {
        auto lock = std::lock_guard<std::mutex>(mtx_);
        callbacks_.push_back(std::move(cb));
    }

    void remove_callback(void* owner) {
        auto lock = std::lock_guard<std::mutex>(mtx_);
        callbacks_.erase(
            std::remove_if(callbacks_.begin(), callbacks_.end(),
                [owner](const auto& cb) { return cb.owner == owner; }),
            callbacks_.end()
        );
    }

    std::size_t callback_count() const {
        auto lock = std::lock_guard<std::mutex>(mtx_);
        return callbacks_.size();
    }

    void broadcast_update() {
        std::vector<bbb::osc::message> messages;
        {
            bbb::osc::message mess;
            while(queued_messages.try_receive(mess)) {
                messages.push_back(std::move(mess));
            }
        }
        auto lock = std::lock_guard<std::mutex>(mtx_);
        for(const auto& mess : messages) {
            for(const auto& cb : callbacks_) {
                cb.fn(mess);
            }
        }
    }

    struct tagged_callback {
        void* owner;
        std::function<void(const bbb::osc::message&)> fn;
    };

private:
    mutable std::mutex mtx_;
    std::vector<tagged_callback> callbacks_;
};

struct receiver_key {
    std::uint16_t port;
    std::string bind_ip;
    bool operator<(const receiver_key& rhs) const {
        if(bind_ip == rhs.bind_ip) return port < rhs.port;
        return bind_ip < rhs.bind_ip;
    }
};

class receiver_registry {
public:
    static receiver_registry& shared() {
        static receiver_registry instance;
        return instance;
    }

    std::shared_ptr<broadcast_receiver> get(std::uint16_t port, const std::string& bind_ip) {
        auto lock = std::lock_guard<std::mutex>(mtx_);
        receiver_key key{port, bind_ip};
        auto it = receivers_.find(key);
        if(it != receivers_.end()) {
            return it->second;
        }
        auto recv = std::make_shared<broadcast_receiver>();
        auto res = recv->bbb::osc::receiver::setup(port, bind_ip);
        if(!res) return nullptr;
        receivers_.insert(std::make_pair(key, recv));
        return recv;
    }

    void release(std::uint16_t port, const std::string& bind_ip) {
        auto lock = std::lock_guard<std::mutex>(mtx_);
        receiver_key key{port, bind_ip};
        auto it = receivers_.find(key);
        if(it != receivers_.end() && it->second->callback_count() == 0) {
            it->second->close();
            receivers_.erase(it);
        }
    }

private:
    receiver_registry() = default;
    std::mutex mtx_;
    std::map<receiver_key, std::shared_ptr<broadcast_receiver>> receivers_;
};

class bbb_osc_receive : public object<bbb_osc_receive> {
public:
    MIN_DESCRIPTION{"Receive OSC messages over UDP"};
    MIN_TAGS{"osc, udp, network"};
    MIN_AUTHOR{"ISHII 2bit"};

    outlet<> osc_out{this, "(anything) received OSC messages"};
    outlet<> info_out{this, "(list) sender info: sender_ip sender_port receiver_ip receiver_port"};

    attribute<int> port{this, "port", 9000,
        description{"UDP port to bind"},
        setter{[this](const atoms& args, int) -> atoms {
            if(args.size() > 0) {
                rebind(static_cast<int>(args[0]), std::string(bind_ip));
            }
            return args;
        }}
    };

    attribute<symbol> bind_ip{this, "bind_ip", "0.0.0.0",
        description{"Local IP address of interface to bind"},
        setter{[this](const atoms& args, int) -> atoms {
            if(args.size() > 0) {
                rebind(static_cast<int>(port), std::string(args[0]));
            }
            return args;
        }}
    };

    message<> close_msg{this, "close", "Close the socket",
        MIN_FUNCTION {
            close();
            return {};
        }
    };

    bbb_osc_receive() {
        m_init_timer.delay(0);
    }

    ~bbb_osc_receive() {
        close();
    }

private:
    std::shared_ptr<broadcast_receiver> receiver_;
    std::vector<bbb::osc::message> pending_;
    std::mutex pending_mtx_;

    timer<timer_options::defer_delivery> m_init_timer{this,
        MIN_FUNCTION {
            init();
            return {};
        }
    };

    timer<timer_options::defer_delivery> m_poll_timer{this,
        MIN_FUNCTION {
            poll();
            m_poll_timer.delay(1);
            return {};
        }
    };

    void init() {
        rebind(static_cast<int>(port), std::string(bind_ip));
        m_poll_timer.delay(1);
    }

    void rebind(int p, const std::string& ip) {
        close();
        auto recv = receiver_registry::shared().get(static_cast<std::uint16_t>(p), ip);
        if(!recv) {
            cerr << "bbb.osc.receive: failed to bind " << ip << ":" << p << endl;
            return;
        }
        receiver_ = recv;
        auto* self = this;
        receiver_->add_callback({self, [self](const bbb::osc::message& mess) {
            auto lock = std::lock_guard<std::mutex>(self->pending_mtx_);
            self->pending_.push_back(mess);
        }});
    }

    void close() {
        if(receiver_) {
            receiver_->remove_callback(this);
            auto p = static_cast<std::uint16_t>(static_cast<int>(port));
            auto ip = std::string(bind_ip);
            receiver_.reset();
            receiver_registry::shared().release(p, ip);
        }
    }

    void poll() {
        if(!receiver_) return;
        receiver_->broadcast_update();
        std::vector<bbb::osc::message> msgs;
        {
            auto lock = std::lock_guard<std::mutex>(pending_mtx_);
            msgs.swap(pending_);
        }
        for(const auto& mess : msgs) {
            output_message(mess);
        }
    }

    void output_message(const bbb::osc::message& mess) {
        atoms info;
        info.push_back(atom(mess.received_host));
        info.push_back(atom(static_cast<int>(mess.received_port)));
        info.push_back(atom(mess.binded_ip));
        info.push_back(atom(static_cast<int>(mess.waiting_port)));

        if(mess.size() == 0) {
            osc_out.send(mess.address);
        } else {
            atoms osc_args;
            for(std::size_t i = 0; i < mess.size(); i++) {
                const auto& arg = mess[i];
                switch(arg.tag) {
                    case bbb::osc::Tag::Int32:
                        osc_args.push_back(atom(static_cast<int>(arg.num.i)));
                        break;
                    case bbb::osc::Tag::Int64:
                        osc_args.push_back(atom(static_cast<int>(arg.num.l)));
                        break;
                    case bbb::osc::Tag::Char:
                        osc_args.push_back(atom(static_cast<int>(arg.num.c)));
                        break;
                    case bbb::osc::Tag::Float:
                        osc_args.push_back(atom(static_cast<double>(arg.num.f)));
                        break;
                    case bbb::osc::Tag::Double:
                        osc_args.push_back(atom(arg.num.d));
                        break;
                    case bbb::osc::Tag::True:
                        osc_args.push_back(atom(1));
                        break;
                    case bbb::osc::Tag::False:
                        osc_args.push_back(atom(0));
                        break;
                    case bbb::osc::Tag::String:
                    case bbb::osc::Tag::Symbol:
                        osc_args.push_back(atom(arg.str));
                        break;
                    case bbb::osc::Tag::Blob: {
                        for(auto byte : arg.blob) {
                            osc_args.push_back(atom(static_cast<int>(byte)));
                        }
                        break;
                    }
                    case bbb::osc::Tag::Midi4: {
                        osc_args.push_back(atom(static_cast<int>(arg.num.midi.port)));
                        osc_args.push_back(atom(static_cast<int>(arg.num.midi.status)));
                        osc_args.push_back(atom(static_cast<int>(arg.num.midi.data1)));
                        osc_args.push_back(atom(static_cast<int>(arg.num.midi.data2)));
                        break;
                    }
                    case bbb::osc::Tag::RGBA: {
                        osc_args.push_back(atom(static_cast<int>(arg.num.color.r)));
                        osc_args.push_back(atom(static_cast<int>(arg.num.color.g)));
                        osc_args.push_back(atom(static_cast<int>(arg.num.color.b)));
                        osc_args.push_back(atom(static_cast<int>(arg.num.color.a)));
                        break;
                    }
                    default:
                        break;
                }
            }
            osc_args.insert(osc_args.begin(), atom(symbol(mess.address)));
            osc_out.send(osc_args);
        }
        info_out.send(info);
    }
};

MIN_EXTERNAL(bbb_osc_receive);
