#include "c74_min.h"
#include "osc_path.hpp"

using namespace c74::min;

class bbb_osc_path_strip : public object<bbb_osc_path_strip> {
public:
    MIN_DESCRIPTION{"Strip a prefix from an OSC path if it matches"};
    MIN_TAGS{"osc, path"};
    MIN_AUTHOR{"ISHII 2bit"};

    inlet<> input{this, "(anything) /prefix/rest payload..."};
    outlet<> match_out{this, "(anything) /rest payload..."};
    outlet<> nomatch_out{this, "(anything) original message"};

    argument<symbol> prefix_arg{this, "prefix", "Prefix to strip",
        MIN_ARGUMENT_FUNCTION {
            prefix = symbol(arg);
        }
    };

    attribute<symbol> prefix{this, "prefix", "",
        description{"OSC path prefix to strip"}
    };

    attribute<bool> payload_attr{this, "payload", true,
        description{"Pass through payload"}
    };

    message<> anything_msg{this, "anything", "Strip prefix from OSC path",
        MIN_FUNCTION {
            if(args.size() < 1 || args[0].a_type != c74::max::A_SYM) {
                nomatch_out.send(args);
                return {};
            }

            std::string path(static_cast<const char*>(symbol(args[0])));
            symbol pfx_sym = prefix;
            std::string pfx(static_cast<const char*>(pfx_sym));

            if(pfx.empty()) {
                match_out.send(args);
                return {};
            }

            std::string stripped = bbb::osc::path::strip_prefix(path, pfx);
            if(stripped.empty()) {
                nomatch_out.send(args);
                return {};
            }

            atoms result;
            result.push_back(symbol(stripped));
            if(payload_attr) {
                for(size_t i = 1; i < args.size(); ++i) {
                    result.push_back(args[i]);
                }
            }
            match_out.send(result);
            return {};
        }
    };
};

MIN_EXTERNAL(bbb_osc_path_strip);
