#include <iostream>
#include <unordered_map>

#include "Generator.h"

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
