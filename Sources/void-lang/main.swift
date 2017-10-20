import Parsey

extension Lexer {
    static func string(_ string: String) -> Parser<String> {
        precondition(string.count > 0)
        var result = Lexer.character(string.characters[string.startIndex])
        for c in string.dropFirst() {
            result = result + Lexer.character(c)
        }
        return result
    }
}

indirect enum Type_: CustomStringConvertible {
    case void
    case pointer(Type_)

    var description: String {
        switch self {
        case .void:
            return "void"
        case let .pointer(pointeeType):
            return "\(pointeeType)*"
        }
    }
}

indirect enum Expression: CustomStringConvertible {
    case intLiteral(Int, SourceRange)
    case identifier(String, SourceRange)
    case binExpr   (Expression, String, Expression, SourceRange)

    var description: String {
        switch self {
        case let .intLiteral(value, _):
            return String(describing: value)
        case let .identifier(name, _):
            return name
        case let .binExpr(lvalue, op, rvalue, _):
            return "(\(lvalue) \(op) \(rvalue))"
        }
    }
}

indirect enum Statement: CustomStringConvertible {
    case varDecl(Type_, String, Expression?, SourceRange)

    var description: String {
        switch self {
        case let .varDecl(type_, name, init_, _):
            if let e = init_ {
                return "\(type_) \(name) = \(e);"
            } else {
                return "\(type_) \(name);"
            }
        }
    }
}

struct Grammar {

    static let whitespace = Lexer.whitespace
    static let name       = Lexer.regex("[a-zA-Z_][0-9a-zA-Z_]*")

    static let intLiteral = Lexer.signedInteger
        ^^^ { (value, loc) in Expression.intLiteral(Int(value)!, loc) }

    static let identifier = name
        ^^^ { (name, loc) in Expression.identifier(name, loc) }

    static let addExpr: Parser<Expression> =
        mulExpr ~~
        ((Lexer.anyCharacter(in: "+-").amid(whitespace.?) ~~ mulExpr)).*
        ^^^ { (args, loc) in
            let (lvalue, rvalues) = args
            var result = lvalue
            for (op, rvalue) in rvalues {
                result = .binExpr(result, op, rvalue, loc)
            }
            return result
        }

    static let mulExpr: Parser<Expression> =
        atom ~~
        (Lexer.anyCharacter(in: "*/%").amid(whitespace.?) ~~ atom).*
        ^^^ { (args, loc) in
            let (lvalue, rvalues) = args
            var result = lvalue
            for (op, rvalue) in rvalues {

                 result = .binExpr(result, op, rvalue, loc)
            }
            return result
        }

    static let atom = identifier | intLiteral | "(" ~~> expr <~~ ")"

    static let expr: Parser<Expression> = addExpr <!-- "expression"

    static let type: Parser<Type_> =
        (Lexer.string("void") ^^ { _ in Type_.void }) ~~ Lexer.character("*").amid(whitespace.?).*
        ^^ { type_, stars in
            var result = type_
            for _ in 0 ..< stars.count {
                result = .pointer(result)
            }
            return result
        }

    static let varDecl: Parser<Statement> =
        type ~~ name ~~
        (Lexer.character("=").amid(whitespace.?) ~~> expr).? <~~
        whitespace.? <~~ Lexer.character(";")
        ^^^ { (args, loc) in
            let ((type_, name), init_) = args
            return Statement.varDecl(type_, name, init_, loc)
        }

}

do {
    try print(Grammar.varDecl.parse("void * * x = 0;"))
} catch(let e) {
    print(e)
}
