#include "../../include/catch.hpp"
#include "helper.h"
#include "parser.h"
#include "optimizer.h"
#include "translation_unit.h"
#include "imports_map.h"

TEST_CASE("Reverse dot notation test") {
    TranslationUnit unit1 = makeUnit();
    Parser p1(unit1, "[] > test\n  ((dx.pow 2).plus (dy.pow 2)).sqrt\n");
    REQUIRE(p1.isProgram());

    TranslationUnit unit2 = makeUnit();
    Parser p2(unit2, "[] > test\n  sqrt.\n    plus.\n      dx.pow 2\n      dy.pow 2\n");
    REQUIRE(p2.isProgram());
    REQUIRE(!unit1.root->equals(unit2.root, false));

    IdTagTable table;
    ImportsMap map;
    Optimizer opt(unit2, table, map);
    REQUIRE(opt.run());
    REQUIRE(unit1.root->equals(unit2.root, false));
}

TEST_CASE("Data test") {
    TranslationUnit unit = makeUnit();
    Parser p(unit, "[] > test\n  1 > a\n  1.0 > b\n  \"Hello\" > c\n  TRUE > d\n  /reg/g > e\n  'c' > f\n  12-BC- > g\n");
    REQUIRE(p.isProgram());

    Object* root = new Object(nullptr, CLASS_TYPE);
    root->setOriginValue("root");
    Object* test = root->makeChild(CLASS_TYPE, "test");
    Object* test_a = test->makeChild(APPLICATION_TYPE, "a");
    Object* test_a_val = test_a->makeChild(INT_TYPE, "1");
    Object* test_b = test->makeChild(APPLICATION_TYPE, "b");
    Object* test_b_val = test_b->makeChild(FLOAT_TYPE, "1.0");
    Object* test_c = test->makeChild(APPLICATION_TYPE, "c");
    Object* test_c_val = test_c->makeChild(STRING_TYPE, "\"Hello\"");
    Object* test_d = test->makeChild(APPLICATION_TYPE, "d");
    Object* test_d_val = test_d->makeChild(BOOL_TYPE, "true");
    Object* test_e = test->makeChild(APPLICATION_TYPE, "e");
    Object* test_e_val = test_e->makeChild(REGEX_TYPE, "/reg/g");
    Object* test_f = test->makeChild(APPLICATION_TYPE, "f");
    Object* test_f_val = test_f->makeChild(CHAR_TYPE, "'c'");
    Object* test_g = test->makeChild(APPLICATION_TYPE, "g");
    Object* test_g_val = test_g->makeChild(BYTES_TYPE);
    Object* test_g_val1 = test_g_val->makeChild(BYTE_TYPE, "12");
    Object* test_g_val2 = test_g_val->makeChild(BYTE_TYPE, "bc");
    REQUIRE(unit.root->equals(root, true));
}

TEST_CASE("Flags test") {
    TranslationUnit unit = makeUnit();
    Parser p(unit, "[] > test\n  1 > a!\n  ...x > b\n  [] > c /int\n  x' > d\n");
    REQUIRE(p.isProgram());

    Object* root = new Object(nullptr, CLASS_TYPE);
    root->setOriginValue("root");
    Object* test = root->makeChild(CLASS_TYPE, "test");
    Object* test_a = test->makeChild(APPLICATION_TYPE, "a");
    Object* test_a_val = test_a->makeChild(INT_TYPE, "1");
    test_a_val->addFlags(CONSTANT_FLAG);
    Object* test_b = test->makeChild(APPLICATION_TYPE, "b");
    Object* test_b_val = test_b->makeChild(VAR_TYPE, "x");
    test_b_val->addFlags(VARARGS_FLAG);
    Object* test_c = test->makeChild(CLASS_TYPE, "c");
    test_c->addFlags(ATOM_FLAG);
    Object* test_d = test->makeChild(APPLICATION_TYPE, "d");
    Object* test_d_val = test_d->makeChild(VAR_TYPE, "x");
    test_d_val->addFlags(CLONE_FLAG);
    REQUIRE(unit.root->equals(root, true));
}