#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <sstream>


Cell::Cell(SheetInterface* sheet, Position position) : sheet_(sheet), position_(position) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>("");
        is_formula_ = false;
    }
    else {
        if (text[0] == '=') {
            if (text.size() == 1) {
                impl_ = std::make_unique<TextImpl>(text);
                is_formula_ = false;
            }
            else {
                try {
                    std::shared_ptr<FormulaImpl> tmp = std::make_unique<FormulaImpl>(text, sheet_);
                    auto poses = tmp->GetReferencedCells();                    
                    dynamic_cast<Sheet*>(sheet_)->IsCicled(position_, poses);
                    dynamic_cast<Sheet*>(sheet_)->AddToGraph(position_, poses);
                    dynamic_cast<Sheet*>(sheet_)->IncreaseSize(position_);
                    is_formula_ = true;
                    impl_ = tmp;
                }
                catch (CircularDependencyException& out) {
                    //std::cout << 1 << std::endl;
                    return;

                }
            }
        }
        else {
            impl_ = std::make_unique<TextImpl>(text);
            dynamic_cast<Sheet*>(sheet_)->IncreaseSize(position_);
            is_formula_ = false;
        }
    }
}

void Cell::Clear() {
    ClearCache();
    impl_.reset();
}

Cell::Value Cell::GetValue() const{
    if (IsFormula()) {
        if (IsValid()) {
            return *cash_;
        }
        else {
            for (const auto& pos : GetReferencedCells()) {
                try {
                    Size s = dynamic_cast<Sheet*>(sheet_)->GetPrintableSize();
                    if ( s.cols - 1 < pos.col || s.rows - 1 < pos.row) {
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

bool Cell::IsValid() const{
    return cash_.has_value();
}

void Cell::ClearCache() {
    cash_.reset();
}

bool Cell::IsFormula() const {
    //bool a = dynamic_cast<FormulaImpl*>(impl_.get()) != nullptr;
    //std::cout << a << std::endl;
    return dynamic_cast<FormulaImpl*>(impl_.get()) != nullptr;
    //return is_formula_;
}


bool Cell::IsCreate() {
    return impl_ != nullptr;
}