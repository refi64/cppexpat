// Include some headers
#include <cppexpat.hpp>
#include <iostream>
#include <string>

// CppExpat classes are in the CppExpat namespace
// All parsers should derive from the CppExpat::ParserBase base class
class MyDerivedParser : public CppExpat::ParserBase
{
    // ElementAttr is an alias for std::map<std::string, std::string>.
    // It is a map of an element's attributes.
    // This function is called for an element opening.
    void start(std::string name, CppExpat::ElementAttr attr)
    {
        std::cout << "start: " << name << std::endl;
    }
    
    // This is called when an element ends.
    void end(std::string name)
    {
        std::cout << "end: " << name << std::endl;
    }
    
    // This is called for character data.
    void cdata(std::string data)
    {
        std::cout << "character data: " << data << std::endl;
    }
    
    // This is for processing instructions.
    void pinstr(std::string, std::string) {}
};

int main()
{
    MyDerivedParser p; // Create an instance of our parser
    p.parse("<x><a b=\"c\">abc</a></x>"); // Parse a string
    return 0;
}

