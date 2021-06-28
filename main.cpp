#include "common.h"
#include "test_runner_p.h"

inline std::ostream& operator<<(std::ostream& output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

inline Position operator"" _pos(const char* str, std::size_t) {
    return Position::FromString(str);
}

inline std::ostream& operator<<(std::ostream& output, Size size) {
    return output << "(" << size.rows << ", " << size.cols << ")";
}

inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit(
        [&](const auto& x) {
            output << x;
        },
        value);
    return output;
}

namespace {




    void TestEmpty() {
        auto sheet = CreateSheet();
        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 0, 0 }));
    }

    void TestInvalidPosition() {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell(Position{ -1, 0 }, "");
        }
        catch (const InvalidPositionException&) {
        }
        try {
            sheet->GetCell(Position{ 0, -2 });
        }
        catch (const InvalidPositionException&) {
        }
        try {
            sheet->ClearCell(Position{ Position::MAX_ROWS, 0 });
        }
        catch (const InvalidPositionException&) {
        }
    }

    void TestSetCellPlainText() {
        auto sheet = CreateSheet();

        auto checkCell = [&](Position pos, std::string text) {
            sheet->SetCell(pos, text);
            CellInterface* cell = sheet->GetCell(pos);
            ASSERT(cell != nullptr);
            ASSERT_EQUAL(cell->GetText(), text);
            ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), text);
        };

        checkCell("A1"_pos, "Hello");
        checkCell("A1"_pos, "World");
        checkCell("B2"_pos, "Purr");
        checkCell("A3"_pos, "Meow");

        const SheetInterface& constSheet = *sheet;
        ASSERT_EQUAL(constSheet.GetCell("B2"_pos)->GetText(), "Purr");

        sheet->SetCell("A3"_pos, "'=escaped");
        CellInterface* cell = sheet->GetCell("A3"_pos);
        ASSERT_EQUAL(cell->GetText(), "'=escaped");
        ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), "=escaped");
    }

    void TestClearCell() {
        auto sheet = CreateSheet();

        sheet->SetCell("C2"_pos, "Me gusta");
        sheet->ClearCell("C2"_pos);
        ASSERT(sheet->GetCell("C2"_pos) == nullptr);

        sheet->ClearCell("A1"_pos);
        sheet->ClearCell("J10"_pos);
    }
    void TestPrint() {
        auto sheet = CreateSheet();
        sheet->SetCell("A2"_pos, "meow");
        sheet->SetCell("B2"_pos, "=1+2");
        sheet->SetCell("A1"_pos, "=1/0");

        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 2, 2 }));

        std::ostringstream texts;
        sheet->PrintTexts(texts);
        ASSERT_EQUAL(texts.str(), "=1/0\t\nmeow\t=1+2\n");

        std::ostringstream values;
        sheet->PrintValues(values);
        ASSERT_EQUAL(values.str(), "#DIV/0!\t\nmeow\t3\n");

        sheet->ClearCell("B2"_pos);
        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 2, 1 }));
    }

    void MyTest() {
        auto sheet = CreateSheet();
        sheet->SetCell("A1"_pos, "meow");
        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 1, 1 }));
        std::ostringstream values;
        sheet->PrintValues(values);
        ASSERT_EQUAL(values.str(), "meow\n");
    }

    void MyTestII() {
        auto sheet = CreateSheet();
        //std::cout << "test begin" << std::endl;
        sheet->SetCell("A1"_pos, "meow");
        sheet->SetCell("A2"_pos, "meow");
        sheet->SetCell("B1"_pos, "meow");
        sheet->ClearCell("A1"_pos);
        //std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        sheet->ClearCell("A2"_pos);
        //std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        sheet->ClearCell("B1"_pos);
        /*std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;*/
        ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 0, 0 }));
        std::ostringstream values;
        //std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        sheet->PrintValues(values);
        //std::cout << values.str() << std::endl;
        ASSERT_EQUAL(values.str(), "");           
    }


    void MyTestIII() {
        auto sheet = CreateSheet();
        sheet->SetCell("A1"_pos, "meowI");
        std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        sheet->SetCell("A2"_pos, "meowII");
        std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        sheet->SetCell("B1"_pos, "meowIII");
        std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        sheet->SetCell("B2"_pos, "meowV");
        std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        std::ostringstream values;
        sheet->PrintValues(values);
        std::cout << values.str() << std::endl;
        sheet->ClearCell("A1"_pos);
        std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        std::ostringstream valuesI;
        sheet->PrintValues(valuesI);
        std::cout << valuesI.str() << std::endl;
        sheet->ClearCell("B2"_pos);
        std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        std::ostringstream valuesII;
        sheet->PrintValues(valuesII);
        std::cout << valuesII.str() << std::endl;
        sheet->ClearCell("A1"_pos);
        std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        std::ostringstream valuesIII;
        sheet->PrintValues(valuesIII);
        std::cout << valuesIII.str() << std::endl;
        sheet->ClearCell("B1"_pos);
        std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        std::cout << "/////////////" << std::endl;
        std::ostringstream values2;
        std::cout << sheet->GetPrintableSize().cols << " " << sheet->GetPrintableSize().rows << std::endl;
        sheet->PrintTexts(values2);
        std::cout << values2.str() << std::endl;
        //std::cout << "meow" << '\t' << '\t' << '\n' << '\t' <<"meow" << std::endl;


    }

    void MyTestIV() {
        auto sheet = CreateSheet();
        sheet->SetCell("A1"_pos, "=3");
        sheet->SetCell("A2"_pos, "=1+7*2");
        sheet->SetCell("A3"_pos, "=A2/A1");
        std::ostringstream values2;
        sheet->PrintValues(values2);
        //sheet->PrintTexts(values2);
        //std::cout << values2.str() << std::endl;
        ASSERT_EQUAL(values2.str(), "3\t\n15\t\n5\n");

    }

    void MyTestV() {
        auto sheet = CreateSheet();
        sheet->SetCell("A2"_pos, "=3");
        sheet->SetCell("A3"_pos, "=A1/A2");
        std::ostringstream values;
        sheet->PrintValues(values);
        ASSERT_EQUAL(values.str(), "\n3\t\n0\n");
        sheet->SetCell("A3"_pos, "=A2/A1");
        std::ostringstream values2;
        sheet->PrintValues(values2);
        ASSERT_EQUAL(values2.str(), "\n3\t\n#DIV0!\n");
    }


    void MyTestVI() {
        auto sheet = CreateSheet();
        sheet->SetCell("A1"_pos, "=3");
        //sheet->SetCell("A2"_pos, "test");
        //sheet->SetCell("A3"_pos, "=A2*A1");
        sheet->SetCell("A4"_pos, "=A2*A100");
        std::ostringstream values2;
        sheet->PrintValues(values2);
        std::cout << values2.str() << std::endl;
        //ASSERT_EQUAL(values2.str(), "\n3\t\n#DIV0!\n");
    }

    void MyTestVII() {
        auto sheet = CreateSheet();
        sheet->SetCell("A2"_pos, "text");
        sheet->SetCell("A3"_pos, "=1+2*7");
        sheet->SetCell("C2"_pos, "=A3/A2");
        sheet->SetCell("C4"_pos, "=C2+8");

        std::ostringstream values2;
        sheet->PrintValues(values2);
        std::cout << values2.str() << std::endl;

    }

    void MyTestVIII() {
        auto sheet = CreateSheet();
        sheet->SetCell("A2"_pos, "=3");        
        sheet->SetCell("C2"_pos, "=A3/A2");
        sheet->SetCell("C4"_pos, "=C2+8");
        std::ostringstream values2;
        sheet->PrintTexts(values2);
        std::cout << values2.str() << std::endl;

        sheet->SetCell("A3"_pos, "=C4-1");
        std::cout << 2 << std::endl;
        std::ostringstream values;
        sheet->PrintTexts(values);
        std::cout << values.str() << std::endl;


    }

    void MyTestVIV() {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell("A2"_pos, "=A1+*");
        }
        catch (FormulaError& a) {
            std::cout << 1 << std::endl;
        }
        catch (CircularDependencyException& a) {
            std::cout << 3 << std::endl;

        }
        catch(InvalidPositionException& a) {
            std::cout << 2 << std::endl;
        }
        catch (FormulaException& e) {
            std::cout << 2333 << std::endl;
        }
        

    }





}  // namespace



int main() {
    TestRunner tr;
    //RUN_TEST(tr, TestEmpty);
    //RUN_TEST(tr, TestInvalidPosition);
    //RUN_TEST(tr, TestSetCellPlainText);
    //RUN_TEST(tr, TestClearCell);
    //RUN_TEST(tr, TestPrint);
    //RUN_TEST(tr, MyTest);
    //RUN_TEST(tr, MyTestII);
    //RUN_TEST(tr, MyTestIII);
    //RUN_TEST(tr, MyTestIV);
    //RUN_TEST(tr, MyTestV);
    //RUN_TEST(tr, MyTestVI);
    //RUN_TEST(tr, MyTestVII);
    //RUN_TEST(tr, MyTestVIII);
    RUN_TEST(tr, MyTestVIV);

    std::cout << "all done" << std::endl;
    return 0;
}
