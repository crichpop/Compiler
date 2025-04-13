#include "Parser.h"
#include "../sourceCodeDriver/Driver.h"
#include "../scanner/Scanner.h"
#include "../error/Error.h"
#include "../table/TableOfName.h"
#include "../table/Item.h"
#include <string>

Parser::Parser(Scanner &scanner, TableOfName &table) : scanner(scanner), table(table) {
    scanner.nextLex();
} 

void Parser::compile() 
{
    table.openScope(); // Блок стандартных идентификаторов
    table.addItem(table.functionItem("ABS", Item::ItemTypes::Integer));
    table.addItem(table.functionItem("MIN", Item::ItemTypes::Integer));
    table.addItem(table.functionItem("MAX", Item::ItemTypes::Integer));
    table.addItem(table.functionItem("ODD", Item::ItemTypes::Boolean));

    table.addItem(table.procedureItem("HALT"));
    table.addItem(table.procedureItem("INC"));
    table.addItem(table.procedureItem("DEC"));
    table.addItem(table.procedureItem("In.Open"));
    table.addItem(table.procedureItem("In.Int"));
    table.addItem(table.procedureItem("Out.Int"));
    table.addItem(table.procedureItem("Out.Ln"));

    table.addItem(table.typeItem("INTEGER", Item::ItemTypes::Integer));
    
    table.openScope(); // Блок модуля

    modulePrc();

    table.closeScope();
    table.closeScope();
}

// MODULE Имя ";"
// [Импорт]
// ПослОбъявл
// [BEGIN
//   ПослОператоров]
// END Имя "."
void Parser::modulePrc() 
{
    checkLex(Scanner::Lex::MODULE);
    // checkLex(Scanner::Lex::NAME);

    errorIfNotExpectedLex(Scanner::Lex::NAME);
    table.newItem(table.moduleItem(scanner.nameValue));
    scanner.nextLex();
    
    checkLex(Scanner::Lex::SEMI);

    if (scanner.lex == Scanner::Lex::IMPORT)
    {
        // IMPORT опциональный
        // распознающая процедура для нетерминала IMPORT
        importPrc();
    }

    sequenceDeclarationsPrc();

    if (scanner.lex == Scanner::Lex::BEGIN) 
    {
        scanner.nextLex();
        sequenceStatementsPrc();
    }

    checkLex(Scanner::Lex::END);
    // checkLex(Scannerx::Lex::NAME);

    errorIfNotExpectedLex(Scanner::Lex::NAME);
    Item item = table.findItem(scanner.nameValue);
    if (item.typeOfItem != "module")
    {
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->syntaxError("имя модуля");
    }
    else if (item.name != scanner.nameValue)
    {
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->syntaxError("имя модуля" + scanner.nameValue);
    }

    scanner.nextLex();
    checkLex(Scanner::Lex::DOT);
    
}

// IMPORT Имя {"," Имя} ";".
void Parser::importPrc()
{
    checkLex(Scanner::Lex::IMPORT);
    contextImportPrc();
    // checkLex(Scanner::Lex::NAME);
    while (scanner.lex == Scanner::Lex::COMMA) {
        checkLex(Scanner::Lex::COMMA);
        contextImportPrc();
        // checkLex(Scanner::Lex::NAME);
    }
    checkLex(Scanner::Lex::SEMI);
}

void Parser::sequenceDeclarationsPrc() 
{
    while (scanner.lex == Scanner::Lex::CONST || scanner.lex == Scanner::Lex::VAR)
    {
        if (scanner.lex == Scanner::Lex::CONST) 
        {
            checkLex(Scanner::Lex::CONST);
            while (scanner.lex == Scanner::Lex::NAME) 
            {
                constDeclarationPrc();
                checkLex(Scanner::Lex::SEMI);
            }
        }
        else 
        {
            checkLex(Scanner::Lex::VAR);
            while (scanner.lex == Scanner::Lex::NAME) 
            {
                varDeclarationPrc();
                checkLex(Scanner::Lex::SEMI);
            }
        }
    }
}

void Parser::constDeclarationPrc()
{
    // checkLex(Scanner::Lex::NAME);
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    std::string constName = scanner.nameValue;
    scanner.nextLex();

    checkLex(Scanner::Lex::EQ);
    int constValue = constExpresionPrc();

    table.newItem(table.constItem(constName, Item::ItemTypes::Integer, std::to_string(constValue)));
}

int Parser::constExpresionPrc() 
{
    int sign = 1;
    
    // не проверяем деление на 0, переполнение и тп
    if (scanner.lex == Scanner::Lex::MINUS)
    {
        checkLex(Scanner::Lex::MINUS);
        sign = -1;

    }
    else if (scanner.lex == Scanner::Lex::PLUS)
    {
        checkLex(Scanner::Lex::PLUS);
    }

    if (scanner.lex == Scanner::Lex::NAME)
    {
        checkLex(Scanner::Lex::NAME);
        Item item = table.findItem(scanner.nameValue);
        if (item.typeOfItem != "const")
        {
            std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
            errorPtr->contextError("имя модуля");
        }
        else
        {
            return std::stoi(item.value) * sign;
        }
    }
    else if (scanner.lex == Scanner::Lex::NUM)
    {
        int constValue = scanner.numValue * sign;
        checkLex(Scanner::Lex::NUM);
        return constValue;
    }
    else
    {
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->syntaxError("имя константы или число");
    }
}

