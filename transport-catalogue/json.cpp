#include "json.h"

#include <string>

using namespace std;

namespace json {

    namespace {

        // ---------------- Loaders --------------------

        Node LoadNode(istream& input);

        using Number = std::variant<int, double>;

        string LoadLiteral(std::istream& input) {
            string res;
            while (isalpha(input.peek())) {
                res.push_back(input.get());
            }
            return res;

        }

        Node LoadBool(std::istream& input) {
            string s = LoadLiteral(input);
            if (s == "true") {
                return Node(true);
            }
            else if (s == "false") {
                return Node(false);
            }
            else {
                throw ParsingError("");
            }
        }

        Node LoadNull(std::istream& input) {
            if (auto literal = LoadLiteral(input); literal == "null"sv) {
                return Node(nullptr);
            }
            else {
                throw ParsingError("");
            }
        }

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
                };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
                };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadArray(istream& input) {
            Array result;
            bool closed = false;
            for (char c; input >> c;) {
                if (c == ']') {
                    closed = true;
                    break;
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!closed) {
                throw ParsingError("");
            }
            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            bool closed = false;
            for (char c; input >> c;) {
                if (c == '}') {
                    closed = true;
                    break;
                }
                if (c == ',') {
                    input >> c;
                }
                string key = LoadString(input);
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            if (!closed) {
                throw ParsingError("");
            }

            return Node(move(result));
        }



        Node LoadNode(istream& input) {
            char c;
            input >> c;

            switch (c) {
            case 'n':
                input.putback(c);
                return Node(LoadNull(input));
            case 't':
                input.putback(c);
                return Node(LoadBool(input));
            case 'f':
                input.putback(c);
                return Node(LoadBool(input));
            case '[':
                return Node(LoadArray(input));
            case '{':
                return Node(LoadDict(input));
            case '\"':
                return Node(LoadString(input));
            default:
                input.putback(c);
                Number n = LoadNumber(input);
                if (std::holds_alternative<int>(n)) {
                    return Node(get<int>(n));
                }
                else if (std::holds_alternative<double>(n)) {
                    return Node(get<double>(n));
                }
                else {
                    throw ParsingError("");
                }
            }

            if (c == 'n') {
                return Node();
            }
            else if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return Node(LoadString(input));
            }
            else {
                input.putback(c);
                Number n = LoadNumber(input);
                if (std::holds_alternative<int>(n)) {
                    return Node(std::get<int>(n));
                }
                else if (std::holds_alternative<double>(n)) {
                    return Node(std::get<double>(n));
                }

            }
        }

    }  // namespace

    // ---------------- Node --------------------


    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return IsInt() || IsPureDouble();
    }
    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);;
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);;
    }
    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);;
    }
    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }
    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);;
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw logic_error("");
        }
        return get<int>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("");
        }
        return get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return get<double>(*this);
        }
        else if (IsInt()) {
            return get<int>(*this);
        }
        throw logic_error("");

    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw logic_error("");
        }
        return get<string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("");
        }
        return get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw logic_error("");
        }
        return get<Dict>(*this);
    }

    const Node::Value& Node::GetValue() const {
        return *this;
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() != rhs.GetValue();
    }

    // ---------------- PrintContext --------------------

    void PrintContext::PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext PrintContext::Indented() const {
        return PrintContext{ out, indent_step, indent_step + indent };
    }

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    template<>
    void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
        ctx.out << '\"';
        for (const char& c : value) {
            if (c == '\n') {
                ctx.out << "\\n"sv;
            }
            else if (c == '\r') {
                ctx.out << "\\r"sv;
            }
            else if (c == '\t') {
                ctx.out << "\\t"sv;
            }

            else if (c == '\"') {
                ctx.out << "\\\""sv;
            }
            else if (c == '\\') {
                ctx.out << "\\\\"sv;
            }

            else {
                ctx.out << c;
            }
        }
        ctx.out << '\"';
    }

    template <>
    void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    template<>
    void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
        ctx.out << (value ? "true"sv : "false"sv);
    }

    template <>
    void PrintValue<Array>(const Array& value, const PrintContext& ctx) {
        const auto& inner_indent = ctx.Indented();
        bool first = true;

        ctx.out << '[' << endl;
        for (const auto& node : value) {
            if (!first) {
                ctx.out << ",\n";
            }
            else {
                first = false;
            }
            inner_indent.PrintIndent();
            PrintNode(node, inner_indent.Indented());
        }
        ctx.out << endl;
        ctx.PrintIndent();
        ctx.out << ']';
    }

    template <>
    void PrintValue<Dict>(const Dict& value, const PrintContext& ctx) {
        const auto& inner_indent = ctx.Indented();
        bool first = true;

        ctx.out << '{' << endl;

        for (const auto& [key, val] : value) {
            if (!first) {
                ctx.out << ",\n";
            }
            else {
                first = false;
            }
            inner_indent.PrintIndent();
            PrintNode(key, inner_indent);
            ctx.out << " : ";
            PrintNode(val, inner_indent);

        }
        ctx.out << endl;
        ctx.PrintIndent();
        ctx.out << '}';
    }


    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) { PrintValue(value, ctx); },
            node.GetValue());
    }

    std::ostream& operator<<(std::ostream& out, const Node& node) {
        if (std::holds_alternative<int>(node.GetValue())) {
            out << std::get<int>(node.GetValue());
        }
        else if (holds_alternative<double>(node.GetValue())) {
            out << std::get<double>(node.GetValue());
        }
        return out;
    }

    // ---------------- Document --------------------

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintContext ctx{ output };
        PrintNode(doc.GetRoot(), ctx);
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs.GetRoot() == rhs.GetRoot());
    }

}  // namespace json