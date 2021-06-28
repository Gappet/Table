#pragma once

#include "common.h"
#include "formula.h"
#include <variant>
#include <optional>
#include <iostream>

using namespace std::literals;


class Impl {
public:
    Impl(std::string value) : value_(value) {}

    std::string& GetString() {
        return value_;
    }

    virtual CellInterface::Value GetValue() = 0;

    virtual std::string GetText() = 0;

    virtual ~Impl() = default;

    std::string value_;
};

class EmptyImpl : public Impl {
public:

    EmptyImpl(std::string value) : Impl(value) {}

    std::string GetText() override {
        return ""s;
    }

    CellInterface::Value GetValue() override {
        return ""s;
    }

};

class TextImpl : public Impl {
public:
    TextImpl(std::string value) : Impl(value) {}

    std::string GetText() override {
        return GetString();
    }

    CellInterface::Value GetValue() override {
        if (GetString()[0] == '\'') {
            return GetString().substr(1);
        }
        return GetString();
    }
};

class FormulaImpl : public Impl {
public:

    FormulaImpl(std::string value, SheetInterface* sheet) : Impl(value), formula_(ParseFormula(value.substr(1))), sheet_(sheet) {}

    std::string GetText() override {
        return "="s + formula_.get()->GetExpression();
    }

    CellInterface::Value GetValue() override {
        FormulaInterface::Value out = formula_->Evaluate(*sheet_);
        if (std::get_if<double>(&out)) {
            return std::get<double>(out);
        }
        return std::get<FormulaError>(out);
    }

    std::vector<Position> GetReferencedCells() {
        return formula_->GetReferencedCells();
    }

private:
    std::unique_ptr<FormulaInterface> formula_;
    SheetInterface* sheet_;
};

class Cell : public CellInterface {
public:
    Cell(SheetInterface* sheet, Position position);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsValid() const;

    void  ClearCache();

    bool IsFormula() const;

    bool IsCreate();

private:
    SheetInterface* sheet_;
    Position position_;
    std::shared_ptr<Impl> impl_ = nullptr;
    std::optional<Cell::Value> cash_ = std::nullopt;

};