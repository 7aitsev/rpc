#include "client.hpp"
#include "json_rpc.hpp"

#include <iostream>

using namespace rpc;

double compute_ap_local(int begin, int end) {
    return ((begin + end) * (end - begin + 1)) / 2.0;
}

double compute_ap_remote(const char * port, int b, int e) {
    client<json::json_rpc> client(port);
    double res = 0;

    try {
        for(int i = b; i <= e; ++i) {
            res = client.call("add", {res, double(i)});
            std::cout << i << ": res = " << res << '\n';
        }
    } catch (const std::exception & e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return res;
}

int main(int argc, char ** argv) {
    if(2 != argc) {
        std::cerr << "Usage: sample_client <port>\n";
        return 1;
    }

    auto task1 = std::async(std::launch::async,
            compute_ap_remote, argv[1], 0, 10);
    auto task2 = std::async(std::launch::async,
            compute_ap_remote, argv[1], 10, 20);

    double res1 = task1.get();
    double res2 = task2.get();

    std::cout
        << "\n===== Summary =====\n"
        << "First:  " << res1 << " == " << compute_ap_local(0,  10) << '\n'
        << "Second: " << res2 << " == " << compute_ap_local(10, 20) << '\n';

    return 0;
}