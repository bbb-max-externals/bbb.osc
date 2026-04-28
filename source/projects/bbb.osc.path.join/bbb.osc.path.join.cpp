#include "c74_min.h"
#include "osc_path.hpp"

using namespace c74::min;

class bbb_osc_path_join : public object<bbb_osc_path_join> {
public:
    MIN_DESCRIPTION{"Join multiple leading OSC path components into one"};
    MIN_TAGS{"osc, path"};
    MIN_AUTHOR{"ISHII 2bit"};

    inlet<> input{this, "(anything) /comp1 /comp2 ... /compN payload..."};
    outlet<> output{this, "(anything) /joined/path payload..."};

    attribute<int> count{this, "count", 0,
        description{"Number of leading atoms to join. 0 = auto-detect path-like atoms"}
    };

    attribute<bool> normalize_attr{this, "normalize", true,
        description{"Normalize resulting path"}
    };

    attribute<bool> payload_attr{this, "payload", true,
        description{"Pass through payload after joined path"}
    };

    message<> anything_msg{this, "anything", "Join leading path components",
        MIN_FUNCTION {
            if(args.size() < 1) return {};

            std::vector<std::string> components;
            size_t payload_start = args.size();
            int n = static_cast<int>(count);

            for(size_t i = 0; i < args.size(); ++i) {
                if(args[i].a_type != c74::max::A_SYM) {
                    payload_start = i;
                    break;
                }
                std::string s(static_cast<const char*>(symbol(args[i])));
                if(n == 0) {
                    if(!bbb::osc::path::is_path_like(s)) {
                        payload_start = i;
                        break;
                    }
                }
                components.push_back(s);
                if(n > 0 && static_cast<int>(components.size()) >= n) {
                    payload_start = i + 1;
                    break;
                }
            }

            std::string joined = normalize_attr ? bbb::osc::path::join(components) : bbb::osc::path::join(components);

            atoms result;
            result.push_back(symbol(joined));
            if(payload_attr) {
                for(size_t i = payload_start; i < args.size(); ++i) {
                    result.push_back(args[i]);
                }
            }
            output.send(result);
            return {};
        }
    };
};

MIN_EXTERNAL(bbb_osc_path_join);
