#include "c74_min.h"
#include "osc_path.hpp"

using namespace c74::min;

class bbb_osc_path_route : public object<bbb_osc_path_route> {
public:
    MIN_DESCRIPTION{"Route messages by OSC path"};
    MIN_TAGS{"osc, path, route"};
    MIN_AUTHOR{"ISHII 2bit"};

    inlet<> input{this, "(anything) /path payload..."};

    attribute<int> mode{this, "mode", 0,
        description{"0=exact, 1=prefix"},
        enum_map{"exact", "prefix"}
    };

    attribute<bool> strip{this, "strip", false,
        description{"Strip matched prefix from path (prefix mode only)"}
    };

    message<> anything_msg{this, "anything", "Route by OSC path",
        MIN_FUNCTION {
            if(args.size() < 1 || args[0].a_type != c74::max::A_SYM) {
                send_nomatch(args);
                return {};
            }

            std::string path(static_cast<const char*>(symbol(args[0])));

            for(size_t i = 0; i < routes_.size(); ++i) {
                const auto& route = routes_[i];
                bool matched = false;
                if(mode == 0) {
                    matched = (bbb::osc::path::normalize(path) == bbb::osc::path::normalize(route));
                } else {
                    matched = bbb::osc::path::starts_with_component(path, route);
                }
                if(matched) {
                    if(strip && mode == 1) {
                        std::string remaining = bbb::osc::path::strip_prefix(path, route);
                        atoms result;
                        result.push_back(symbol(remaining));
                        for(size_t j = 1; j < args.size(); ++j) {
                            result.push_back(args[j]);
                        }
                        outlets_[i]->send(result);
                    } else if(mode == 0) {
                        atoms result;
                        for(size_t j = 1; j < args.size(); ++j) {
                            result.push_back(args[j]);
                        }
                        outlets_[i]->send(result);
                    } else {
                        outlets_[i]->send(args);
                    }
                    return {};
                }
            }
            send_nomatch(args);
            return {};
        }
    };

    message<> dump_msg{this, "dump", "Print route configuration",
        MIN_FUNCTION {
            cout << "bbb.osc.path.route routes:" << endl;
            for(size_t i = 0; i < routes_.size(); ++i) {
                cout << "  " << i << ": " << routes_[i] << " -> outlet " << i << endl;
            }
            cout << "  unmatched -> outlet " << routes_.size() << endl;
            return {};
        }
    };

    bbb_osc_path_route(const atoms& args = {}) {
        for(size_t i = 0; i < args.size(); ++i) {
            if(args[i].a_type == c74::max::A_SYM) {
                routes_.push_back(static_cast<const char*>(c74::min::symbol(args[i])));
            }
        }
        for(size_t i = 0; i < routes_.size(); ++i) {
            outlets_.push_back(std::make_unique<outlet<>>(this, "(" + routes_[i] + ")"));
        }
        outlets_.push_back(std::make_unique<outlet<>>(this, "(unmatched)"));
    }

private:
    std::vector<std::string> routes_;
    std::vector<std::unique_ptr<outlet<>>> outlets_;

    void send_nomatch(const atoms& args) {
        outlets_.back()->send(args);
    }
};

MIN_EXTERNAL(bbb_osc_path_route);
