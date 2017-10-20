#pragma once

#define BOOST_SPIRIT_DEBUG
#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <iostream>
#include <string>
#include <vector>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "ast.hh"

namespace voidlang {

    namespace qi      = boost::spirit::qi;
    namespace ascii   = boost::spirit::ascii;
    namespace phoenix = boost::phoenix;

    template<typename Iterator>
    struct IdentifierParser: qi::grammar<Iterator, Identifier()> {
        IdentifierParser(): IdentifierParser::base_type(start) {
            using qi::alnum;
            using qi::alpha;
            using qi::char_;

            // With `string` as attribute type qi is able to use the attribute
            // as container to store several chars, but not with a struct. The
            // solution is to first parse the string from, and then pass the
            // string to the rule that turns it into an identifier.
            // See https://stackoverflow.com/questions/18166958

            string %= (alpha | char_('_')) >> *(alnum | char_('_'));
            start   = this->string;
        }

        qi::rule<Iterator, Identifier()>  start;
        qi::rule<Iterator, std::string()> string;
    };

    template<typename Iterator, typename Skipper>
    struct ExpressionParser: qi::grammar<Iterator, Skipper, Expression()> {
        ExpressionParser(): ExpressionParser::base_type(start, "start") {
            using qi::_1;
            using qi::_2;
            using qi::_val;
            using phoenix::construct;

            lor_op .add ("||", Operator::lor );
            land_op.add ("&&", Operator::land);
            eq_op  .add ("==", Operator::eq  ) ("!=", Operator::ne);
            cmp_op .add ("<" , Operator::lt  ) ("<=", Operator::le)
                        (">" , Operator::gt  ) (">=", Operator::ge);
            shf_op .add ("<<", Operator::shfl) (">>", Operator::shfr);
            add_op .add ("+",  Operator::add ) ("-", Operator::sub);
            mul_op .add ("*",  Operator::mul ) ("/", Operator::div);
            ref_op .add ("*",  Operator::ref );
            dref_op.add ("&",  Operator::dref);
            not_op .add ("!",  Operator::not_);

            lor_expr = land_expr              [_val = _1]
                >> -(lor_op >> land_expr)     [_val = construct<BinaryExpression>(_val, _1, _2)];
            land_expr = eq_expr               [_val = _1]
                >> -(land_op >> eq_expr)      [_val = construct<BinaryExpression>(_val, _1, _2)];
            eq_expr = cmp_expr                [_val = _1]
                >> -(eq_op >> cmp_expr)       [_val = construct<BinaryExpression>(_val, _1, _2)];
            cmp_expr = shf_expr               [_val = _1]
                >> -(cmp_op >> shf_expr)      [_val = construct<BinaryExpression>(_val, _1, _2)];
            shf_expr = add_expr               [_val = _1]
                >> -(shf_op >> add_expr)      [_val = construct<BinaryExpression>(_val, _1, _2)];
            add_expr = mul_expr               [_val = _1]
                >> -(add_op >> mul_expr)      [_val = construct<BinaryExpression>(_val, _1, _2)];
            mul_expr = ref_expr               [_val = _1]
                >> -(mul_op >> ref_expr)      [_val = construct<BinaryExpression>(_val, _1, _2)];
            ref_expr = (ref_op >> dref_expr)  [_val = construct<UnaryExpression>(_1, _2)]
                | dref_expr                   [_val = _1];
            dref_expr = (dref_op >> not_expr) [_val = construct<UnaryExpression>(_1, _2)]
                | not_expr                    [_val = _1];
            not_expr = (not_op >> atom)       [_val = construct<UnaryExpression>(_1, _2)]
                | atom                        [_val = _1];

            literal = qi::int_;
            atom    = (identifier | literal | '(' >> start >> ')');
            start   = lor_expr | atom;

            qi::on_error<qi::fail>(
               start, std::cout << phoenix::val("Error! Expecting: ")
                                << qi::_4 << phoenix::val(" here: \"")
                                << phoenix::construct<std::string>(qi::_3, qi::_2)
                                << phoenix::val("\"") << std::endl);

            // BOOST_SPIRIT_DEBUG_NODES((start)(add_expr)(ref_expr));
        }

        qi::symbols<char, Operator> lor_op , land_op, eq_op  , cmp_op , shf_op ,
                                    add_op , mul_op , ref_op , dref_op, not_op ;

        IdentifierParser<Iterator>                identifier;
        qi::rule<Iterator, Skipper, Literal()>    literal;
        qi::rule<Iterator, Skipper, Expression()> lor_expr;
        qi::rule<Iterator, Skipper, Expression()> land_expr;
        qi::rule<Iterator, Skipper, Expression()> eq_expr;
        qi::rule<Iterator, Skipper, Expression()> cmp_expr;
        qi::rule<Iterator, Skipper, Expression()> shf_expr;
        qi::rule<Iterator, Skipper, Expression()> add_expr;
        qi::rule<Iterator, Skipper, Expression()> mul_expr;
        qi::rule<Iterator, Skipper, Expression()> ref_expr;
        qi::rule<Iterator, Skipper, Expression()> dref_expr;
        qi::rule<Iterator, Skipper, Expression()> not_expr;
        qi::rule<Iterator, Skipper, Expression()> atom;
        qi::rule<Iterator, Skipper, Expression()> start;
    };

    template<typename Iterator, typename Skipper>
    struct Grammar: qi::grammar<Iterator, Skipper, VariableDeclaration()> {
        Grammar(): Grammar::base_type(start, "start") {
            using qi::_val;
            using qi::alnum;
            using qi::alpha;
            using qi::char_;
            using phoenix::construct;

            type = qi::lit("void") [_val = construct<VoidType>()]
                >> *(qi::lit('*') [_val = construct<PointerType>(_val)]);

            name  %= (alpha | char_('_')) >> *(alnum | char_('_'));
            start %= type >> name >> ';';
        }

        qi::rule<Iterator, std::string()>                  name;
        qi::rule<Iterator, Skipper, Type()>                type;
        qi::rule<Iterator, Skipper, VariableDeclaration()> start;
    };

    template<typename Iterator>
    bool parse(Iterator begin, Iterator end) {
        // Create a parser for Void's grammar.
        // ExpressionParser<Iterator, ascii::space_type> grammar;
        Grammar<Iterator, ascii::space_type> grammar;

        // Parse the program.
        // Expression e;
        VariableDeclaration e;
        auto r = qi::phrase_parse(begin, end, grammar, ascii::space, e);
        if (r && (begin == end)) {
            std::cout << e << std::endl;
        } else {
            std::cout << "parsing failed" << std::endl;
            std::cout << std::string(begin, end) << std::endl;
        }

        // Note that we fail if we didn't get a full match, i.e. begin != end.
        return (begin == end) && r;
    }

}