void Parser::varDeclarationPrc() 
{
    contextVarPrc();
    // checkLex(Scanner::Lex::NAME);
    while (scanner.lex == Scanner::Lex::COMMA)
    {
        checkLex(Scanner::Lex::COMMA);
        contextVarPrc();
        // checkLex(Scanner::Lex::NAME);
    }

    checkLex(Scanner::Lex::COLON);
    typePrc();
}

void Parser::typePrc() 
{
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    Item item = table.findItem(scanner.nameValue);

    if (item.typeOfItem != "type")
    {
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->contextError("Необьявленное имя типа");
    }

    scanner.nextLex();
}

void Parser::sequenceStatementsPrc() 
{
    statementsPrc();
    while (scanner.lex == Scanner::Lex::SEMI)
    {
        scanner.nextLex();
        statementsPrc();
    }
}

// [
//   Переменная ":=" Выраж
//   | [Имя "."] Имя ["(" Параметр {"," Параметр}] ")"]
//   | IF Выраж THEN
//     ПослОператоров
//   {ELSIF Выраж THEN
//     ПослОператоров}
//   [ELSE
//     ПослОператоров]
//    END
//   | WHILE Выраж DO
//     ПослОператоров
//   END
// ]
void Parser::statementsPrc()
{
    if (scanner.lex == Scanner::Lex::NAME)
    {
        variableOrCallPrc();
    } 
    else if (scanner.lex == Scanner::Lex::IF)
    {
        ifStatementPrc();
    }
    else if (scanner.lex == Scanner::Lex::WHILE)
    {
        whileStatementPrc();
    }
}

//   Переменная ":=" Выраж
//   | [Имя "."] Имя ["(" Параметр {"," Параметр}] ")"]
void Parser::variableOrCallPrc()
{
    // checkLex(Scanner::Lex::NAME);
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    Item item = table.findItem(scanner.nameValue);
    scanner.nextLex();

    if (item.typeOfItem == "var")
    {
        checkLex(Scanner::Lex::ASS);
        Item::ItemTypes expressionType = expressionPrc();
        if (item.type != expressionType) 
        {
            std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
            errorPtr->contextError("Неверный тип при присваивании");
        }
    }
    else if (item.typeOfItem == "procedure" || item.typeOfItem == "module")
    {
        if (scanner.lex == Scanner::Lex::DOT)
        {
            if (item.typeOfItem != "module")
            {
                std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
                errorPtr->contextError("Ожидается имя модуля");
            }
            scanner.nextLex();
            // checkLex(Scanner::Lex::NAME);
            errorIfNotExpectedLex(Scanner::Lex::NAME);
            std::string procedureName = item.name + "." + scanner.nameValue;
            Item procedureItem = table.findItem(procedureName);
            if (procedureItem.typeOfItem != "procedure")
            {
                std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
                errorPtr->contextError("Ожидается процедура");
            }
            scanner.nextLex();
        }
        else if (item.typeOfItem != "procedure")
        {
            std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
            errorPtr->contextError("Ожидается имя процедуры");
        }

        if (scanner.lex == Scanner::Lex::LPAR)
        {
            scanner.nextLex();
            if (scanner.lex != Scanner::Lex::RPAR)
            {
                parameterPrc();
                while (scanner.lex == Scanner::Lex::COMMA)
                {
                    scanner.nextLex();
                    parameterPrc();
                }
                checkLex(Scanner::Lex::RPAR);  
            }
            else
            {
                scanner.nextLex();
            }
        }   
    }
    else
    {
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->contextError("Ожидается имя перменной или процедуры");
    }
}

//   IF Выраж THEN
//     ПослОператоров
//   {ELSIF Выраж THEN
//     ПослОператоров}
//   [ELSE
//     ПослОператоров]
//    END
void Parser::ifStatementPrc()
{
    checkLex(Scanner::Lex::IF);
    expressionPrc();
    checkLex(Scanner::Lex::THEN);
    sequenceStatementsPrc();

    while (scanner.lex == Scanner::Lex::ELSIF)
    {
        checkLex(Scanner::Lex::ELSIF);
        expressionPrc();
        checkLex(Scanner::Lex::THEN);
        sequenceStatementsPrc();
    }

    if (scanner.lex == Scanner::Lex::ELSE)
    {
        checkLex(Scanner::Lex::ELSE);
        sequenceStatementsPrc();
    }

    checkLex(Scanner::Lex::END);
}

//   WHILE Выраж DO
//     ПослОператоров
//   END
void Parser::whileStatementPrc()
{
    checkLex(Scanner::Lex::WHILE);
    expressionPrc();
    checkLex(Scanner::Lex::DO);
    sequenceStatementsPrc();
    checkLex(Scanner::Lex::END);
}

