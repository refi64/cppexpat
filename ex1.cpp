// Include some headers
#include <cppexpat.hpp>
#include <iostream>
#include <string>

// CppExpat classes are in the cppexpat namespace
// All parsers should derive from the cppexpat::ParserBase base class
class MyDerivedParser : public cppexpat::ParserBase {
    // ElementAttr is an alias for std::map<std::string, std::string>.
    // It is a map of an element's attributes.
    // This function is called for an element opening.
    void start(std::string name, cppexpat::ElementAttr attr) {
        std::cout << "start: " << name << std::endl;
    }

    // This is called when an element ends.
    void end(std::string name) {
        std::cout << "end: " << name << std::endl;
    }

    // This is called for character data.
    void chardata(std::string data) {
        std::cout << "character data: " << data << std::endl;
    }

    // This is for processing instructions.
    void pinstr(std::string, std::string) {}
};

int main() {
    MyDerivedParser p; // Create an instance of the parser class
    p.parse(R"(<x><a b="c">abc</a></x>)"); // Parse a string
    return 0;
}
