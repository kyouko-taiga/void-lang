#pragma once

#include <ostream>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_wrapper.hpp>

namespace voidlang {

    struct PrintVisitor: boost::static_visitor<> {
        PrintVisitor(std::ostream& o): o(o) {}

        template<typename T>
        void operator()(const T& e) const {
            o << e;
        }

        std::ostream& o;
    };

    // -----------------------------------------------------------------------

    struct Literal {
        int value;
    };

    std::ostream& operator<<(std::ostream& o, const Literal& lit) {
        return o << lit.value;
    }

    // -----------------------------------------------------------------------

    struct Identifier {
        std::string name;
    };

    std::ostream& operator<<(std::ostream& o, const Identifier& ident) {
        return o << ident.name;
    }

    // -----------------------------------------------------------------------

    enum class Operator {
        not_,
        dref,
        ref ,
        mul , div , mod ,
        add , sub ,
        shfl, shfr,
        lt  , le  , gt  , ge  ,
        eq  , ne  ,
        land,
        lor ,
    };

    std::ostream& operator<<(std::ostream& o, const Operator& op) {
        switch (op) {
            case Operator::not_: return o << "!" ;
            case Operator::dref: return o << "&" ;
            case Operator::ref : return o << "*" ;
            case Operator::mul : return o << "*" ;
            case Operator::div : return o << "/" ;
            case Operator::mod : return o << "%" ;
            case Operator::add : return o << "+" ;
            case Operator::sub : return o << "-" ;
            case Operator::shfl: return o << "<<";
            case Operator::shfr: return o << ">>";
            case Operator::lt  : return o << "<" ;
            case Operator::le  : return o << "<=";
            case Operator::gt  : return o << ">" ;
            case Operator::ge  : return o << ">=";
            case Operator::eq  : return o << "==";
            case Operator::ne  : return o << "!=";
            case Operator::land: return o << "&&";
            case Operator::lor : return o << "||";
        }

        assert(false && "unexpected operator");
    }

    // -----------------------------------------------------------------------

    struct UnaryExpression;
    struct BinaryExpression;

    typedef boost::variant<
    Literal,
    Identifier,
    boost::recursive_wrapper<UnaryExpression>,
    boost::recursive_wrapper<BinaryExpression>
    > Expression;

    std::ostream& operator<<(std::ostream& o, const Expression& expr) {
        boost::apply_visitor(PrintVisitor(o), expr);
        return o;
    }

    // -----------------------------------------------------------------------

    struct UnaryExpression {
        UnaryExpression(
                        const Operator&   operator_,
                        const Expression& operand)
        : operator_(operator_), operand(operand) {}

        Operator   operator_;
        Expression operand;
    };

    std::ostream& operator<<(std::ostream& o, const UnaryExpression& expr) {
        return o << expr.operator_ << expr.operand;
    }

    // -----------------------------------------------------------------------

    struct BinaryExpression {
        BinaryExpression(
                         const Expression& left,
                         const Operator&   operator_,
                         const Expression& right)
        : left(left), operator_(operator_), right(right) {}

        Expression left;
        Operator   operator_;
        Expression right;
    };

    std::ostream& operator<<(std::ostream& o, const BinaryExpression& expr) {
        return o << "(" << expr.left
        << " " << expr.operator_
        << " " << expr.right
        << ")";
    }

    // -----------------------------------------------------------------------

    struct VoidType;
    struct PointerType;

    typedef boost::variant<
    VoidType,
    boost::recursive_wrapper<PointerType>
    > Type;

    std::ostream& operator<<(std::ostream& o, const Type& type) {
        boost::apply_visitor(PrintVisitor(o), type);
        return o;
    }

    // -----------------------------------------------------------------------

    struct VoidType {};

    std::ostream& operator<<(std::ostream& o, const VoidType& type) {
        return o << "void";
    }

    // -----------------------------------------------------------------------

    struct PointerType {
        PointerType(Type pointee_type)
        : pointee_type(pointee_type) {}

        Type pointee_type;
    };

    std::ostream& operator<<(std::ostream& o, const PointerType& type) {
        return o << type.pointee_type << "*";
    }

    // -----------------------------------------------------------------------

    struct VariableDeclaration {
        Type        type;
        std::string name;
    };

    std::ostream& operator<<(std::ostream& o, const VariableDeclaration& decl) {
        return o << decl.type << " " << decl.name;
    }

}

BOOST_FUSION_ADAPT_STRUCT(voidlang::Literal, (int, value))
BOOST_FUSION_ADAPT_STRUCT(voidlang::Identifier, (std::string, name))
BOOST_FUSION_ADAPT_STRUCT(voidlang::VariableDeclaration,
                          (voidlang::Type, type)
                          (std::string   , name))
