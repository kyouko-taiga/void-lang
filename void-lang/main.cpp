#include <iostream>
#include <string>

#include "parser.hh"

int main(int argc, const char* argv[]) {
    std::string source = "void ** * a;";
    voidlang::parse(source.begin(), source.end());

    return 0;
}
