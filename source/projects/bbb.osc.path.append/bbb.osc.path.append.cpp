#include "c74_min.h"
#include "osc_path.hpp"

using namespace c74::min;

class bbb_osc_path_append : public object<bbb_osc_path_append> {
public:
    MIN_DESCRIPTION{"Append components to an OSC path"};
    MIN_TAGS{"osc, path"};
    MIN_AUTHOR{"ISHII 2bit"};

    inlet<> input{this, "(anything) /path payload..."};
    outlet<> output{this, "(anything) /path/appended payload..."};

    attribute<bool> normalize_attr{this, "normalize", true,
        description{"Normalize resulting path"}
    };

    attribute<bool> payload_attr{this, "payload", true,
        description{"Pass through payload"}
    };

    message<> anything_msg{this, "anything", "Append components to path",
        MIN_FUNCTION {
            if(args.size() < 2) return {};

            if(args[0].a_type != c74::max::A_SYM) return {};
            std::string path(static_cast<const char*>(symbol(args[0])));

            std::string appended;
            for(size_t i = 1; i < args.size(); ++i) {
                if(args[i].a_type == c74::max::A_SYM) {
                    std::string comp(static_cast<const char*>(symbol(args[i])));
                    if(bbb::osc::path::is_path_like(comp)) {
                        path = bbb::osc::path::append(path, comp);
                    } else {
                        atoms result;
                        result.push_back(symbol(bbb::osc::path::append(path, comp)));
                        if(payload_attr) {
                            for(size_t j = i; j < args.size(); ++j) {
                                if(j == i) continue;
                                result.push_back(args[j]);
                            }
                        }
                        output.send(result);
                        return {};
                    }
                } else {
                    atoms result;
                    result.push_back(symbol(bbb::osc::path::normalize(path)));
                    if(payload_attr) {
                        for(size_t j = i; j < args.size(); ++j) {
                            result.push_back(args[j]);
                        }
                    }
                    output.send(result);
                    return {};
                }
            }
            atoms result;
            result.push_back(symbol(bbb::osc::path::normalize(path)));
            output.send(result);
            return {};
        }
    };
};

MIN_EXTERNAL(bbb_osc_path_append);
