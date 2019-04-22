#include <iostream>
#include "json_rpc.hpp"
#include "server.hpp"

using namespace rpc;

double add(double a, double b) { return a + b; }

struct div {
    double operator()(double a, double b) {
        return a / b;
    }
} div_ftr;

int main(int argc, char ** argv) {
    if(argc != 2) {
        std::cout << "Usage: server <port>\n";
        return 1;
    }
    
    server<json::json_rpc> server(argv[1]);
    
    server.bind("sub", [](double a, double b) { return a - b; });
    
    std::function<double (double, double)> add_obj = add;
    server.bind("add", add_obj);
    
    server.bind("div", div_ftr);
    
    try {
        server.run(2);
    } catch (const std::exception & e) {
        std::cout << "Server failed: " << e.what() << "\n";
        return 1;
    }
    
    std::cout << "Finished\n";
    return 0;
}