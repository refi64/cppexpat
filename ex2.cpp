#include <cppexpat.hpp>
#include <iostream>
#include <string>

int main() {
    /* XMLParser lets you simply specify callbacks instead of creating a parser
       class. It's useful if you're only going to parse once or twice. */
    cppexpat::XMLParser p;
    // If this syntax is foreign to you, look up C++11's lambdas
    p.set_start_handler([=](std::string s, cppexpat::ElementAttr attr) {
        std::cout << "start: " << s << std::endl;
    });
    p.set_end_handler([=](std::string s) {
        std::cout << "end: " << s << std::endl;
    });
    p.set_chardata_handler([=](std::string s) {
        std::cout << "character data: " << s << std::endl;
    });
    p.parse(R"(<x><a b="c">abc</a></x>)");
    return 0;
}
