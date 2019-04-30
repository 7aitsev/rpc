#pragma once

#include "abstract_rpc.hpp"

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

        class json_err : public abstract_err, public i_json_rpc {
        public:
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