#include <cppexpat.hpp>
#include <iostream>
#include <string>

int main()
{
    /* XMLParser lets you simply specify callbacks instead of creating a parser
       class. It's useful if you're only going to parse once or twice. */
    CppExpat::XMLParser p;
    // If this syntax is foreign to you, look up C++11's lambdas
    p.setStartElementHandler([=](std::string s, CppExpat::ElementAttr attr)
                             { std::cout << "start: " << s << std::endl; });
    p.setEndElementHandler([=](std::string s)
                           { std::cout << "end: " << s << std::endl; });
    p.setCharDataHandler([=](std::string s)
                      { std::cout << "character data: " << s << std::endl; });
    p.parse("<x><a b=\"c\">abc</a></x>");
    return 0;
}
