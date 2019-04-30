#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "abstract_rpc.hpp"

using namespace boost::interprocess;
using mutex = interprocess_mutex;
using namespace boost::asio;
using namespace boost::asio::ip;
using boost::system::error_code;

namespace rpc {

    template<class RPC> class peer;

    template<class RPC>
    struct shared_data {
        using peer_ptr = boost::shared_ptr<peer<RPC>>;
        using array = std::vector<peer_ptr>;
        using func_type = std::function<double (double, double)>;

        array peers;
        mutex peers_mx;
        std::unordered_map<std::string, func_type> m_methods;
    };

    template<class RPC>
    class peer
        : public boost::enable_shared_from_this<peer<RPC>>
        , boost::noncopyable {
    public:
        using ptr = boost::shared_ptr<peer>;
        using sd_ptr = boost::shared_ptr<shared_data<RPC>>;
        using req_type  = typename RPC::req_type;
        using resp_type = typename RPC::resp_type;
        using err_type  = typename RPC::err_type;

        peer(tcp::socket sock, sd_ptr & sd)
            : m_sock(std::move(sock))
            , m_sd(sd) { }

        void start() {
            m_is_running = true;
            {
                scoped_lock<mutex> lock(m_sd->peers_mx);
                m_sd->peers.push_back(this->shared_from_this());
            }
            recv_req();
        }

        void stop() {
            if(m_is_running) {
                m_is_running = false;
                try {
                    m_sock.shutdown(tcp::socket::shutdown_both);
                    m_sock.close();
                } catch (const std::exception & e) {
                    // log
                }

                ptr self = this->shared_from_this();
                scoped_lock<mutex> lock(m_sd->peers_mx);
                auto it = std::find(
                        m_sd->peers.cbegin(), m_sd->peers.cend(), self);
                m_sd->peers.erase(it);
            }
        }

    private:
        void recv_req() {
            auto self(this->shared_from_this());
            m_sock.async_read_some(buffer(m_data),
                [this, self](error_code ec, std::size_t length) {
                    if (! ec) {
                        if(! m_is_running) return;
                        m_curr_data_len = length;

                        if(! handle_req()) {
                            send_resp();
                        }
                    } else {
                        stop();
                    }
                });
        }

        void send_resp() {
            auto self(this->shared_from_this());
            boost::asio::async_write(m_sock, buffer(m_data, m_curr_data_len),
                 [this, self](error_code ec, std::size_t /*length*/) {
                     if (! ec) {
                         if(! m_is_running) return;
                         m_data.fill(0);
                         recv_req();
                     } else {
                         stop();
                     }
                 });
        }

        void write_to_buff(std::stringstream & ss) {
            std::string str_res(ss.str());
            //std::cout << str_res << str_res.size() << "\n";
            m_curr_data_len = (max_len > str_res.size())
                              ? str_res.size() : max_len;
            for(uint16_t i = 0; i < m_curr_data_len; ++i) {
                m_data[i] = str_res[i];
            }
        }

        void err_resp(const std::string & msg, uint64_t id) {
            try {
                err_type err;
                err.m_id = id;
                err.m_err = msg;
                err.save();

                write_to_buff(err.m_ss);
            } catch(const std::exception & e) {
                stop();
            }
        }

        int handle_req() {
            req_type req;
            std::stringstream ss(std::string(m_data.data()));
            try {
                req.load(ss);
            } catch (const std::exception & e) {
                stop(); // can't parse
                return 1;
            }

            resp_type resp;
            try {
                resp.m_id = req.m_id;
                // todo: check that a method exists
                resp.m_result = m_sd->m_methods[req.m_method](
                        req.m_params[0], req.m_params[1]);
            } catch (const std::exception & e) {
                err_resp("bad invocation, or arithmetic error", req.m_id);
                return 0;
            }

            try {
                resp.save();
                write_to_buff(resp.m_ss);
            } catch (const std::exception & e) {
                err_resp(e.what(), req.m_id);
            }
            return 0;
        }

        enum : uint16_t { max_len = 1024U };
        std::array<char, max_len> m_data;
        uint16_t m_curr_data_len = 0;
        sd_ptr m_sd;
        tcp::socket m_sock;
        bool m_is_running = false;
    };

    template<class RPC>
    class server {
    public:
        using rpc_type = RPC;
        using func_type = typename shared_data<RPC>::func_type;

        explicit server(uint16_t port)
            : m_ep(ip::address_v4::loopback(), port)
            , m_acc(m_ios, m_ep, true)
            , m_sock(m_ios)
            , m_sig(m_ios, SIGINT, SIGTERM) {
            m_sig.async_wait(
                    [this](const error_code & err, int signal) {
                        m_ios.stop(); m_sig.clear();
                    });
        }

        explicit server(const std::string & port)
            : server(boost::lexical_cast<uint16_t>(port)) { }

        void run() {
            accept_loop();
            m_ios.run();
        }

        void run(uint8_t threads_count) {
            for(uint8_t i = 0; i < threads_count; ++i) {
                m_threads.create_thread(
                        boost::bind(&server::run, boost::ref(*this)));
            }
            m_threads.join_all();
        }

        void bind(std::string method, func_type func) {
            m_sd->m_methods.emplace(std::make_pair(
                    std::move(method), std::move(func)));
        }

    private:
        void accept_loop() {
            m_acc.async_accept(m_sock,
                [this](error_code ec) {
                    if (! ec) {
                        boost::make_shared<peer<RPC>>(
                                std::move(m_sock), m_sd)->start();
                    } else {
                        return;
                    }

                    accept_loop();
                });
        }

        io_service m_ios;
        tcp::endpoint m_ep;
        tcp::acceptor m_acc;
        tcp::socket m_sock;
        signal_set m_sig;

        typename peer<RPC>::sd_ptr m_sd =
                boost::make_shared<shared_data<RPC>>();
        boost::thread_group m_threads;
    };

}