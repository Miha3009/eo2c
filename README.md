Компилятор языка программирования EOLANG в язык программирования C++

## Сборка
Компилятор использует CMake. Для сборки нужно выполнить команду:

`cmake -Bbuild . && cd build && make`

Исполняемый файл появится в папке bin. 

## Запуск

Для начала можно запустить все тесты: `eo2c dataize -s ../tests/eo/functional -b test_build -m test.test-all`

В примере выполняется команда датаизации, то есть код генерируется, компилируется и запускается.
Флаг -s определяет папку с исходным кодом, -b папку с генерируемым кодом, -m объект main.

Все команды:

* generate. Только генерирует код.

* compile. Генерирует код и компилирует его.

* dataize. Генерирует, компилирует и запускает программу.

* configure. Запоминает флаги в конфиг.

* clean. Удаляет сгенерированный код.

Остальные флаги:

* -a (argument) передает аргументы в исполняемый файлы аргументы. Пример использования ниже.

* -f (flags) флаги для CMake. Например, можно указать какой компилятор C++ использовать.

* -l (lib) устанавливает тип проекта "библиотека". Для этого типа не генерируется main.

* -p (project) задает имя проекту. Исполняемый файл будет иметь это название. По умолчанию eolang.

* -q (quiet) отключает вывод компилятора.

* --stack задает размер виртуального стека в байтах.

Пример. Программа, считающая сумму двух чисел:

```
+package example
+alias org.eolang.io.stdout
+alias org.eolang.seq

[x y] > sumOfTwo
  [text value] > print
    stdout > @
      QQ.txt.sprintf "%s равно %d\n" text value

  [] > sum
    plus. > @
      x
      y

  seq > @
    print "X" x
    print "Y" y
    print "X+Y" sum
```

Пусть она лежит в example/example.eo, тогда запустить её можно командой: `eo2c dataize -q -s example -b example_build -m example.sumOfTwo -a "3 4"`. В данном случае будет найдена сумма чисел 3 и 4.

## Основные проблемы

Сейчас почти не сделаны константы (которые определяются спец. символом `!`).

Нужно добавить больше переименований для ключевых слов/библиотечных функций. Например, сейчас нельзя назвать объект main.

Некоторые программы могут завершается аварийно или выводит stackoverflow (кастомный) из-за моих ошибок с адресами, клонированием и другими связанными с памятью. Их можно избежать, если отключить очищение памяти. Для этого в bin/templates/object.h нужно закоментировать тело функции `stack_restore`.