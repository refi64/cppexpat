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
#include <exception>
#include <istream>
#include <expat.h>
#include <utility>
#include <cstring>
#include <sstream>
#include <string>
#include <memory>
#include <map>

#define TO_PBASE static_cast<ParserBase*>(userdata)

namespace CppExpat
{
    class XMLError: public std::exception
    {
    public:
        XMLError(XML_Parser p)
        {
            XML_Error err = XML_GetErrorCode(p);
            this->msg = XML_ErrorString(err);
            this->lineno = XML_GetCurrentLineNumber(p);
            this->colno = XML_GetCurrentColumnNumber(p);
        }
        virtual const char* what() const throw()
        {
            std::stringstream ss;
            ss << this->msg << " at line " << this->lineno << ", column " << this->colno;
            return ss.str().c_str();
        }
    private:
        XML_Size lineno;
        XML_Size colno;
        std::string msg;
    };
    
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
        //! Called for processing instructions.
        virtual void pinstr(std::string target, std::string data) {}
        //! Parse an input stream.
        void parse(std::istream& f, int sz);
        //! Parse a string.
        void parse(std::string s);
    private:
        static void startElement(void* userdata, const char* name, const char** attr);
        static void endElement(void* userdata, const char* name);
        static void cData(void* userdata, const char* data, int len);
        static void processingInstr(void* userdata, const char* target, const char* data);
        static ElementAttr build_attr(const char** attr);
        XML_Parser p;
    };
    
    ParserBase::ParserBase(): p(XML_ParserCreate(NULL))
    {
        XML_SetElementHandler(p, this->startElement, this->endElement);
        XML_SetCharacterDataHandler(p, this->cData);
        XML_SetProcessingInstructionHandler(p, this->processingInstr);
        XML_SetUserData(p, static_cast<void*>(this));
    }
    
    void ParserBase::startElement(void* userdata, const char* name, const char** attr)
    {
        TO_PBASE->start(name, ParserBase::build_attr(attr));
    }
    
    void ParserBase::endElement(void* userdata, const char* name)
    {
        TO_PBASE->end(name);
    }
    
    void ParserBase::cData(void* userdata, const char* data, int len)
    {
        // Add the null terminator
        char res[len+1];
        strncpy(res, data, len);
        res[len] = '\0';
        TO_PBASE->chardata(res);
    }
    
    void ParserBase::processingInstr(void* userdata, const char* target, const char* data)
    {
        TO_PBASE->pinstr(target, data);
    }
    
    void ParserBase::parse(std::istream& in, int sz=bufsize)
    {
        XML_Status s;
        std::unique_ptr<char[]> buf{new char[sz]};
        if (in.eof()) return;
        for (;;)
        {
            in.read(buf.get(), sz);
            s = XML_Parse(this->p, buf.get(), in.gcount(), in.eof());
            if (!s) throw XMLError(this->p);
            if (in.eof()) break;
        }
    }
    
    void ParserBase::parse(std::string s)
    {
        XML_Status st = XML_Parse(this->p, s.c_str(), s.length(), true);
        if (!st) throw XMLError(this->p);
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
    using pinstrCallbackType = std::function<void(std::string,std::string)>;
    using pinstrCallback = pinstrCallbackType;
    
    //! A pre-made XML parser class that takes callback functions.
    class XMLParser : public ParserBase
    {
    public:
        XMLParser() {}
        //! Set the start element handler(should be (void)(std::string, ElementAttr)).
        inline void setStartElementHandler(startCallback);
        //! Set the end element handler(should be (void)(std::string)).
        inline void setEndElementHandler(endCallback);
        //! Set the character data handler(should be (void)(std::string)).
        inline void setCDataHandler(cdataCallback);
        //! Set the processing instruction handler(should be (void)(std::string,std::string))
        inline void setPInstrHandler(pinstrCallback);
        void start(std::string s, ElementAttr attr) { this->startF(s,attr); }
        void end(std::string s) { this->endF(s); }
        void chardata(std::string s) { this->cdataF(s); }
        void pinstr(std::string target, std::string data) { this->pInstrF(target, data); }
    private:
        startCallback startF{[](std::string, ElementAttr){}};
        endCallback endF{[](std::string){}};
        cdataCallback cdataF{[](std::string){}};
        pinstrCallback pInstrF{[](std::string,std::string){}};
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
    inline void XMLParser::setPInstrHandler(pinstrCallback pinstr)
    {
        this->pInstrF = pinstr;
    }
}

#undef TO_PBASE

