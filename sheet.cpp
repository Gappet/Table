#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <memory>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position"s);
    }

    if (fields_.size() <= static_cast<size_t>(pos.row)) {
        fields_.resize(pos.row * 2 + 1);
    }

    if (fields_[pos.row].size() <= static_cast<size_t>(pos.col)) {
        fields_.at(pos.row).resize(pos.col * 2 + 1);
    }
    fields_[pos.row][pos.col] = std::make_shared<Cell>();
    fields_[pos.row][pos.col].get()->Set(text);

    if (max_col_ <= pos.col) {
        max_col_ = pos.col + 1;
    }

    if (max_row_ <= pos.row) {
        max_row_ = pos.row + 1;
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position"s);
    }

    if (fields_.size() <= static_cast<size_t>(pos.row)) {
        return nullptr;
    }

    if (fields_[pos.row].size() <= static_cast<size_t>(pos.col)) {
        return nullptr;
    }

    if (fields_[pos.row][pos.col] == nullptr) {
        return nullptr;
    }


    return fields_.at(pos.row).at(pos.col).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position"s);
    }

    if (fields_.size() <= static_cast<size_t>(pos.row)) {
        return nullptr;
    }

    if (fields_[pos.row].size() <= static_cast<size_t>(pos.col)) {
        return nullptr;
    }

    if (fields_[pos.row][pos.col] == nullptr) {
        return nullptr;
    }
    return fields_[pos.row][pos.col].get();

}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position"s);
    }

    if (fields_.size() <= static_cast<size_t>(pos.row)) {
        return;
    }

    if (fields_[pos.row].size() <= static_cast<size_t>(pos.col)) {
        return;
    }

    if (fields_.at(pos.row).at(pos.col) != nullptr) {
        fields_.at(pos.row).at(pos.col).reset();
    }

    int new_max_rows = 0;
    int new_max_cols = 0;
    for (int i = 0; i < max_row_; ++i) {
        bool is_cell = false;
        int cnt = 0;
        for (int j = 0; j < static_cast<int>(fields_[i].size()); ++j) {
            if (this->GetCell({ static_cast<int>(i), static_cast<int>(j) }) != nullptr) {
                is_cell = true;
                cnt = j + 1;
            }
        }
        if (is_cell) {
            if (new_max_rows <= i) {
                new_max_rows = i + 1;
            }

            if (new_max_cols <= cnt) {
                new_max_cols = cnt;
            }
        }
    }

    max_col_ = new_max_cols;
    max_row_ = new_max_rows;
}

Size Sheet::GetPrintableSize() const {
    return { static_cast<int>(max_row_), static_cast<int>(max_col_) };
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    if (size.rows == 0) {
        return;
    }

    for (int i = 0; i < size.rows - 1; ++i) {
        for (int j = 0; j < static_cast<int>(fields_[i].size()); ++j) {
            if (size.cols <= static_cast<int>(j)) {
                break;
            }

            auto cell = GetCell({ i, j });
            if (cell != nullptr) {
                auto out = cell->GetValue();
                if (std::get_if<double>(&out)) {
                    output << std::get<double>(out);
                }
                else if (std::get_if<std::string>(&out)) {
                    output << std::get<std::string>(out);
                }
                else {
                    output << std::get<FormulaError>(out);
                }
            }
            output << '\t';
        }
        output << '\n';
    }

    bool first = true;
    for (int j = 0; j < static_cast<int>(fields_[size.rows - 1].size()); ++j) {
        if (size.cols <= static_cast<int>(j)) {
            break;
        }

        if (!first) {
            output << '\t';
        }
        first = false;
        auto cell = GetCell({ size.rows - 1,  j });
        if (cell != nullptr) {
            auto out = cell->GetValue();
            if (std::get_if<double>(&out)) {
                output << std::get<double>(out);
            }
            else if (std::get_if<std::string>(&out)) {
                output << std::get<std::string>(out);
            }
            else {
                output << std::get<FormulaError>(out);
            }
        }
    }
    output << '\n';
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    if (size.rows == 0) {
        return;
    }

    for (int i = 0; i < size.rows - 1; ++i) {
        for (int j = 0; j < static_cast<int>(fields_[i].size()); ++j) {
            if (size.cols <= j) {
                break;
            }

            auto cell = GetCell({ i,  j });
            if (cell != nullptr) {
                output << cell->GetText();
            }
            output << '\t';
        }
        output << '\n';
    }

    bool first = true;
    for (int j = 0; j < static_cast<int>(fields_[size.rows - 1].size()); ++j) {
        if (size.cols <= static_cast<int>(j)) {
            break;
        }

        if (!first) {
            output << '\t';
        }
        first = false;
        auto cell = GetCell({ size.rows - 1,  static_cast<int>(j) });
        if (cell != nullptr) {
            output << cell->GetText();
        }
    }
    output << '\n';
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}