#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {}

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute([&sheet](const Position& pos) {
                if (sheet.GetCell(pos) == nullptr) {
                    return CellInterface::Value(0.0);
                }
                return sheet.GetCell(pos)->GetValue(); });
        } catch (FormulaError& out) {
            return out;            
        }
    }
    std::string GetExpression() const override {
        std::stringstream out;        
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        return {ast_.GetCells().begin(), ast_.GetCells().end()};
    }

    

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}