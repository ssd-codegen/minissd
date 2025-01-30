#include <gtest/gtest.h>

#include "minissd.h"

namespace helper
{

    struct Parsed
    {
        Parser *parser;
        AstNode *ast;

        Parsed(Parser *parser, AstNode *ast) : parser(parser), ast(ast) {}

        ~Parsed()
        {
            minissd_free_ast(ast);
            minissd_free_parser(parser);
        }
    };

    Parsed parse(const char *source_code)
    {
        Parser *parser = minissd_create_parser(source_code);
        AstNode *ast = minissd_parse(parser);
        return {parser, ast};
    }
}

// Test for successful parsing of valid input
TEST(ParserTest, ValidInput_Data)
{
    const char *source_code = "data Person { name: string, age: int };";

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_NE(ast, nullptr);                            // Ensure AST is not NULL
    ASSERT_EQ(minissd_get_node_type(ast), NODE_DATA);   // Check if node type is 'data'
    ASSERT_STREQ(minissd_get_data_name(ast), "Person"); // Check data node name

    // Check properties
    Property *prop = minissd_get_properties(ast);
    ASSERT_NE(prop, nullptr); // Ensure there are properties
    ASSERT_STREQ(minissd_get_property_name(prop), "name");
    ASSERT_STREQ(minissd_get_property_type(prop), "string");

    prop = minissd_get_next_property(prop);
    ASSERT_NE(prop, nullptr);
    ASSERT_STREQ(minissd_get_property_name(prop), "age");
    ASSERT_STREQ(minissd_get_property_type(prop), "int");
}

// Test for successful parsing of enum nodes
TEST(ParserTest, ValidInput_Enum)
{
    const char *source_code = "enum Color { Red = 1, Green, Blue };";

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_NE(ast, nullptr);                           // Ensure AST is not NULL
    ASSERT_EQ(minissd_get_node_type(ast), NODE_ENUM);  // Check if node type is 'enum'
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color"); // Check enum node name

    // Check enum values
    EnumValue *value = minissd_get_enum_values(ast);
    ASSERT_NE(value, nullptr); // Ensure there are enum values
    ASSERT_STREQ(minissd_get_enum_value_name(value), "Red");
    ASSERT_EQ(minissd_get_enum_value(value, nullptr), 1); // Check value for 'Red'

    value = minissd_get_next_enum_value(value);
    ASSERT_NE(value, nullptr);
    ASSERT_STREQ(minissd_get_enum_value_name(value), "Green");
    ASSERT_EQ(minissd_get_enum_value(value, nullptr), 0); // Default value for 'Green'

    value = minissd_get_next_enum_value(value);
    ASSERT_NE(value, nullptr);
    ASSERT_STREQ(minissd_get_enum_value_name(value), "Blue");
    ASSERT_EQ(minissd_get_enum_value(value, nullptr), 0); // Default value for 'Blue'
}

// Test for successful parsing of import nodes
TEST(ParserTest, ValidInput_Import)
{
    const char *source_code = "import my::module;";

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_NE(ast, nullptr);                                  // Ensure AST is not NULL
    ASSERT_EQ(minissd_get_node_type(ast), NODE_IMPORT);       // Check if node type is 'import'
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module"); // Check path
}

// Test for valid input: No enum values
TEST(ParserTest, ValidInput_MissingEnumValues)
{
    const char *source_code = "enum Color { Red, Green };";

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(minissd_get_node_type(ast), NODE_ENUM);
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color");

    EnumValue *value = minissd_get_enum_values(ast);
    ASSERT_NE(value, nullptr);
    ASSERT_STREQ(minissd_get_enum_value_name(value), "Red");
    ASSERT_EQ(minissd_get_enum_value(value, nullptr), 0); // Default value for 'Red'

    value = minissd_get_next_enum_value(value);
    ASSERT_NE(value, nullptr);
    ASSERT_STREQ(minissd_get_enum_value_name(value), "Green");
    ASSERT_EQ(minissd_get_enum_value(value, nullptr), 0); // Default value for 'Green'
}

// Test for invalid input: Missing type in data node
TEST(ParserTest, InvalidInput_MissingType)
{
    const char *source_code = "data Person { name, age: int };"; // Missing type for 'name'

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_EQ(ast, nullptr); // Parsing should fail
    ASSERT_STREQ(parser->error, "Error: Expected ':' after property name at line 1, column 20");
}

// Test for invalid input: Missing braces in data node
TEST(ParserTest, InvalidInput_MissingBraces)
{
    const char *source_code = "data Person name: string, age: int"; // Missing closing brace

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_EQ(ast, nullptr); // Parsing should fail
    ASSERT_STREQ(parser->error, "Error: Expected '{' after data name at line 1, column 14");
}

