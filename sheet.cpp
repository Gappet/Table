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

    //std::shared_ptr<Cell> tmp = std::make_shared<Cell>(this, pos);
    //tmp->Set(text);

    if (fields_.size() <= static_cast<size_t>(pos.row)) {
        fields_.resize(pos.row * 2 + 1);
    }

    if (fields_[pos.row].size() <= static_cast<size_t>(pos.col)) {
        fields_.at(pos.row).resize(pos.col * 2 + 1);
    }

    if (fields_[pos.row][pos.col] == nullptr) {
        fields_[pos.row][pos.col] = std::make_shared<Cell>(this, pos);
    }
    fields_[pos.row][pos.col].get()->Set(text);

    //if (max_col_ <= pos.col) {
    //    max_col_ = pos.col + 1;
    //}

    //if (max_row_ <= pos.row) {
    //    max_row_ = pos.row + 1;
    //}
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


void Sheet::IsCicled(Position& pos, std::vector<Position>& poses) {
    if (std::find(poses.begin(), poses.end(), pos) !=poses.end()) {
        throw CircularDependencyException(pos.ToString() + " position is circled");
    }

    for (const Position& ps : poses) {
        if (used_cells_.count(ps)) {
            if (used_cells_[ps].find(pos) != used_cells_[ps].end()) {
                throw CircularDependencyException("");
            }
        }
    }

    std::set<Position> buffer{ poses.begin(), poses.end() };
    for (const Position& ps : poses) {
        if (used_cells_.count(ps)) {
            std::cout << 2222 << std::endl;
            IsCicled(pos, used_cells_[ps], buffer);
        }        
    }
    

}

void Sheet::IsCicled(Position& pos, std::set<Position>& poses, std::set<Position>& buffer) {
    if (poses.find(pos) != poses.end()) {
        std::cout << 11111111 << std::endl;
        throw CircularDependencyException(pos.ToString() + " use in formula");
    }
        

    for (const Position& ps : poses) {
        if (buffer.find(ps) == buffer.end()) {
            if (used_cells_.count(ps)) {
                if (used_cells_.at(ps).find(pos) != used_cells_.at(ps).end()) {
                    throw CircularDependencyException("11111");
                }
            }
            buffer.insert(ps);
        }
    }

    for (const Position& ps : poses) {
        if (used_cells_.count(pos)) {
            IsCicled(pos, used_cells_.at(ps), buffer);
        }
    }

}

void Sheet::IncreaseSize(Position& pos) {
    if (max_col_ <= pos.col) {
        max_col_ = pos.col + 1;
    }

    if (max_row_ <= pos.row) {
        max_row_ = pos.row + 1;
    }

}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
