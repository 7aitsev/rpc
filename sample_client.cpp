#include "client.hpp"
#include "json_rpc.hpp"

using namespace rpc;

double compute_ap(const char * port, int b, int e) {
    client<json::json_rpc> client(port);
    double res = 0;
    
    try {
        for(int i = b; i <= e; ++i) {
            res = client.call("add", {res, double(i)});
            std::cout << i << ": res = " << res << '\n';
        }
    } catch (const std::exception & e) {
        std::cout << e.what() << '\n';
        return 1;
    }
    
    return res;
}

int main(int argc, char ** argv) {
    if(argc != 2) {
        std::cout << "Usage: sample_client <port>\n";
        return 1;
    }
    
    auto task1 = std::async(std::launch::async,
            compute_ap, argv[1], 0, 10);
    auto task2 = std::async(std::launch::async,
            compute_ap, argv[1], 10, 20);
    
    double res1 = task1.get();
    double res2 = task2.get();
    
    std::cout << "\n===== Summary =====\n"
              << "First:  " << res1 << " == " << 10.0*11/2 << '\n'
              << "Second: " << res2 << " == " << (10.0+20)*11/2 << '\n';
    
    client<json::json_rpc> client(argv[1]);
    try {
        client.call("div", {10.0, 0});
    } catch(const std::exception & e) {
        std::cout << e.what() << '\n';
    }
    
    return 0;
}