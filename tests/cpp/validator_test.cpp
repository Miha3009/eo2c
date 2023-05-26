#include "../../include/catch.hpp"
#include "validator.h"
#include "helper.h"
#include <iostream>

static NullOstream s;

/*
+alias package.y

[] > test1
  x > @

[] > test2
  [] > x
  x > @

[] > test3
  1 > x
  x > @

[x] > test4
  x > @

[] > test5
  [] > x
  [] > a
    x > @

[] > test6
  y > @

[x] > test7
  a > []
    x > @
*/
TEST_CASE("Context test") {
    TranslationUnit unit = makeUnit();
    ImportsMap map;
    Validator v(unit, map);
    unit.metas.push_back({"alias", "package.y"});
    v.setErrorStream(&s);

    Object* test1 = unit.root->makeChild(CLASS_TYPE, "test1");
    Object* test1_decorator = test1->makeChild(APPLICATION_TYPE, "@");
    Object* test1_var = test1_decorator->makeChild(VAR_TYPE, "x");
    REQUIRE(!v.checkContext(test1_var));

    Object* test2 = unit.root->makeChild(CLASS_TYPE, "test2");
    Object* test2_child = test2->makeChild(CLASS_TYPE, "x");
    Object* test2_decorator = test2->makeChild(APPLICATION_TYPE, "@");
    Object* test2_var = test2_decorator->makeChild(VAR_TYPE, "x");
    REQUIRE(v.checkContext(test2_var));

    Object* test3 = unit.root->makeChild(CLASS_TYPE, "test3");
    Object* test3_child = test3->makeChild(APPLICATION_TYPE, "x");
    test3_child->makeChild(INT_TYPE, "1");
    Object* test3_decorator = test3->makeChild(APPLICATION_TYPE, "@");
    Object* test3_var = test3_decorator->makeChild(VAR_TYPE, "x");
    REQUIRE(v.checkContext(test3_var));

    Object* test4 = unit.root->makeChild(CLASS_TYPE, "test4");
    test4->addAttribute("x");
    Object* test4_decorator = test4->makeChild(APPLICATION_TYPE, "@");
    Object* test4_var = test4_decorator->makeChild(VAR_TYPE, "x");
    REQUIRE(v.checkContext(test4_var));

    Object* test5 = unit.root->makeChild(CLASS_TYPE, "test5");
    Object* test5_child = test5->makeChild(CLASS_TYPE, "x");
    Object* test5_child2 = test5->makeChild(CLASS_TYPE, "a");
    Object* test5_decorator = test5_child2->makeChild(APPLICATION_TYPE, "@");
    Object* test5_var = test5_decorator->makeChild(VAR_TYPE, "x");
    REQUIRE(v.checkContext(test5_var));

    Object* test6 = unit.root->makeChild(CLASS_TYPE, "test6");
    Object* test6_decorator = test6->makeChild(APPLICATION_TYPE, "@");
    Object* test6_var = test6_decorator->makeChild(VAR_TYPE, "y");
    REQUIRE(v.checkContext(test6_var));

    Object* test7 = unit.root->makeChild(CLASS_TYPE, "test7");
    test7->addAttribute("x");
    Object* test7_child = test7->makeChild(CLASS_TYPE, "a");
    Object* test7_decorator = test7_child->makeChild(APPLICATION_TYPE, "@");
    Object* test7_var = test7_decorator->makeChild(VAR_TYPE, "x");
    REQUIRE(v.checkContext(test7_var));
}

/*
[] > test1
  [] > x
  1 > x

[x] > test2
  [] > x

[x x] > test3
*/
TEST_CASE("Dublicates test") {
    TranslationUnit unit = makeUnit();
    ImportsMap map;
    Validator v(unit, map);
    v.setErrorStream(&s);

    Object* test1 = unit.root->makeChild(CLASS_TYPE, "test1");
    test1->makeChild(CLASS_TYPE, "x");
    Object* test1_child = test1->makeChild(APPLICATION_TYPE, "x");
    test1_child->makeChild(INT_TYPE, "1");
    REQUIRE(!v.checkDuplicateNames(test1));

    Object* test2 = unit.root->makeChild(CLASS_TYPE, "test2");
    test2->addAttribute("x");
    test2->makeChild(CLASS_TYPE, "x");
    REQUIRE(!v.checkDuplicateNames(test2));

    Object* test3 = unit.root->makeChild(CLASS_TYPE, "test3");
    test3->addAttribute("x");
    test3->addAttribute("x");
    REQUIRE(!v.checkDuplicateNames(test3));
}
