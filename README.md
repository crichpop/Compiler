# Тестовые программы

Эта папка содержит небольшие программы для проверки компилятора языка O.

## Локальный запуск

Из корня проекта:

```bash
cmake -S . -B build
cmake --build build
./build/Compiler tests/01_integer.o
```

Другой тест запускается так же:

```bash
./build/Compiler tests/02_boolean_true.o
./build/Compiler tests/03_boolean_comparison.o
./build/Compiler tests/04_boolean_if_while.o
./build/Compiler tests/05_odd.o
```

## Ожидаемые ошибки

Эти файлы нужны для проверки контекстных ошибок:

```bash
./build/Compiler tests/bad_01_integer_to_boolean.o
./build/Compiler tests/bad_02_boolean_to_integer.o
./build/Compiler tests/bad_03_boolean_arithmetic.o
./build/Compiler tests/bad_04_unknown_type.o
```

Если компилятор показывает сообщение об ошибке, это правильное поведение.

## Запуск через Docker

Собрать образ:

```bash
docker build -t o-compiler .
```

Запустить корректный тест:

```bash
docker run --rm o-compiler tests/02_boolean_true.o
```

Запустить тест с ожидаемой ошибкой:

```bash
docker run --rm o-compiler tests/bad_01_integer_to_boolean.o
```

Можно передать любой файл из папки `tests`.
