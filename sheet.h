#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>
#include <map>
#include <algorithm> 
#include <set>
class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    std::set<Position>& GetDependentCells(Position pos) const;

    std::vector<Position>& GetUsedCells(Position pos) const;

    void IsCicled(Position& pos, std::vector<Position>& poses);

    void AddToGraph(const Position& pos, std::vector<Position>& poses) {

        for (const Position& ps : used_cells_[pos]) {
            dependent_cells_[ps].extract(pos);
        }
                
        used_cells_[pos] = {poses.begin(), poses.end()};

        for (const Position& ps : poses) {
            dependent_cells_[ps].insert(pos);
        }

        for (const Position& ps : dependent_cells_[pos]) {
            dynamic_cast<Cell*>(GetCell(ps))->ClearCache();
        }

        dynamic_cast<Cell*>(GetCell(pos))->ClearCache();
    }

    void IncreaseSize(Position& pos);

    





private:
    // Можете дополнить ваш класс нужными полями и методами

    void IsCicled(Position& pos, std::set<Position>& poses, std::set<Position>& buffer);

    std::vector<std::vector<std::shared_ptr<Cell>>> fields_;
    int max_col_ = 0;
    int max_row_ = 0;

    std::map<Position, std::set<Position>> used_cells_;
    std::map<Position, std::set<Position>> dependent_cells_;
};