// Test for invalid input: No enum values
TEST(ParserTest, InvalidInput_NoEnumValues)
{
    const char *source_code = "enum Color {};"; // Missing closing brace

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_EQ(ast, nullptr); // Parsing should fail
    ASSERT_STREQ(parser->error, "Error: Enum must have at least one value at line 1, column 15");
}

// Test for empty input (edge case)
TEST(ParserTest, EmptyInput)
{
    const char *source_code = "";

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_EQ(ast, nullptr); // No AST should be generated
    ASSERT_STREQ(parser->error, "Error: Expected at least one node at line 1, column 1");
}

// Test for edge case: Invalid character
TEST(ParserTest, InvalidCharacter)
{
    const char *source_code = "data Person { name: string, age: int }; @"; // Invalid character '@'

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_EQ(ast, nullptr); // Parsing should fail due to invalid character
    ASSERT_STREQ(parser->error, "Error: Expected identifier at line 1, column 42");
}

// Test for correctly parsing attributes in data node
TEST(ParserTest, ValidInput_WithAttributes)
{
    const char *source_code = "data Person { #[attr1(name=\"value1\")] name: string, age: int };";

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_NE(ast, nullptr);                            // Ensure AST is not NULL
    ASSERT_EQ(minissd_get_node_type(ast), NODE_DATA);   // Check if node type is 'data'
    ASSERT_STREQ(minissd_get_data_name(ast), "Person"); // Check data node name

    Property *props = minissd_get_properties(ast);
    ASSERT_NE(props, nullptr); // Ensure there are properties

    // Check for attributes
    Attribute *attr = minissd_get_property_attributes(props);
    ASSERT_NE(attr, nullptr); // Ensure there are attributes
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr1");

    Argument *arg = minissd_get_attribute_arguments(attr);
    ASSERT_NE(arg, nullptr); // Ensure there are arguments
    ASSERT_STREQ(arg->key, "name");
    ASSERT_STREQ(arg->value, "value1");
}

// Test for multiple attributes in the same node
TEST(ParserTest, ValidInput_MultipleAttributes)
{
    const char *source_code = "data Person { #[attr1] #[attr2(name=\"value1\")] name: string, age: int };";

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_NE(ast, nullptr);                            // Ensure AST is not NULL
    ASSERT_EQ(minissd_get_node_type(ast), NODE_DATA);   // Check if node type is 'data'
    ASSERT_STREQ(minissd_get_data_name(ast), "Person"); // Check data node name

    Property *props = minissd_get_properties(ast);
    ASSERT_NE(props, nullptr); // Ensure there are properties

    // Check for first attribute
    Attribute *attr = minissd_get_property_attributes(props);
    ASSERT_NE(attr, nullptr); // Ensure there are attributes
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr1");

    // Check for second attribute
    attr = minissd_get_next_attribute(attr);
    ASSERT_NE(attr, nullptr); // Ensure there is another attribute
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr2");

    Argument *arg = minissd_get_attribute_arguments(attr);
    ASSERT_NE(arg, nullptr); // Ensure there are arguments
    ASSERT_STREQ(arg->key, "name");
    ASSERT_STREQ(arg->value, "value1");
}

TEST(ParserTest, ValidInput_MultipleAttributes2)
{
    const char *source_code = "data Person { #[attr1, attr2(name=\"value1\")] name: string, age: int };";

    auto parsed = helper::parse(source_code);
    auto parser = parsed.parser;
    auto ast = parsed.ast;

    ASSERT_NE(ast, nullptr);                            // Ensure AST is not NULL
    ASSERT_EQ(minissd_get_node_type(ast), NODE_DATA);   // Check if node type is 'data'
    ASSERT_STREQ(minissd_get_data_name(ast), "Person"); // Check data node name

    Property *props = minissd_get_properties(ast);
    ASSERT_NE(props, nullptr); // Ensure there are properties

    // Check for first attribute
    Attribute *attr = minissd_get_property_attributes(props);
    ASSERT_NE(attr, nullptr); // Ensure there are attributes
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr1");

    // Check for second attribute
    attr = minissd_get_next_attribute(attr);
    ASSERT_NE(attr, nullptr); // Ensure there is another attribute
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr2");

    Argument *arg = minissd_get_attribute_arguments(attr);
    ASSERT_NE(arg, nullptr); // Ensure there are arguments
    ASSERT_STREQ(arg->key, "name");
    ASSERT_STREQ(arg->value, "value1");
}
