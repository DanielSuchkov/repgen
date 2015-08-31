#ifndef GENERATOR_H
#define GENERATOR_H
#include <fstream>
#include <string>
#include <exception>
#include <memory>
#include <unordered_map>
#include <functional>
#include <tuple>

class WrongFormat : public std::exception {
public:
    WrongFormat(const std::string &msg)
        : msg("wrong format" + std::string(msg.empty() ? "" : ": ") + msg) {}

    const char *what() const noexcept /*gcc need this specifier*/ {
        return msg.c_str();
    }
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

        access_field = { /// \todo maybe move it to ValSource to get more flexibility?
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
        std::string pattern_buf; // same purpose

        std::getline(templ_file, line_buf);
        while (templ_file.good()) {
            size_t search_pos = 0;
            while ((search_pos = find_pattern(pattern_buf, line_buf, search_pos)) != std::string::npos) {
                try { // replace pattern with value
                    auto val = get_field_value(pattern_buf);
                    auto placeholder_len = open_delim.size() + pattern_buf.size() + close_delim.size();
                    line_buf.replace(search_pos, placeholder_len, val);
                    search_pos += val.size();
                }
                catch (const std::out_of_range &err) {
                    std::cout << err.what() << "; cannot get value: [" << pattern_buf << "]\n";
                    search_pos++;
                }
                catch (const WrongFormat &err) {
                    std::cout << err.what() << "; arg: [" << pattern_buf << "]\n";
                    search_pos++;
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
    const std::string open_delim;
    const std::string close_delim;
    std::unordered_map<
        std::string, std::function<std::string (const ValSource *, const std::string &)>
    > access_field;

private:
    /// \param record_field must be like this: FieldName(RecordName)
    std::string get_field_value(const std::string &pattern) const {
        std::string record, field;
        std::tie(record, field) = parse_pattern(pattern);
        auto it = access_field.find(field);
        if (it == access_field.end()) {
            throw std::out_of_range("no such field: [" + field + "]");
        }
        return it->second(&src, record);
    }

    static auto parse_pattern(const std::string &pattern)
        -> std::tuple<std::string, std::string> {
        auto rs = pattern.find('(');
        if (rs == std::string::npos) {
            throw WrongFormat("cannot find opening '('");
        }

        auto re = pattern.find(')', rs);
        if (re == std::string::npos) {
            throw WrongFormat("cannot find closing ')'");
        }

        return std::make_tuple(pattern.substr(rs + 1, re - rs - 1), pattern.substr(0, rs));
    }

    size_t find_pattern(std::string &pattern_buf, const std::string &line_buf, size_t s_pos) const {
        s_pos = line_buf.find(open_delim, s_pos);
        if (s_pos == std::string::npos) { return std::string::npos; }
        auto e_pos = line_buf.find(close_delim, s_pos);
        if (e_pos == std::string::npos) { return std::string::npos; }
        pattern_buf.assign(
                    line_buf.substr(s_pos + open_delim.size(),
                                    e_pos - s_pos - open_delim.size())
                    ); // get pattern name
        return s_pos;
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
