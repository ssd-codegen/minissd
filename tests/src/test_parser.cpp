#include <gtest/gtest.h>

#include "minissd.h"

class ParserTest : public ::testing::Test
{
protected:
    Parser *parser;
    AstNode *ast;

    void TearDown() override
    {
        minissd_free_ast(ast);
        minissd_free_parser(parser);
    }
};

// Test for successful parsing of valid input
TEST_F(ParserTest, ValidInput_Data)
{
    const char *source_code = "data Person { name: string, age: int };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);                                // Ensure AST is not NULL
    NodeType const *node_type = minissd_get_node_type(ast); // Get node type
    ASSERT_NE(node_type, nullptr);                          // Ensure node type is not NULL
    ASSERT_EQ(*node_type, NODE_DATA);                       // Check if node type is 'data'
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");     // Check data node name

    // Check properties
    Property const *prop = minissd_get_properties(ast);
    ASSERT_NE(prop, nullptr); // Ensure there are properties
    ASSERT_STREQ(minissd_get_property_name(prop), "name");
    ASSERT_STREQ(minissd_get_property_type(prop), "string");

    prop = minissd_get_next_property(prop);
    ASSERT_NE(prop, nullptr);
    ASSERT_STREQ(minissd_get_property_name(prop), "age");
    ASSERT_STREQ(minissd_get_property_type(prop), "int");
}

// Test for successful parsing of enum nodes
TEST_F(ParserTest, ValidInput_Enum)
{
    const char *source_code = "enum Color { Red = 1, Green, Blue };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);                                // Ensure AST is not NULL
    NodeType const *node_type = minissd_get_node_type(ast); // Get node type
    ASSERT_NE(node_type, nullptr);                          // Ensure node type is not NULL
    ASSERT_EQ(*node_type, NODE_ENUM);                       // Check if node type is 'enum'
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color");      // Check enum node name

    // Check enum variants
    EnumVariant const *variant = minissd_get_enum_variants(ast);
    ASSERT_NE(variant, nullptr); // Ensure there are enum variants
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Red");
    ASSERT_EQ(minissd_get_enum_variant(variant, nullptr), 1); // Check variant for 'Red'

    variant = minissd_get_next_enum_value(variant);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Green");
    ASSERT_EQ(minissd_get_enum_variant(variant, nullptr), 0); // Default variant for 'Green'

    variant = minissd_get_next_enum_value(variant);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Blue");
    ASSERT_EQ(minissd_get_enum_variant(variant, nullptr), 0); // Default variant for 'Blue'
}

// Test for successful parsing of import nodes
TEST_F(ParserTest, ValidInput_Import)
{
    const char *source_code = "import my::module;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);                                  // Ensure AST is not NULL
    NodeType const *node_type = minissd_get_node_type(ast);   // Get node type
    ASSERT_NE(node_type, nullptr);                            // Ensure node type is not NULL
    ASSERT_EQ(*node_type, NODE_IMPORT);                       // Check if node type is 'import'
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module"); // Check path
}

// Test for valid input: No enum values
TEST_F(ParserTest, ValidInput_MissingEnumValues)
{
    const char *source_code = "enum Color { Red, Green };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast); // Get node type
    ASSERT_NE(node_type, nullptr);                          // Ensure node type is not NULL
    ASSERT_EQ(*node_type, NODE_ENUM);                       // Check if node type is 'enum'
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color");

    EnumVariant const *variant = minissd_get_enum_variants(ast);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Red");
    ASSERT_EQ(minissd_get_enum_variant(variant, nullptr), 0); // Default variant for 'Red'

    variant = minissd_get_next_enum_value(variant);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Green");
    ASSERT_EQ(minissd_get_enum_variant(variant, nullptr), 0); // Default variant for 'Green'
}

// Test for invalid input: Missing type in data node
TEST_F(ParserTest, InvalidInput_MissingType)
{
    const char *source_code = "data Person { name, age: int };"; // Missing type for 'name'

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr); // Parsing should fail
    ASSERT_STREQ(parser->error, "Error: Expected ':' after property name at line 1, column 20");
}

// Test for invalid input: Missing braces in data node
TEST_F(ParserTest, InvalidInput_MissingBraces)
{
    const char *source_code = "data Person name: string, age: int"; // Missing closing brace

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr); // Parsing should fail
    ASSERT_STREQ(parser->error, "Error: Expected '{' after data name at line 1, column 14");
}

