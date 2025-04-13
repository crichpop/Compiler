#include "../scanner/Scanner.h"
#include "../table/TableOfName.h"

#pragma once

class Parser
{
public:
    Parser(Scanner &scanner, TableOfName &table);
    void compile();

private:
    Scanner scanner;
    TableOfName table;

    void modulePrc();
    void importPrc();
    void sequenceDeclarationsPrc();
    void sequenceStatementsPrc();
    void constDeclarationPrc();
    int constExpresionPrc();
    void varDeclarationPrc();
    void statementsPrc();
    void variableOrCallPrc();
    void typePrc();
    void ifStatementPrc();
    void whileStatementPrc();
    void parameterPrc();
    Item::ItemTypes expressionPrc();
    Item::ItemTypes simpleExpressionPrc();
    Item::ItemTypes termPrc();
    Item::ItemTypes multiplierPrc();

    void contextImportPrc();
    void contextVarPrc();

    void checkLex(Scanner::Lex lex);
    void errorIfNotExpectedLex(Scanner::Lex lex);
    void checkIntType(Item::ItemTypes type);
};

