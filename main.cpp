#include <iostream>
#include <unordered_map>
#include <functional>

#include "Generator.hpp"

struct Record
{
    std::string name;
    int value;
    std::string description;
    unsigned unit;
};

/*!
 * \brief The ValStorage class is example of ValSource
 */
class ValStorage
{
public:
    ValStorage()
    {
        vals = {
            { "The sandwich-maker", Record { "Arthur Dent", 29, "Human, Male", 42 } },
            { "Ix", Record { "Ford Prefect", 36, "Betelgeusian, Male", 13 } },
            { "Trillian Astra", Record { "Tricia Marie McMillan", 32, "Human, Female", 57 } },
            { "Slartibartfast", Record { "Slartibartfast", 52, "Magrathean, Male", 61 } },
            { "Marvin", Record { "the Paranoid Android", 60, "GPP prototype android, Male design", 18 } },
            { "Zaphod Beeblebrox", Record { "President Zaphod Beeblebrox", 31, "Betelgeusian, Male", 27 } }
        };
    }

    /// Have to return string representation of value. If no such value – throw std::out_of_range.
//    std::string get_val(const std::string &name) const
//    {
//        return vals.at(name);
//    }

    std::string get_name(const std::string &name) const
    {
        return get_record(name).name;
    }

    std::string get_value(const std::string &name) const
    {
        return std::to_string(get_record(name).value);
    }

    std::string get_description(const std::string &name) const
    {
        return get_record(name).description;
    }

    std::string get_unit(const std::string &name) const
    {
        return std::to_string(get_record(name).unit);
    }

    Generator::field_accessors_t get_fields_accessors() const
    {
        auto bindThis = [this] (auto fn_ptr) {
            return std::bind(fn_ptr, this, std::placeholders::_1);
        };

        return {
            { "Name", bindThis(&ValStorage::get_name) },
            { "Value", bindThis(&ValStorage::get_value) },
            { "Description", bindThis(&ValStorage::get_description) },
            { "Unit", bindThis(&ValStorage::get_unit) }
        };
    }

private:
    std::unordered_map<std::string, Record> vals;

private:
    const Record &get_record(const std::string &name) const
    {
        return vals.at(name);
    }
};

int main() {
    try
    {
        ValStorage stg;
        Generator gnr("./test_template", stg.get_fields_accessors());
        gnr.generate_file("./test_report");
    }
    catch (const std::exception &err)
    {
        std::cout << err.what() << std::endl;
    }
}
