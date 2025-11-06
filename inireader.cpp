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
// Both the section name and the key name comparisons disregard differences in case, so
// all of the following would work also:
// $ inireader sample.ini  client  phone
// $ inireader sample.ini  client  PHONE
// $ inireader sample.ini  CLIENT  PHONE
//
// The phone number would be printed on the terminal


#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>


// Contains a name value pair, as parsed by the parse_section_entry() function.
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

    void clear()
    {
        this->n.clear();
        this->v.clear();
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
    void set_name(const std::string& name)
    {
        this->n = name;
    }

    void set_value(const std::string& value)
    {
        this->v = value;
    }

    friend bool parse_section_entry(const std::string& line, Entry& e);

protected:
    std::string n;
    std::string v;
};



// Performs a case insensitive comparison of two strings.

bool iequals(const std::string& a, const std::string& b) {
    if(a.size() != b.size())
    {
        return false;
    }

    return std::equal(a.begin(), a.end(), b.begin(),
                  [](char a, char b) { return std::tolower(static_cast<unsigned char>(a))
                                           == std::tolower(static_cast<unsigned char>(b)); });
}



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

std::string unquote(std::string_view str) {
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"')
    {
        str.remove_prefix(1);
        str.remove_suffix(1);
    }
    return std::string(str);
}


// Gets the next non-empty, non-comment line from the file.
// Returns the string length of the fetched line.
int get_line(std::ifstream& f, std::string& line)
{
    line.clear();

    while(f.good() && !f.eof())
    {
        std::getline(f, line);
        line = trim(line);
        if (line.empty()) {
            // The line was completely empty, or consisted only of whitespace
            continue;
        }
        if(line.starts_with(';')) {
            // This is a comment line.
            continue;
        }
        return line.length();
    }

    return 0;
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
                ret = iequals(sub, section_name);
            }
        }
    }

    return ret;
}




// Parses a section entry (if it is an entry) and sets the values in an instance of Entry which becomes the return value.
// If the section is not an entry, nullptr is returned.

bool parse_section_entry(const std::string& line, Entry& e)
{
    e.clear();
    std::string trimmed = trim(line);
    if (!trimmed.empty())
    {
        auto pos = trimmed.find('=');
        if (pos != std::string::npos)
        {
            std::string name = trim(trimmed.substr(0, pos));
            std::string value = unquote(trim(trimmed.substr(pos + 1)));
        
            if (!name.empty())
            {
                e.set_name(name);
                e.set_value(value);
                return true;
            }
        }
    }
    return false;
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
            Entry e;

            while(!done && f.good() && !f.eof())
            {
                get_line(f, line);

                if (line.empty()) {
                    // The line was completely empty. If this happens, we should be at the end of the file.
                    // Just continue and f.eof() should terminate the loop on the next iteration.
                    continue;
                }
                if (line.starts_with('[') && in_section) {
                    // This is the beginning of a section, and we were already processing our target section.
                    in_section = false;
                    // We've read the section and not found the result.
                    done = true;
                    continue;
                }
                else if (in_section) {
                    parse_section_entry(line, e);

                    if (e.valid() && iequals(e.name(), name)) {
                        std::cout << e.value() << std::endl;
                        // We've printed out the single value the user was after. We can quit now.
                        done = true;
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
