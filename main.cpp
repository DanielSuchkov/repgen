#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <cstring>
#include <memory>


#include <unordered_map>

/*!
 * \brief The ValStorage class is example of ValSource
 */
class ValStorage {
public:
    ValStorage() {
        for (char i = 'a'; i <= 'z'; ++i) {
            vals[std::string{i}] = std::string{i - 'a' + '0'};
        }
    }

    /// Have to return string representation of value. If no such value – throw std::out_of_range.
    std::string get_val(const std::string &name) const {
        return vals.at(name);
    }

private:
    std::unordered_map<std::string, std::string> vals;
};

template< class ValSource>
class Generator {
public:
    Generator(
            const std::string &template_path,
            const ValSource &src,
            const char *open_delim,
            const char *close_delim
    )
        : templ_file(template_path)
        , src(src)
        , open_delim(open_delim)
        , close_delim(close_delim) {
        if (!templ_file) {
            throw std::ios_base::failure("cannot open file [" + template_path + "]");
        }
    }

    void generate_file(const std::string &output_path) {
        std::ofstream of(output_path);
        if (!of) {
            throw std::ios_base::failure("cannot create file [" + output_path + "]");
        }
        std::string line_buf; // to avoid reallocations
        std::string name_buf; // same purpose

        std::getline(templ_file, line_buf);
        while (templ_file.good()) {
            size_t s_pos = 0;
            while ((s_pos = line_buf.find(open_delim, s_pos)) != std::string::npos) {
                auto e_pos = line_buf.find(close_delim, s_pos);
                if (e_pos == std::string::npos) {
                    break;
                }

                name_buf.assign(line_buf.begin() + s_pos + strlen(open_delim), line_buf.begin() + e_pos);
                auto pattern_length = name_buf.size() + strlen(open_delim) + strlen(close_delim);
                try {
                    std::string val = src.get_val(name_buf);
                    line_buf.replace(s_pos, pattern_length, val);
                    s_pos += val.length();
                }
                catch (const std::out_of_range &) {
                    std::cout << "no such value: " << name_buf << std::endl;
                    s_pos += strlen(open_delim);
                }
            }
            of.write(line_buf.c_str(), line_buf.size());
            of.put('\n');
            std::getline(templ_file, line_buf);
        }
        templ_file.seekg(0, templ_file.beg);
    }

private:
    std::ifstream templ_file;
    const ValSource &src;
    const char *open_delim;
    const char *close_delim;
};

/*!
 * To deduce type of ValSource automatically
 */
template< class ValSource >
std::unique_ptr<Generator<ValSource>> make_generator(
        const std::string &template_path,
        const ValSource &src,
        const char *open_delim = "<<<",
        const char *close_delim = ">>>") {
    return std::make_unique<Generator<ValSource>>(template_path, src, open_delim, close_delim);
}

int main() {
    try {
        ValStorage stg;
        auto gnr = make_generator(std::string("./test_template"), stg);
        gnr->generate_file("./test_report");
    }
    catch (const std::exception &err) {
        std::cout << err.what() << std::endl;
    }
}
