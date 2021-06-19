#pragma once

#include "common.h"
#include "formula.h"
#include <variant>

using namespace std::literals;


class Impl {
public:
    Impl(std::string value) : value_(value) {}

    void Set(std::string& value);

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

    FormulaImpl(std::string value) : Impl(value), formula_(ParseFormula(value.substr(1))) {
        /*formula_ = ParseFormula(value);*/
    }

    std::string GetText() override {
        //std::cout << formula_.get()->GetExpression() << std::endl;
        return "="s + formula_.get()->GetExpression();
    }

    CellInterface::Value GetValue() override {
        FormulaInterface::Value out = formula_.get()->Evaluate();
        if (std::get_if<double>(&out)) {
            return std::get<double>(out);
        }
        return std::get<FormulaError>(out);
    }

private:

    std::unique_ptr<FormulaInterface> formula_;
};

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;


    //можете воспользоваться нашей подсказкой, но это необязательно.


private:

    std::shared_ptr<Impl> impl_ = nullptr;

};