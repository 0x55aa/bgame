#include "string_utils.hpp"
#include <sstream>

std::vector<std::string> split ( const std::string str, const char delimiter )
{
     std::vector<std::string> internal;
     std::stringstream ss ( str ); // Turn the string into a stream.
     std::string tok;

     while ( getline ( ss, tok, delimiter ) ) {
          internal.push_back ( tok );
     }

     return internal;
}

std::string to_proper_noun_case(const std::string &original)
{
    std::string result;
    result += std::toupper(original[0]);
    for (std::size_t i = 1; i < original.size(); ++i)
    {
        result += std::tolower(original[i]);
    }
    return result;
}
