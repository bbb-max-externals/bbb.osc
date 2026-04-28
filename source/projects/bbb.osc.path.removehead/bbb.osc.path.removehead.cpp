#include "c74_min.h"
#include "osc_path.hpp"

using namespace c74::min;

class bbb_osc_path_removehead : public object<bbb_osc_path_removehead> {
public:
    MIN_DESCRIPTION{"Remove the first N components from an OSC path"};
    MIN_TAGS{"osc, path"};
    MIN_AUTHOR{"ISHII 2bit"};

    inlet<> input{this, "(anything) /a/b/c payload..."};
    outlet<> output{this, "(anything) /c payload..."};

    attribute<int> count{this, "count", 1,
        description{"Number of leading components to remove"},
        setter{MIN_FUNCTION {
            return args;
        }}
    };

    attribute<bool> payload_attr{this, "payload", true,
        description{"Pass through payload"}
    };

    message<> anything_msg{this, "anything", "Remove head components",
        MIN_FUNCTION {
            if(args.size() < 1 || args[0].a_type != c74::max::A_SYM) return {};

            std::string path(static_cast<const char*>(symbol(args[0])));
            std::string result_path = bbb::osc::path::remove_head(path, static_cast<size_t>(std::max(0, static_cast<int>(count))));

            atoms result;
            result.push_back(symbol(result_path));
            if(payload_attr) {
                for(size_t i = 1; i < args.size(); ++i) {
                    result.push_back(args[i]);
                }
            }
            output.send(result);
            return {};
        }
    };
};

MIN_EXTERNAL(bbb_osc_path_removehead);
