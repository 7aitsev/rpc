#ifndef RPC_JSON_RPC_HPP
#define RPC_JSON_RPC_HPP

#include "abstract_rpc.hpp"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace rpc {
    
    namespace json {
        
        class i_json_rpc {
        public:
            std::string m_version = "2.0";
        };
        
        class json_req : public abstract_req, public i_json_rpc {
        public:
            void load(std::istream & buffer) override;
            void save() override;
        };
        
        class json_resp : public abstract_resp, public i_json_rpc {
        public:
            void load(std::istream & buffer) override;
            void save() override;
        };
        
        class json_err : public json_resp {
        public:
            using json_resp::json_resp;
            
            void load(std::istream & buffer) override;
            void save() override;
        };
        
        class json_rpc final : public abstract_rpc {
        public:
            using resp_type = json_resp;
            using err_type  = json_err;
            using req_type  = json_req;
        };
        
    }
    
}

#endif //RPC_JSON_RPC_HPP