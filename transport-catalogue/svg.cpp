#include "svg.h"

namespace svg {

    using namespace std::literals;

    void PrintColor(std::ostream& out, [[maybe_unused]] std::monostate obj) {
        out << "none"sv;
    }
    void PrintColor(std::ostream& out, std::string obj) {
        out << obj;
    }
    void PrintColor(std::ostream& out, Rgb obj) {
        out << "rgb("sv << (unsigned int)obj.red << ","sv << (unsigned int)obj.green << ","sv << (unsigned int)obj.blue << ")"sv;
    }
    void PrintColor(std::ostream& out, Rgba obj) {
        out << "rgba("sv << (unsigned int)obj.red << ","sv << (unsigned int)obj.green << ","sv << (unsigned int)obj.blue << ","sv << obj.opacity << ")"sv;
    }

    std::ostream& operator<<(std::ostream& out, const Color& obj) {
        std::visit([&out](auto value) {PrintColor(out, value); }, obj);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& obj) {
        switch (obj) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& obj) {
        switch (obj) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        }
        return out;
    }

    // ---------- RenderContext ----------------

    RenderContext RenderContext::Indented() const {
        return { out, indent_step, indent + indent_step };
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
    // ---------- Object ----------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>\n"sv;
    }


    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points.push_back(point);
        return *this;
    }
    //<polyline points = "0,100 50,25 50,75 100,0" / >
    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points.size(); i++) {
            Point point = points[i];
            out << "" << point.x << "," << point.y;
            if (i != points.size() - 1) {
                out << " "sv;
            }
        }
        out << "\""sv;

        RenderAttrs(out);
        out << "/> \n"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << size_ << "\""sv;
        if (font_family_.size()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (font_weight_.size()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        RenderAttrs(out);
        out << ">";

        for (char c : data_) {
            if (c == '\"') {
                out << "&quot;"sv;
            }
            else if (c == '\'') {
                out << "&apos;"sv;
            }
            else if (c == '<') {
                out << "&lt;"sv;
            }
            else if (c == '>') {
                out << "&gt;"sv;
            }
            else if (c == '&') {
                out << "&amp;"sv;
            }
            else {
                out << c;
            }
        }

        out << "</text>\n"sv;
    }

    // ---------- Document ------------------



    // Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects.push_back(move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const {
        RenderContext context(out);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns = \"http://www.w3.org/2000/svg\" version = \"1.1\">\n"sv;
        for (auto& object : objects) {
            object->Render(context);
        }
        out << "</svg>"sv;
    }


}  // namespace svg