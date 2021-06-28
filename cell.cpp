#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <sstream>
#include <ctype.h>


Cell::Cell(SheetInterface* sheet, Position position) : sheet_(sheet), position_(position) {}

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
                std::shared_ptr<FormulaImpl> tmp = std::make_unique<FormulaImpl>(text, sheet_);
                impl_ = tmp;
            }
        }
        else {
            impl_ = std::make_unique<TextImpl>(text);
            dynamic_cast<Sheet*>(sheet_)->IncreaseSize(position_);
        }
    }
}

void Cell::Clear() {
    ClearCache();
    impl_.reset();
}

Cell::Value Cell::GetValue() const {
    if (IsFormula()) {
        if (IsValid()) {
            return *cash_;
        }
        else {
            for (const auto& pos : GetReferencedCells()) {
                try {
                    Size s = dynamic_cast<Sheet*>(sheet_)->GetPrintableSize();
                    if (s.cols - 1 < pos.col || s.rows - 1 < pos.row) {
                        return FormulaError(FormulaError::Category::Ref);
                    }
                    auto cell = reinterpret_cast<Sheet*>(sheet_)->GetCell(pos);
                    if (cell != nullptr) {
                        if (!reinterpret_cast<Cell*>(cell)->IsFormula()) {
                            return FormulaError(FormulaError::Category::Value);
                        }
                        if (!reinterpret_cast<Cell*>(cell)->IsValid()) {
                            reinterpret_cast<Cell*>(cell)->GetValue();
                        }
                    }
                }
                catch (InvalidPositionException& out) {
                    return FormulaError(FormulaError::Category::Ref);
                }
            }

            auto out = impl_->GetValue();
            if (std::get_if<double>(&out)) {
                const_cast<std::optional<Cell::Value>&>(cash_) = std::get<double>(out);
            }
            else {
                const_cast<std::optional<Cell::Value>&>(cash_) = std::get<FormulaError>(out);
            }

            return *cash_;
        }

    }
    return impl_->GetValue();
}

std::vector<Position> Cell::GetReferencedCells() const {
    if (!IsFormula()) {
        return {};
    }
    return dynamic_cast<FormulaImpl*>(impl_.get())->GetReferencedCells();

}
std::string Cell::GetText() const {
    return impl_.get()->GetText();
}

bool Cell::IsValid() const {
    return cash_.has_value();
}

void Cell::ClearCache() {
    cash_.reset();
}

bool Cell::IsFormula() const {
    return dynamic_cast<FormulaImpl*>(impl_.get()) != nullptr;
}

bool Cell::IsCreate() {
    return impl_ != nullptr;
}

