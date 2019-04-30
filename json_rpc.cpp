#include "json_rpc.hpp"

#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace rpc {

    namespace json {

        void json_req::load(std::istream & buffer) {
            using boost::property_tree::ptree;
            read_json(buffer, m_pt);
            m_version = m_pt.get("jsonrpc", "2.0");
            m_id = m_pt.get<uint64_t>("id");
            m_method = m_pt.get<std::string>("method");
            BOOST_FOREACH(ptree::value_type & v, m_pt.get_child("params")) {
                m_params.push_back(v.second.get_value<long double>());
            }
        }
        void json_req::save() {
            using boost::property_tree::ptree;
            ptree children;
            m_pt.put("jsonrpc", m_version);
            m_pt.put("id", m_id);
            m_pt.put("method", m_method);
            for(auto & param : m_params) {
                ptree child;
                child.put("", param);
                children.push_back(std::make_pair("", std::move(child)));
            }
            m_pt.add_child("params", children);
            write_json(m_ss, m_pt, false);
        }

        void json_resp::load(std::istream & buffer) {
            read_json(buffer, m_pt);
            m_version = m_pt.get("jsonrpc", "2.0");
            m_id = m_pt.get<uint64_t>("id");
            m_result = m_pt.get<long double>("result");
        }
        void json_resp::save() {
            m_pt.put("jsonrpc", m_version);
            m_pt.put("id", m_id);
            m_pt.put("result", m_result);
            write_json(m_ss, m_pt, false);
        }

        void json_err::load(std::istream & buffer) {
            read_json(buffer, m_pt);
            m_version = m_pt.get("jsonrpc", "2.0");
            m_id = m_pt.get<uint64_t>("id");
            m_err = m_pt.get<std::string>("error");
        }
        void json_err::save() {
            m_pt.put("jsonrpc", m_version);
            m_pt.put("id", m_id);
            m_pt.put("error", m_err);
            write_json(m_ss, m_pt, false);
        }

    }

}