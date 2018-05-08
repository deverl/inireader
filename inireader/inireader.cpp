// This command line program just reads a named value from a section in an ini
// file.
//
// usage: inireader <path-to-ini-file>  <section-name>  <value-name>
//
// For example, if the ini file was as shown here:
//
//     --------------------------------------------------------------------
//     ; File: sample.ini
// 
//     [USER]
//     email = "somebody@domain.com"
//     [CLIENT]
//     phone = "555-555-1212"
// 
//     --------------------------------------------------------------------
//
// You could read the client's phone number using this command:
//
// $ inireader sample.ini  CLIENT  phone
//
// The phone number would be printed on the terminal


#include <iostream>
#include <fstream>
#include <string>


// The result returned from parse_section_entry()
class Entry
{
public:
    Entry()
    {
    }

    Entry(const std::string& name, const std::string& value)
    {
        this->n = name;
        this->v = value;
    }

public:
    bool valid()
    {
        return !n.empty() && !v.empty();
    }

    std::string name()
    {
        return this->n;
    }

    std::string value()
    {
        return this->v;
    }

protected:
    std::string n;
    std::string v;
};


// Remove leading and trailing whitespace from a string.

std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t");
    if (std::string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}






// Removes leading and trailing quotes from the string (if both are present)
std::string unquote(const std::string& str) {
    if (!str.empty()) {
        const char quote('"');
        if (str[0] == quote && str[str.length() - 1] == quote)
        {
            return str.substr(1, str.length() - 2);
        }
    }
    return str;
}



// Returns true if the given str is a section entry from an ini file (begins
// with '[' and ends with ']') AND if the string between the brackets matches
// the specified section name.
bool is_section(const std::string& str, const std::string& section_name)
{
    bool ret(false);

    if (!str.empty() && !section_name.empty()) 
    {
        std::string::size_type section_name_length = section_name.length();
        if (str.length() >= section_name_length + 2) {
            if(str[0] == '[' && str[section_name_length + 1] == ']')
            {
                std::string sub(str.substr(1, section_name_length));
                if(sub == section_name)
                {
                    ret = true;
                }
            }
        }
    }

    return ret;
}




// Parses a section entry (if it is an entry) and sets the values in an instance of Entry which becomes the return value.
// If the section is not an entry, nullptr is returned.
Entry * parse_section_entry(const std::string& line)
{
    std::string s(trim(line));

    if(!s.empty())
    {
        // Read the first token. This is up to the first '=' character, and after stripping whitespace.

        std::string name;

        int i(0);

        while(s[i] != '=') {
            name += s[i];
            ++i;
        }

        name = trim(name);

        s = s.substr(i + 1);

        s = unquote(trim(s));

        Entry *e = new Entry(name, s);

        return e;
    }

    return nullptr;
}






int main(int argc, char *argv[])
{
    int ret(0);
    
    if(argc == 4)
    {
        std::string path(argv[1]);
        std::string section(argv[2]);
        std::string name(argv[3]);

        std::ifstream f(path);

        if(f.good())
        {
            std::string line;
            bool done(false);
            bool in_section(false);

            while(!done && f.good() && !f.eof())
            {
                std::getline(f, line);
                if (line.empty()) {
                    // The line was completely empty.
                    continue;
                }
                line = trim(line);
                if (line.empty()) {
                    // The line consisted only of whitespace.
                    continue;
                }
                if(line[0] == ';') {
                    // This is a comment line.
                    continue;
                }
                if(line[0] == '[' && in_section) {
                    // This is the beginning of a section, and we were already processing our target section.
                    in_section = false;
                    // We've read the section and not found the result.
                    done = true;
                    continue;
                }
                else if (in_section) {
                    Entry *e(parse_section_entry(line));

                    if(e)
                    {
                        if (e->valid() && e->name() == name) {
                            std::cout << e->value() << std::endl;
                        }

                        delete e;
                    }
                }
                else if(is_section(line, section)) {
                    // This is the beginning of our target section.
                    in_section = true;
                    continue;
                }
            }
        }
        else
        {
            std::cerr << "Couldn't open file " << path << " for reading" << std::endl;
            ret = 3;
        }
    }
    else {
        std::cerr << "Invalid usage. You must supply three parameters: <path> <section> <name>." << std::endl;
        ret = 1;
    }

    return ret;
}
