#include "c74_min.h"
#include "osc_path.hpp"

using namespace c74::min;

class bbb_osc_path_prefix : public object<bbb_osc_path_prefix> {
public:
    MIN_DESCRIPTION{"Test if an OSC path has a given prefix"};
    MIN_TAGS{"osc, path"};
    MIN_AUTHOR{"ISHII 2bit"};

    inlet<> input{this, "(anything) /path payload..."};
    outlet<> match_out{this, "(anything) matched message"};
    outlet<> nomatch_out{this, "(anything) unmatched message"};

    argument<symbol> prefix_arg{this, "prefix", "Prefix to test",
        MIN_ARGUMENT_FUNCTION {
            prefix = symbol(arg);
        }
    };

    attribute<symbol> prefix{this, "prefix", "",
        description{"OSC path prefix to match"}
    };

    message<> anything_msg{this, "anything", "Test prefix match",
        MIN_FUNCTION {
            if(args.size() < 1 || args[0].a_type != c74::max::A_SYM) {
                nomatch_out.send(args);
                return {};
            }

            std::string path(static_cast<const char*>(symbol(args[0])));
            symbol pfx_sym = prefix;
            std::string pfx(static_cast<const char*>(pfx_sym));

            if(pfx.empty() || bbb::osc::path::starts_with_component(path, pfx)) {
                match_out.send(args);
            } else {
                nomatch_out.send(args);
            }
            return {};
        }
    };
};

MIN_EXTERNAL(bbb_osc_path_prefix);
