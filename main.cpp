#include "iostream"
#include "sourceCodeDriver/Driver.h"
#include "parser/Parser.h"
#include "error/Error.h"
#include "scanner/Scanner.h"

int main(int argc, char const *argv[])
{   
    std::cout << "Компилятор языка O" << std::endl;
    Driver driver;

    if (argc < 2) 
    {
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->error("В качестве аргумента должен быть указан файл *.o");
    }

    driver.resetText(argv[1]);

    Scanner scanner(driver);
    TableOfName table;
    Parser parser(scanner, table);
    parser.compile();

    std::cout << "Скомпилировано";

    return 0;
}
