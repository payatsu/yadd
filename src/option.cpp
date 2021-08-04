#include "option.hpp"

#include <regex>
#include <fcntl.h>
#include <getopt.h>
#include "misc.hpp"

#define REGEX_RANGE(capgrp) \
    "(" capgrp "(?:[[:digit:]]+|0x[[:xdigit:]]+)[kmgKMG]?)" "@" \
    "(" capgrp "(?:[[:digit:]]+|0x[[:xdigit:]]+)[kmgKMG]?)"

#define REGEX_TARGET \
    "((?:" REGEX_RANGE("?:") ")|(?:[[:graph:]]+))"

#define REGEX_TRANSFER \
    REGEX_TARGET ":" REGEX_TARGET

option_parser::option_parser(int argc, char* argv[])
: argc_(argc), argv_(argv) {}

param option_parser::parse_cmdopt()const
{
    while(true){
        opterr = 0;
        int option_index = 0;
        option longopts[] = {
            {"help",    no_argument, nullptr, 'h'},
            {"verbose", no_argument, nullptr, 'v'},
            {}
        };

        int c = getopt_long(argc_, argv_, "hv", longopts, &option_index);
        if(c == -1){
            break;
        }

        switch(c){
        case 'h': break;
        case 'v': break;
        case '?':
            errno = EINVAL;
            ERROR_THROW(argv_[0], std::string("unknown option: '")
                    + static_cast<char>(optopt) + '\'');
            break;
        default:
            break;
        }
    }

    param param;
    for(int i = optind; i < argc_; ++i){
        param.transfers.emplace_back(to_transfer(argv_[i]));
    }

    return param;
}

transfer option_parser::to_transfer(const std::string& spec)const
{
    std::string src, dst;
    parse_transfer(spec, src, dst);
    return transfer{to_target(src), to_target(dst)};
}

target option_parser::to_target(const std::string& spec)const
{
    try{
        range r;
        parse_range(spec, r);
        return target("/dev/mem", r.offset, r.length);
    }catch(const std::runtime_error&){
        return target(spec);
    }
}

void option_parser::parse_transfer(const std::string& str, std::string& src, std::string& dst)const
{
    std::smatch m;
    if(!std::regex_match(str, m, std::regex(REGEX_TRANSFER))){
        errno = EINVAL;
        ERROR_THROW(argv_[0], std::string("\"") + str + '"');
    }

    src = m.str(1);
    dst = m.str(2);
}

void option_parser::parse_range(const std::string& str, range& range)const
{
    std::smatch m;
    if(!std::regex_match(str, m, std::regex(REGEX_RANGE("")))){
        errno = EINVAL;
        throw std::runtime_error(std::to_string(-errno));
    }

    std::size_t idx;

    range.length = std::stoul(m.str(1), &idx, 0);
    if(idx < m.str(1).size()){
        range.length *= to_number(m.str(1).at(idx));
    }
    range.offset = std::stoul(m.str(2), &idx, 0);
    if(idx < m.str(2).size()){
        range.offset *= to_number(m.str(2).at(idx));
    }
}

std::size_t option_parser::to_number(char c)
{
    std::size_t n = 1u;
    switch(c){
    case 'k': case 'K': n <<= 10; break;
    case 'm': case 'M': n <<= 20; break;
    case 'g': case 'G': n <<= 30; break;
    default: break;
    }
    return n;
}

// vim: expandtab shiftwidth=0 tabstop=4 :
