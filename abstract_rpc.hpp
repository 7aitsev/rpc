#ifndef RPC_ABSTRACT_RPC_HPP
#define RPC_ABSTRACT_RPC_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>

namespace rpc {
    
    class i_abstract_rpc {
    public:
        boost::property_tree::ptree m_pt;
        std::stringstream m_ss;
        uint64_t m_id;
        
        virtual void load(std::istream & buffer) = 0;
        virtual void save() = 0;
    };
    
    class abstract_req : public i_abstract_rpc {
    public:
        std::string m_method;
        std::vector<long double> m_params;
    };
    
    class abstract_resp : public i_abstract_rpc {
    public:
        long double m_result;
        std::string m_err;
    };
    
    class abstract_err : public abstract_resp {
    };
    
    class abstract_rpc {
    public:
        using req_type  = abstract_req;
        using resp_type = abstract_resp;
        using err_type  = abstract_err;
    };
    
}
#endif //RPC_ABSTRACT_RPC_HPP
