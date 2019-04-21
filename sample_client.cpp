#include "client.hpp"
#include "json_rpc.hpp"

using namespace rpc;

int main(int argc, char ** argv) {
    if(argc != 2) {
        std::cout << "Usage: sample_client <port>\n";
        return 1;
    }
    
    client<json::json_rpc> client(argv[1]);

    try {
        for(int i = 0; i < 10; ++i) {
            double res = client.call("add", {double(i*i), double(-i)});
            std::cout << i << ": res = " << res << '\n';
        }
    } catch (const std::exception & e) {
        std::cout << e.what() << '\n';
        return 1;
    }
    
    client.stop();
    
    try {
        client.call("div", {10.0, 0.0});
    } catch(const std::exception & e) {
        std::cout << e.what() << '\n';
    }
    
    return 0;
}