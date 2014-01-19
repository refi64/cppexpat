/*
The MIT License (MIT)

Copyright (c) 2014 Ryan Gonzalez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <functional>
#include <fstream>
#include <expat.h>
#include <utility>
#include <cstring>
#include <string>
#include <map>

namespace CppExpat
{
    using ElementAttr = std::map<std::string, std::string>;
    constexpr int bufsize = 10240;
    
    //! The parser base class.
    /*! Create a derived version of this class to implement custom callbacks. */
    class ParserBase
    {
    public:
        ParserBase();
        ~ParserBase() { XML_ParserFree(p); }
        //! Called when an element starts.
        virtual void start(std::string name, ElementAttr attr) {}
        //! Called when an element ends.
        virtual void end(std::string name) {}
        //! Called when character data is encountered.
        virtual void chardata(std::string data) {}
        //! Parse a file stream.
        void parse(std::ifstream f, int sz);
        //! Parse a string.
        void parse(std::string s);
    private:
        static void startElement(void* userdata, const char* name, const char** attr);
        static void endElement(void* userdata, const char* name);
        static void cData(void* userdata, const char* data, int len);
        static ElementAttr build_attr(const char** attr);
        XML_Parser p;
    };
    
    ParserBase::ParserBase(): p(XML_ParserCreate(NULL))
    {
        XML_SetElementHandler(p, this->startElement, this->endElement);
        XML_SetCharacterDataHandler(p, this->cData);
        XML_SetUserData(p, static_cast<void*>(this));
    }
    
    void ParserBase::startElement(void* userdata, const char* name, const char** attr)
    {
        (static_cast<ParserBase*>(userdata))->start(name, ParserBase::build_attr(attr));
    }
    
    void ParserBase::endElement(void* userdata, const char* name)
    {
        (static_cast<ParserBase*>(userdata))->end(name);
    }
    
    void ParserBase::cData(void* userdata, const char* data, int len)
    {
        // Add the null terminator
        char res[len+1];
        strncpy(res, data, len);
        res[len] = '\0';
        (static_cast<ParserBase*>(userdata))->chardata(res);
    }
    
    void ParserBase::parse(std::ifstream f, int sz=bufsize)
    {
        char* buf;
        buf = static_cast<char*>(XML_GetBuffer(p, sz));
        f.read(buf, sz);
        for (;;)
        {
            XML_ParseBuffer(p, f.gcount(), f.gcount() == 0);
            buf = static_cast<char*>(XML_GetBuffer(p, sz));
            f.read(buf, sz);
            if (f.eof())
            {
                delete buf;
                break;
            }
        }
    }
    
    void ParserBase::parse(std::string s)
    {
        XML_Parse(p, s.c_str(), s.length(), true);
    }
    
    ElementAttr ParserBase::build_attr(const char** attr)
    {
        ElementAttr res;
        for (int i=0; attr[i]; i+=2)
        {
            res[attr[i]] = attr[i+1];
        }
        return res;
    }
    
    using startCallbackType = std::function<void(std::string,ElementAttr)>;
    using startCallback = startCallbackType;
    using endCallbackType = std::function<void(std::string)>;
    using endCallback = endCallbackType;
    using cdataCallbackType = std::function<void(std::string)>;
    using cdataCallback = cdataCallbackType;
    
    //! A pre-made XML parser class that takes callback functions.
    class XMLParser : public ParserBase
    {
    public:
        XMLParser() {}
        //! Set the start element handler(should be (void)(std::string, ElementAttr).
        inline void setStartElementHandler(startCallback);
        //! Set the end element handler(should be (void)(std::string).
        inline void setEndElementHandler(endCallback);
        //! Set the character data handler(should be (void)(std::string).
        inline void setCDataHandler(cdataCallback);
        void start(std::string s, ElementAttr attr) { this->startF(s,attr); }
        void end(std::string s) { this->endF(s); }
        void chardata(std::string s) { this->cdataF(s); }
    private:
        startCallback startF;
        endCallback endF;
        cdataCallback cdataF;
    };
    
    inline void XMLParser::setStartElementHandler(startCallback start)
    {
        this->startF = start;
    }
    
    inline void XMLParser::setEndElementHandler(endCallback end)
    {
        this->endF = end;
    }
    inline void XMLParser::setCDataHandler(cdataCallback cdata)
    {
        this->cdataF = cdata;
    }
}