// Test for invalid input: No enum variants
TEST_F(ParserTest, InvalidInput_NoEnumVariants)
{
    const char *source_code = "enum Color {};"; // Missing closing brace

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr); // Parsing should fail
    ASSERT_STREQ(parser->error, "Error: Enum must have at least one variant at line 1, column 15");
}

// Test for empty input (edge case)
TEST_F(ParserTest, EmptyInput)
{
    const char *source_code = "";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr); // No AST should be generated
    ASSERT_STREQ(parser->error, "Error: Expected at least one node at line 1, column 1");
}

// Test for edge case: Invalid character
TEST_F(ParserTest, InvalidCharacter)
{
    const char *source_code = "data Person { name: string, age: int }; @"; // Invalid character '@'

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr); // Parsing should fail due to invalid character
    ASSERT_STREQ(parser->error, "Error: Expected identifier at line 1, column 42");
}

// Test for correctly parsing attributes in data node
TEST_F(ParserTest, ValidInput_WithAttributes)
{
    const char *source_code = "data Person { #[attr1(name=\"value1\")] name: string, age: int };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);                                // Ensure AST is not NULL
    NodeType const *node_type = minissd_get_node_type(ast); // Get node type
    ASSERT_NE(node_type, nullptr);                          // Ensure node type is not NULL
    ASSERT_EQ(*node_type, NODE_DATA);                       // Check if node type is 'data'
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");     // Check data node name

    Property const *props = minissd_get_properties(ast);
    ASSERT_NE(props, nullptr); // Ensure there are properties

    // Check for attributes
    Attribute const *attr = minissd_get_property_attributes(props);
    ASSERT_NE(attr, nullptr); // Ensure there are attributes
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr1");

    AttributeParameter const *param = minissd_get_attribute_parameters(attr);
    ASSERT_NE(param, nullptr); // Ensure there are arguments
    ASSERT_STREQ(param->key, "name");
    ASSERT_STREQ(param->opt_value, "value1");
}

// Test for multiple attributes in the same node
TEST_F(ParserTest, ValidInput_MultipleAttributes)
{
    const char *source_code = "data Person { #[attr1] #[attr2(name=\"value1\")] name: string, age: int };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);                                // Ensure AST is not NULL
    NodeType const *node_type = minissd_get_node_type(ast); // Get node type
    ASSERT_NE(node_type, nullptr);                          // Ensure node type is not NULL
    ASSERT_EQ(*node_type, NODE_DATA);                       // Check if node type is 'data'
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");     // Check data node name

    Property const *props = minissd_get_properties(ast);
    ASSERT_NE(props, nullptr); // Ensure there are properties

    // Check for first attribute
    Attribute const *attr = minissd_get_property_attributes(props);
    ASSERT_NE(attr, nullptr); // Ensure there are attributes
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr1");

    // Check for second attribute
    attr = minissd_get_next_attribute(attr);
    ASSERT_NE(attr, nullptr); // Ensure there is another attribute
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr2");

    AttributeParameter const *param = minissd_get_attribute_parameters(attr);
    ASSERT_NE(param, nullptr); // Ensure there are arguments
    ASSERT_STREQ(param->key, "name");
    ASSERT_STREQ(param->opt_value, "value1");
}

TEST_F(ParserTest, ValidInput_MultipleAttributes2)
{
    const char *source_code = "data Person { #[attr1, attr2(name=\"value1\")] name: string, age: int };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);                                // Ensure AST is not NULL
    NodeType const *node_type = minissd_get_node_type(ast); // Get node type
    ASSERT_NE(node_type, nullptr);                          // Ensure node type is not NULL
    ASSERT_EQ(*node_type, NODE_DATA);                       // Check if node type is 'data'
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");     // Check data node name

    Property const *props = minissd_get_properties(ast);
    ASSERT_NE(props, nullptr); // Ensure there are properties

    // Check for first attribute
    Attribute const *attr = minissd_get_property_attributes(props);
    ASSERT_NE(attr, nullptr); // Ensure there are attributes
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr1");

    // Check for second attribute
    attr = minissd_get_next_attribute(attr);
    ASSERT_NE(attr, nullptr); // Ensure there is another attribute
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr2");

    AttributeParameter const *param = minissd_get_attribute_parameters(attr);
    ASSERT_NE(param, nullptr); // Ensure there are arguments
    ASSERT_STREQ(param->key, "name");
    ASSERT_STREQ(param->opt_value, "value1");
}
