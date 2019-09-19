#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace pt = boost::property_tree;

void read_and_write(const std::string & file_name)
{
    pt::ptree   tree;
    std::size_t found         = file_name.find_last_of(".");
    std::string new_file_name = file_name.substr(0, found);
    new_file_name += ".json";

    // https://stackoverflow.com/questions/6572550/boostproperty-tree-xml-pretty-printing
    pt::read_xml(file_name, tree, pt::xml_parser::trim_whitespace);
    pt::write_json(new_file_name, tree);
}

int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        std::cout << "No files for conversion" << std::endl;
        return 0;
    }

    std::vector<std::string> files;
    for(int i = 1; i < argc; i++)
    {
        files.push_back(argv[i]);
    }

    for(const auto & file: files)
    {
        try
        {
            read_and_write(file);
        }
        catch(std::exception & e)
        {
            std::cout << "Error while parsing file: " << file
                      << ". Msg: " << e.what() << "\n";
        }
    }

    return 0;
}
