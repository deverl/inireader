#include <iostream>
#include <fstream>
// #include <stdio.h>
#include <string>
#include <vector>




std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}



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




std::vector<std::string> parse_section_entry(const std::string& line)
{
    std::vector<std::string> v;
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

        s = trim(s);

        v.push_back(name);
        v.push_back(s);
    }

    return v;
}





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
                    std::vector<std::string> v(parse_section_entry(line));

                    if(v.size() == 2 && v[0] == name)
                    {
                        std::string value = unquote(v[1]);
                        std::cout << value << std::endl;
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