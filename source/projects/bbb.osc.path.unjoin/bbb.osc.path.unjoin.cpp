#include "c74_min.h"
#include "osc_path.hpp"

using namespace c74::min;

class bbb_osc_path_unjoin : public object<bbb_osc_path_unjoin> {
public:
    MIN_DESCRIPTION{"Split an OSC path into separate component atoms"};
    MIN_TAGS{"osc, path"};
    MIN_AUTHOR{"ISHII 2bit"};

    inlet<> input{this, "(anything) /path1/path2 payload..."};
    outlet<> output{this, "(anything) /path1 /path2 payload..."};

    attribute<bool> slash{this, "slash", true,
        description{"Include leading slash on split components"}
    };

    attribute<bool> payload_attr{this, "payload", true,
        description{"Pass through payload after split components"}
    };

    message<> anything_msg{this, "anything", "Unjoin OSC path into components",
        MIN_FUNCTION {
            if(args.size() < 1 || args[0].a_type != c74::max::A_SYM) return {};

            std::string path(static_cast<const char*>(symbol(args[0])));
            auto parts = bbb::osc::path::split(path);

            atoms result;
            for(const auto& p : parts) {
                result.push_back(symbol(slash ? ("/" + p) : p));
            }
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

MIN_EXTERNAL(bbb_osc_path_unjoin);
