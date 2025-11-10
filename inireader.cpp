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

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

using namespace std;

// Represents a name-value pair parsed from an INI file line.
class Entry {
public:
    Entry() = default;
    Entry(string name, string value)
        : n(std::move(name))
        , v(std::move(value)) { }

    [[nodiscard]] bool valid() const noexcept {
        return !n.empty() && !v.empty();
    }

    void clear() noexcept {
        n.clear();
        v.clear();
    }

    [[nodiscard]] const string& name() const noexcept { return n; }
    [[nodiscard]] const string& value() const noexcept { return v; }

    friend bool                 parse_section_entry(const string& line, Entry& e);

private:
    string n;
    string v;
};

// Case-insensitive string comparison
[[nodiscard]] bool iequals(string_view a, string_view b) noexcept {
    return a.size() == b.size() && equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
               return tolower(x) == tolower(y);
           });
}

// Trim leading/trailing whitespace
[[nodiscard]] string trim(string_view sv) {
    auto is_not_space = [](unsigned char c) { return !isspace(c); };
    auto start        = find_if(sv.begin(), sv.end(), is_not_space);
    auto end          = find_if(sv.rbegin(), sv.rend(), is_not_space).base();
    return (start < end) ? string(start, end) : string {};
}

// Remove surrounding quotes if present
[[nodiscard]] string unquote(string_view sv) {
    if (sv.size() >= 2 && sv.front() == '"' && sv.back() == '"') {
        sv.remove_prefix(1);
        sv.remove_suffix(1);
    }
    return string(sv);
}

// Parse a line as a key=value entry; returns true if successful
bool parse_section_entry(const string& line, Entry& e) {
    e.clear();
    string trimmed = trim(line);
    if (trimmed.empty()) {
        return false;
    }

    if (auto pos = trimmed.find('='); pos != string::npos) {
        string name  = trim(trimmed.substr(0, pos));
        string value = unquote(trim(trimmed.substr(pos + 1)));

        if (!name.empty()) {
            e = Entry { std::move(name), std::move(value) };
            return true;
        }
    }
    return false;
}

// Check if a line represents the desired section header [Section]
[[nodiscard]] bool is_section(const string& line, string_view section_name) {
    if (line.size() < 3 || line.front() != '[' || line.back() != ']') {
        return false;
    }

    string inner = trim(line.substr(1, line.size() - 2));
    return iequals(inner, section_name);
}

// Main program
int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <path> <section> <name>\n";
        return 1;
    }

    const filesystem::path path(argv[1]);
    const string           section(argv[2]);
    const string           name(argv[3]);

    ifstream               file(path);
    if (!file) {
        cerr << "Error: could not open file \"" << path.string() << "\"\n";
        return 3;
    }

    string line;
    bool   in_section = false;
    Entry  entry;

    while (getline(file, line)) {
        string trimmed = trim(line);
        if (trimmed.empty() || trimmed.starts_with(';') || trimmed.starts_with('#')) {
            continue;
        }

        if (trimmed.starts_with('[') && trimmed.ends_with(']')) {
            if (in_section) {
                break; // leaving target section
            }
            in_section = is_section(trimmed, section);
            continue;
        }

        if (in_section && parse_section_entry(trimmed, entry)) {
            if (entry.valid() && iequals(entry.name(), name)) {
                cout << entry.value() << '\n';
                return 0;
            }
        }
    }

    cerr << "Entry \"" << name << "\" not found in section [" << section << "]\n";
    return 2;
}