// Параметр | Выражение
// Пока нет контекстного анализатора
void Parser::parameterPrc()
{
    expressionPrc();
}

// ПростоеВыраж [Отношение ПростоеВыраж]
Item::ItemTypes Parser::expressionPrc()
{
    Item::ItemTypes simpleExpressionType = simpleExpressionPrc();
    
    if (scanner.lex == Scanner::Lex::EQ ||
        scanner.lex == Scanner::Lex::NE ||
        scanner.lex == Scanner::Lex::LT ||
        scanner.lex == Scanner::Lex::LE ||
        scanner.lex == Scanner::Lex::GT ||
        scanner.lex == Scanner::Lex::GE)
    {
        checkIntType(simpleExpressionType);
        scanner.nextLex();
        simpleExpressionType = simpleExpressionPrc();
        checkIntType(simpleExpressionType);
        return Item::ItemTypes::Boolean;
    }
    else
    {
        return simpleExpressionType;
    }
}

// ["+"|"-"] Слагаемое {ОперСлож Слагаемое}.
Item::ItemTypes Parser::simpleExpressionPrc()
{
    Item::ItemTypes termType;
    if (scanner.lex == Scanner::Lex::PLUS ||
        scanner.lex == Scanner::Lex::MINUS)
    {
        scanner.nextLex();
        termType = termPrc();
        checkIntType(termType);
    }
    else
    {
        termType = termPrc();
        if (scanner.lex == Scanner::Lex::PLUS ||
            scanner.lex == Scanner::Lex::MINUS)
        {
            checkIntType(termType);
        }
    }

    while (scanner.lex == Scanner::Lex::PLUS ||
            scanner.lex == Scanner::Lex::MINUS)
    {
        scanner.nextLex();
        termType = termPrc();
        checkIntType(termType);
    }

    return termType;
}

// Множитель {ОперацияУмножения Множитель}
Item::ItemTypes Parser::termPrc()
{
    Item::ItemTypes multiplierType;
    multiplierType = multiplierPrc();

    if (scanner.lex == Scanner::Lex::DIV ||
            scanner.lex == Scanner::Lex::MOD ||
            scanner.lex == Scanner::Lex::MULT)
    {
        checkIntType(multiplierType);
    }

    while (scanner.lex == Scanner::Lex::DIV ||
            scanner.lex == Scanner::Lex::MOD ||
            scanner.lex == Scanner::Lex::MULT)
    {
        scanner.nextLex();
        multiplierType = multiplierPrc();
        checkIntType(multiplierType);
    }

    return multiplierType;
}

// Имя ["(" Выраж | Тип ")"]
// | Число
// | "(" Выраж ")".
Item::ItemTypes Parser::multiplierPrc()
{
    if (scanner.lex == Scanner::Lex::NAME)
    {
        Item item = table.findItem(scanner.nameValue);
        if (item.typeOfItem == "const" ||
            item.typeOfItem == "var")
        {
            scanner.nextLex();
            return item.type;
        }
        else if (item.typeOfItem == "function")
        {
            scanner.nextLex();
            checkLex(Scanner::Lex::LPAR);
            expressionPrc();
            checkLex(Scanner::Lex::RPAR);
            return item.type;
        }
        else
        {
            std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
            errorPtr->contextError("Ожидается константа, имя или функция");
        }
    }
    else if (scanner.lex == Scanner::Lex::NUM)
    {
        scanner.nextLex();
        return Item::ItemTypes::Integer;
    }
    else if (scanner.lex == Scanner::Lex::LPAR)
    {
        scanner.nextLex();
        Item::ItemTypes expressionType = expressionPrc();
        checkLex(Scanner::Lex::RPAR);
        return expressionType;
    }
    else
    {
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->syntaxError("Имя, число или '('");
    }

    // заглушка !!!!
    return Item::ItemTypes::Integer;
}

void Parser::contextImportPrc() 
{
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    if (scanner.nameValue == "In" || scanner.nameValue == "Out")
    {
        table.newItem(table.moduleItem(scanner.nameValue));
    }
    else
    {
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->contextError("Ожидается модуль `In` или `Out`");
    }

    scanner.nextLex();
}

void Parser::contextVarPrc()
{
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    table.newItem(table.varItem(scanner.nameValue, Item::ItemTypes::Integer, ""));
    scanner.nextLex();
}

void Parser::checkLex(Scanner::Lex lex)
{
    if (Scanner::lex == lex)
    {
        scanner.nextLex();
    }
    else 
    {
        std::string expected = scanner.getStringNameOfLex(lex);
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->syntaxError(expected);
    }
}

void Parser::errorIfNotExpectedLex(Scanner::Lex lex)
{
    if (Scanner::lex != lex)
    {
        std::string expected = scanner.getStringNameOfLex(lex);
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->syntaxError(expected);
    }
}

void Parser::checkIntType(Item::ItemTypes type)
{
    if (type != Item::ItemTypes::Integer)
    {
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->contextError("Ожидается целый тип");
    }
}