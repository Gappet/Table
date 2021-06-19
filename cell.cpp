#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Р РµР°Р»РёР·СѓР№С‚Рµ СЃР»РµРґСѓСЋС‰РёРµ РјРµС‚РѕРґС‹
Cell::Cell() {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>("");
    }
    else {
        if (text[0] == '=') {
            if (text.size() == 1) {
                impl_ = std::make_unique<TextImpl>(text);
            }
            else {
                impl_ = std::make_unique<FormulaImpl>(text);
            }
        }
        else {
            impl_ = std::make_unique<TextImpl>(text);
        }
    }
}

void Cell::Clear() {
    //impl_ = std::make_unique<EmptyImpl>("");
    impl_.reset();
}

Cell::Value Cell::GetValue() const {
    return impl_.get()->GetValue();
}
std::string Cell::GetText() const {
    return impl_.get()->GetText();
}