#pragma once

#include <cassert>
#include <cstdint>
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

namespace rpc {

    template<class RPC>
    class client {
    public:
        using req_type  = typename RPC::req_type;
        using resp_type = typename RPC::resp_type;
        using err_type  = typename RPC::err_type;

        explicit client(uint16_t port)
            : m_ep(ip::address_v4::loopback(), port)
            , m_sock(m_ios) { }

        explicit client(const char * port)
            : client(boost::lexical_cast<uint16_t>(port)) { }

        long double call(const std::string & method,
                const std::initializer_list<long double> & params) {
            if(! m_is_connected) {
                m_sock.connect(m_ep);
                m_is_connected = true;
            }
            assert(params.size() == 2 && "Invalid number of params");
            make_call(method, params);
            return get_result();
        }

        void stop() {
            if(m_is_connected) {
                m_sock.shutdown(tcp::socket::shutdown_both);
                m_sock.close();
                m_is_connected = false;
            }
        }

    private:
        // TODO: move this and server::write_to_buf to a base class
        void write_to_buff(std::stringstream & ss) {
            std::string str_res(ss.str());
            m_curr_data_len = (max_len > str_res.size())
                              ? str_res.size() : max_len;
            for(uint16_t i = 0; i < m_curr_data_len; ++i) {
                m_data[i] = str_res[i];
            }
        }

        void make_call(const std::string & method,
                const std::initializer_list<long double> & params) {
            req_type req;
            req.m_id = ++m_msg_id;
            req.m_method = method;
            req.m_params = params;
            req.save();

            write_to_buff(req.m_ss);

            m_sock.write_some(buffer(m_data, m_curr_data_len));
            m_data.fill('\0');
        }

        long double get_result() {
            uint16_t len = m_sock.read_some(buffer(m_data));
            resp_type resp;
            std::stringstream ss(std::string(m_data.data()));
            try {
                resp.load(ss);
            } catch (const std::exception & e) {
                err_type err;
                try {
                    ss.clear();
                    ss.seekg(0, std::ios::beg);
                    err.load(ss);
                } catch (const std::exception & e) {
                    std::string msg("Can't parse response: ");
                    throw std::runtime_error(msg + e.what());
                }
                throw std::runtime_error(err.m_err);
            }
            return resp.m_result;
        }

        enum : uint16_t { max_len = 1024U };
        std::array<char, max_len> m_data;
        io_service m_ios;
        tcp::endpoint m_ep;
        tcp::socket m_sock;
        uint64_t m_msg_id = 0;
        uint16_t m_curr_data_len = 0;
        bool m_is_connected = false;
    };

}