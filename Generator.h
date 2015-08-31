#ifndef GENERATOR_H
#define GENERATOR_H
#include <fstream>
#include <string>
#include <exception>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <functional>
#include <tuple>

class WrongFormat : public std::exception {
public:
    WrongFormat(const std::string &msg)
        : msg("Wrong format" + std::string(msg.empty() ? "" : ": ") + msg) {}

private:
    std::string msg;
};

template<class ValSource>
class Generator {
public:
    Generator(
            const std::string &template_path,
            const ValSource &src,
            const char *open_delim,
            const char *close_delim)
        : templ_file(template_path)
        , src(src)
        , open_delim(open_delim)
        , close_delim(close_delim) {
        if (!templ_file) {
            throw std::ios_base::failure("cannot open file [" + template_path + "]");
        }

        access_field = {
            { "Name", &ValSource::get_name },
            { "Value", &ValSource::get_value },
            { "Description", &ValSource::get_description },
            { "Unit", &ValSource::get_unit }
        };
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
            while ((s_pos = line_buf.find(open_delim, s_pos)) != std::string::npos) { // find start
                auto e_pos = line_buf.find(close_delim, s_pos); // find end
                if (e_pos == std::string::npos) {
                    break;
                }

                /// \todo Rewrite it using std::string::substr
                name_buf.assign(line_buf.begin() + s_pos + strlen(open_delim), line_buf.begin() + e_pos); // get pattern name
                auto pattern_length = name_buf.size() + strlen(open_delim) + strlen(close_delim);
                try { // replace pattern with value
                    auto val = get_field_value(name_buf);
                    line_buf.replace(s_pos, pattern_length, val);
                    s_pos += val.length();
                }
                catch (const std::out_of_range &err) {
                    std::cout << err.what() << "; cannot get value: " << name_buf << std::endl;
                    s_pos += strlen(open_delim);
                }
                catch (const WrongFormat &err) {
                    std::cout << err.what() << " [" << name_buf << "]" << std::endl;
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
    std::unordered_map<
        std::string, std::function<std::string (const ValSource *, const std::string &)>
    > access_field;

private:
    /// \param record_field must be like this: FieldName(RecordName)
    std::string get_field_value(const std::string &placeholder) const {
        std::string record, field;
        std::tie(record, field) = parse_placeholder(placeholder);
        auto it = access_field.find(field);
        if (it == access_field.end()) {
            throw std::out_of_range("no such field: " + field);
        }
        std::cout << record << ": " << field << std::endl;
        return it->second(&src, record);
    }

    static auto parse_placeholder(const std::string &placeholder)
        -> std::tuple<std::string, std::string> {
        auto rs = placeholder.find('(');
        if (rs == std::string::npos) { throw WrongFormat("cannot find opening '('"); }
        auto re = placeholder.find(')', rs);
        if (re == std::string::npos) { throw WrongFormat("cannot find closing ')'"); }
        return std::make_tuple(placeholder.substr(rs + 1, re - rs - 1), placeholder.substr(0, rs));
    }
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

#endif // GENERATOR_H
