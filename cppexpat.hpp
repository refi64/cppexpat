/*
Copyright (c) 2015 Ryan Gonzalez

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

#ifndef CPPEXPAT_HPP
#define CPPEXPAT_HPP

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

#define CPPEXPAT_TO_PBASE static_cast<ParserBase*>(userdata)

namespace cppexpat {
    using std::string;

    //! An exception thrown when Expat encounters an error.
    class XMLError: public std::exception {
    public:
        inline XMLError(XML_Parser p):
            msg{XML_ErrorString(XML_GetErrorCode(p))},
            lineno{XML_GetCurrentLineNumber(p)},
            colno{XML_GetCurrentColumnNumber(p)} {}

        virtual const char* what() const noexcept {
            std::stringstream ss;
            ss << msg << " at line " << lineno << ", column " << colno;
            return ss.str().c_str();
        }
    private:
        string msg;
        XML_Size lineno, colno;
    };

    //! An alias for an attribute map.
    using ElementAttr = std::map<string, string>;
    constexpr unsigned int bufsize = 10240;

    //! The parser base class.
    /*! Create a derived version of this class to implement custom callbacks. */
    class ParserBase {
    public:
        //\cond HIDDEN
        ParserBase();
        ~ParserBase() { XML_ParserFree(p); }
        //\endcond
        //! Parse an input stream.
        void parse(std::istream& f, int sz);
        //! Parse a string.
        void parse(string s);
    protected:
        //! Called when an element starts.
        virtual void start(string name, ElementAttr attr) {}
        //! Called when an element ends.
        virtual void end(string name) {}
        //! Called when character data is encountered.
        virtual void chardata(string data) {}
        //! Called for processing instructions.
        virtual void pinstr(string target, string data) {}
    private:
        static void start_wrapper(void* userdata, const char* name,
            const char** attr);
        static void end_wrapper(void* userdata, const char* name);
        static void chardata_wrapper(void* userdata, const char* data, int len);
        static void pinstr_wrapper(void* userdata, const char* target,
            const char* data);
        static ElementAttr build_attr(const char** attr);
        XML_Parser p;
    };

    ParserBase::ParserBase(): p{XML_ParserCreate(nullptr)} {
        XML_SetElementHandler(p, start_wrapper, end_wrapper);
        XML_SetCharacterDataHandler(p, chardata_wrapper);
        XML_SetProcessingInstructionHandler(p, pinstr_wrapper);
        XML_SetUserData(p, static_cast<void*>(this));
    }

    void ParserBase::start_wrapper(void* userdata, const char* name,
        const char** attr) {
        CPPEXPAT_TO_PBASE->start(name, ParserBase::build_attr(attr));
    }

    void ParserBase::end_wrapper(void* userdata, const char* name) {
        CPPEXPAT_TO_PBASE->end(name);
    }

    void ParserBase::chardata_wrapper(void* userdata, const char* data, int len) {
        // Add the null terminator
        char res[len+1];
        memcpy(res, data, len);
        res[len] = '\0';
        CPPEXPAT_TO_PBASE->chardata(res);
    }

    void ParserBase::pinstr_wrapper(void* userdata, const char* target,
        const char* data) {
        CPPEXPAT_TO_PBASE->pinstr(target, data);
    }

    void ParserBase::parse(std::istream& in, int sz=bufsize) {
        std::unique_ptr<char[]> buf{new char[sz]};
        if (in.eof()) return;
        for (;;) {
            in.read(buf.get(), sz);
            if (!XML_Parse(p, buf.get(), in.gcount(), in.eof()))
                throw XMLError{p};
            if (in.eof()) break;
        }
    }

    void ParserBase::parse(string s) {
        if (!XML_Parse(p, s.c_str(), s.length(), true)) throw XMLError{p};
    }

    ElementAttr ParserBase::build_attr(const char** attr) {
        ElementAttr res;
        for (int i=0; attr[i]; i+=2) res[attr[i]] = attr[i+1];
        return res;
    }

    using StartCallback = std::function<void(string,ElementAttr)>;
    using EndCallback = std::function<void(string)>;
    using ChardataCallback = std::function<void(string)>;
    using PinstrCallback = std::function<void(string,string)>;

    //! A pre-made XML parser class that takes callback functions.
    class XMLParser : public ParserBase {
    public:
        inline XMLParser() {}
        //! Set the start element handler.
        inline void set_start_handler(StartCallback c) { startf = c; }
        //! Set the end element handler.
        inline void set_end_handler(EndCallback c) { endf = c; }
        //! Set the character data handler.
        inline void set_chardata_handler(ChardataCallback c) { chardataf = c; }
        //! Set the processing instruction handler.
        inline void set_pinstr_handler(PinstrCallback c) { pinstrf = c; }
    protected:
        inline void start(string s, ElementAttr attr) { startf(s, attr); }
        inline void end(string s) { endf(s); }
        inline void chardata(string s) {chardataf(s); }
        inline void pinstr(string target, string data) { pinstrf(target, data); }
    private:
        StartCallback startf{[](string, ElementAttr){}};
        EndCallback endf{[](string){}};
        ChardataCallback chardataf{[](string){}};
        PinstrCallback pinstrf{[](string,string){}};
    };
}

#undef CPPEXPAT_TO_PBASE
#endif // CPPEXPAT_HPP
