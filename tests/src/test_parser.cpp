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

TEST_F(ParserTest, ValidInput_Data)
{
    const char *source_code = "data Person { name: string, };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_DATA);
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");

    Property const *prop = minissd_get_properties(ast);
    ASSERT_NE(prop, nullptr);
    ASSERT_STREQ(minissd_get_property_name(prop), "name");

    Type const *type = minissd_get_property_type(prop);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "string");

    prop = minissd_get_next_property(prop);
    ASSERT_EQ(prop, nullptr);
}

TEST_F(ParserTest, ValidInput_DataNoTrailingComma)
{
    const char *source_code = "data Person { name: string };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_DATA);
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");

    Property const *prop = minissd_get_properties(ast);
    ASSERT_NE(prop, nullptr);
    ASSERT_STREQ(minissd_get_property_name(prop), "name");

    Type const *type = minissd_get_property_type(prop);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "string");

    prop = minissd_get_next_property(prop);
    ASSERT_EQ(prop, nullptr);
}

TEST_F(ParserTest, ValidInput_DataWithSpaceAfter)
{
    const char *source_code = "data Person { name: string, } ;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_DATA);
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");

    Property const *prop = minissd_get_properties(ast);
    ASSERT_NE(prop, nullptr);
    ASSERT_STREQ(minissd_get_property_name(prop), "name");

    Type const *type = minissd_get_property_type(prop);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "string");

    prop = minissd_get_next_property(prop);
    ASSERT_EQ(prop, nullptr);
}

TEST_F(ParserTest, ValidInput_DataMultipleProperties)
{
    const char *source_code = "data Person { name: string , age: int, };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_DATA);
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");

    Property const *prop = minissd_get_properties(ast);
    ASSERT_NE(prop, nullptr);
    ASSERT_STREQ(minissd_get_property_name(prop), "name");
    Type const *type = minissd_get_property_type(prop);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "string");

    prop = minissd_get_next_property(prop);
    ASSERT_NE(prop, nullptr);
    ASSERT_STREQ(minissd_get_property_name(prop), "age");
    type = minissd_get_property_type(prop);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "int");
}

TEST_F(ParserTest, ValidInput_DataMultiplePropertiesWithoutTrailingComma)
{
    const char *source_code = "data Person { name: string, age: int };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_DATA);
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");

    Property const *prop = minissd_get_properties(ast);
    ASSERT_NE(prop, nullptr);
    ASSERT_STREQ(minissd_get_property_name(prop), "name");
    Type const *type = minissd_get_property_type(prop);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "string");

    prop = minissd_get_next_property(prop);
    ASSERT_NE(prop, nullptr);
    ASSERT_STREQ(minissd_get_property_name(prop), "age");
    type = minissd_get_property_type(prop);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "int");
}

TEST_F(ParserTest, ValidInput_DataWithAttribute)
{
    const char *source_code = "data Person { #[test] name: string, };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_DATA);

    ASSERT_STREQ(minissd_get_data_name(ast), "Person");

    Property const *prop = minissd_get_properties(ast);
    ASSERT_NE(prop, nullptr);
    ASSERT_STREQ(minissd_get_property_name(prop), "name");
    Type const *type = minissd_get_property_type(prop);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "string");

    Attribute const *attr = minissd_get_property_attributes(prop);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "test");
    ASSERT_EQ(minissd_get_attribute_parameters(attr), nullptr);

    prop = minissd_get_next_property(prop);
    ASSERT_EQ(prop, nullptr);
}

TEST_F(ParserTest, InvalidInput_DataWithoutProperties)
{
    const char *source_code = "data Person { };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected property at line 1, column 17");
}

TEST_F(ParserTest, InvalidInput_DataWithoutName)
{
    const char *source_code = "data  { name: string };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected data name at line 1, column 8");
}

TEST_F(ParserTest, InvalidInput_DataWithoutSemicolon)
{
    const char *source_code = "data Person { name: string }";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected ';' after data declaration at line 1, column 29");
}

TEST_F(ParserTest, ValidInput_ServiceOneHandler)
{
    const char *source_code = "service MyService { fn some_function(); };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Handler const *handlers = minissd_get_handlers(ast);
    ASSERT_NE(handlers, nullptr);
    ASSERT_STREQ(minissd_get_handler_name(handlers), "some_function");
    ASSERT_EQ(minissd_get_handler_arguments(handlers), nullptr);

    handlers = minissd_get_next_handler(handlers);
    ASSERT_EQ(handlers, nullptr);
}

TEST_F(ParserTest, ValidInput_ServiceOneHandlerAndArguments)
{
    const char *source_code = "service MyService { fn some_function(a: int, b: string); };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Handler const *handlers = minissd_get_handlers(ast);
    ASSERT_NE(handlers, nullptr);
    ASSERT_STREQ(minissd_get_handler_name(handlers), "some_function");
    Argument const *args = minissd_get_handler_arguments(handlers);
    ASSERT_NE(args, nullptr);
    ASSERT_STREQ(minissd_get_argument_name(args), "a");
    Type const *type = minissd_get_argument_type(args);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "int");
    args = minissd_get_next_argument(args);
    ASSERT_NE(args, nullptr);
    ASSERT_STREQ(minissd_get_argument_name(args), "b");
    type = minissd_get_argument_type(args);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "string");
    args = minissd_get_next_argument(args);
    ASSERT_EQ(args, nullptr);

    handlers = minissd_get_next_handler(handlers);
    ASSERT_EQ(handlers, nullptr);
}

TEST_F(ParserTest, ValidInput_ServiceOneHandlerArgumentsAndReturnValue)
{
    const char *source_code = "service MyService { fn some_function(a: int, b: string) -> int ; };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Handler const *handlers = minissd_get_handlers(ast);
    ASSERT_NE(handlers, nullptr);
    ASSERT_STREQ(minissd_get_handler_name(handlers), "some_function");
    Argument const *args = minissd_get_handler_arguments(handlers);
    ASSERT_NE(args, nullptr);
    ASSERT_STREQ(minissd_get_argument_name(args), "a");
    Type const *type = minissd_get_argument_type(args);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "int");
    args = minissd_get_next_argument(args);
    ASSERT_NE(args, nullptr);
    ASSERT_STREQ(minissd_get_argument_name(args), "b");
    type = minissd_get_argument_type(args);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "string");
    args = minissd_get_next_argument(args);
    ASSERT_EQ(args, nullptr);

    type = minissd_get_handler_return_type(handlers);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "int");

    handlers = minissd_get_next_handler(handlers);
    ASSERT_EQ(handlers, nullptr);
}

TEST_F(ParserTest, ValidInput_ServiceOneEventWithArguments)
{
    const char *source_code = "service MyService { event some_event(a: int, b: string) ; };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Event const *events = minissd_get_events(ast);
    ASSERT_NE(events, nullptr);
    ASSERT_STREQ(minissd_get_event_name(events), "some_event");
    Argument const *args = minissd_get_event_arguments(events);
    ASSERT_NE(args, nullptr);
    ASSERT_STREQ(minissd_get_argument_name(args), "a");
    Type const *type = minissd_get_argument_type(args);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "int");
    args = minissd_get_next_argument(args);
    ASSERT_NE(args, nullptr);
    ASSERT_STREQ(minissd_get_argument_name(args), "b");
    type = minissd_get_argument_type(args);
    ASSERT_NE(type, nullptr);
    ASSERT_STREQ(minissd_get_type_name(type), "string");
    args = minissd_get_next_argument(args);
    ASSERT_EQ(args, nullptr);

    events = minissd_get_next_event(events);
    ASSERT_EQ(events, nullptr);
}

TEST_F(ParserTest, InvalidInput_ServiceOneEventWithArgumentsAndReturnType)
{
    const char *source_code = "service MyService { event some_event(a: int, b: string) -> int ; };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected ';' after service component at line 1, column 58");
}

TEST_F(ParserTest, ValidInput_ServiceOneEvent)
{
    const char *source_code = "service MyService { event some_event(); };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Event const *events = minissd_get_events(ast);
    ASSERT_NE(events, nullptr);
    ASSERT_STREQ(minissd_get_event_name(events), "some_event");
    ASSERT_EQ(minissd_get_event_arguments(events), nullptr);

    events = minissd_get_next_event(events);
    ASSERT_EQ(events, nullptr);
}

TEST_F(ParserTest, ValidInput_ServiceOneEventWithSpaceAfter)
{
    const char *source_code = "service MyService { event some_event(); } ;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Event const *events = minissd_get_events(ast);
    ASSERT_NE(events, nullptr);
    ASSERT_STREQ(minissd_get_event_name(events), "some_event");
    ASSERT_EQ(minissd_get_event_arguments(events), nullptr);

    events = minissd_get_next_event(events);
    ASSERT_EQ(events, nullptr);
}

TEST_F(ParserTest, ValidInput_ServiceOneHandlerWithSpaceAfter)
{
    const char *source_code = "service MyService { fn some_function(); } ;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Handler const *handlers = minissd_get_handlers(ast);
    ASSERT_NE(handlers, nullptr);
    ASSERT_STREQ(minissd_get_handler_name(handlers), "some_function");
    ASSERT_EQ(minissd_get_handler_arguments(handlers), nullptr);

    handlers = minissd_get_next_handler(handlers);
    ASSERT_EQ(handlers, nullptr);
}

TEST_F(ParserTest, ValidInput_ServiceOneEventOneHandler)
{
    const char *source_code = "service MyService { fn some_function(); event some_event(); };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Event const *events = minissd_get_events(ast);
    ASSERT_NE(events, nullptr);
    ASSERT_STREQ(minissd_get_event_name(events), "some_event");
    ASSERT_EQ(minissd_get_event_arguments(events), nullptr);

    ASSERT_EQ(minissd_get_next_event(events), nullptr);

    Handler const *handlers = minissd_get_handlers(ast);
    ASSERT_NE(handlers, nullptr);
    ASSERT_STREQ(minissd_get_handler_name(handlers), "some_function");
    ASSERT_EQ(minissd_get_handler_arguments(handlers), nullptr);

    ASSERT_EQ(minissd_get_next_handler(handlers), nullptr);
}

TEST_F(ParserTest, ValidInput_ServiceOneHandlerOneDependency)
{
    const char *source_code = "service MyService { fn some_function(); depends on some::other::service; };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Handler const *handlers = minissd_get_handlers(ast);
    ASSERT_NE(handlers, nullptr);
    ASSERT_STREQ(minissd_get_handler_name(handlers), "some_function");
    ASSERT_EQ(minissd_get_handler_arguments(handlers), nullptr);

    ASSERT_EQ(minissd_get_next_handler(handlers), nullptr);

    Dependency const *dependencies = minissd_get_dependencies(ast);
    ASSERT_NE(dependencies, nullptr);
    ASSERT_STREQ(minissd_get_dependency_path(dependencies), "some::other::service");

    ASSERT_EQ(minissd_get_next_dependency(dependencies), nullptr);
}

TEST_F(ParserTest, ValidInput_ServiceOneEventOneDependency)
{
    const char *source_code = "service MyService { event some_event(); depends on some::other::service; };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Event const *events = minissd_get_events(ast);
    ASSERT_NE(events, nullptr);
    ASSERT_STREQ(minissd_get_event_name(events), "some_event");
    ASSERT_EQ(minissd_get_event_arguments(events), nullptr);

    ASSERT_EQ(minissd_get_next_event(events), nullptr);

    Dependency const *dependencies = minissd_get_dependencies(ast);
    ASSERT_NE(dependencies, nullptr);
    ASSERT_STREQ(minissd_get_dependency_path(dependencies), "some::other::service");

    ASSERT_EQ(minissd_get_next_dependency(dependencies), nullptr);
}

TEST_F(ParserTest, ValidInput_ServiceMultipleDependenciesEventsHandlers)
{
    const char *source_code = "service MyService { depends on a::b::c ; depends on d::e::f ; fn a(); fn b(); event c(); event d(); };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_SERVICE);
    ASSERT_STREQ(minissd_get_service_name(ast), "MyService");

    Handler const *handlers = minissd_get_handlers(ast);
    ASSERT_NE(handlers, nullptr);
    ASSERT_STREQ(minissd_get_handler_name(handlers), "a");
    ASSERT_EQ(minissd_get_handler_arguments(handlers), nullptr);

    handlers = minissd_get_next_handler(handlers);
    ASSERT_NE(handlers, nullptr);
    ASSERT_STREQ(minissd_get_handler_name(handlers), "b");
    ASSERT_EQ(minissd_get_handler_arguments(handlers), nullptr);

    ASSERT_EQ(minissd_get_next_handler(handlers), nullptr);

    Event const *events = minissd_get_events(ast);
    ASSERT_NE(events, nullptr);
    ASSERT_STREQ(minissd_get_event_name(events), "c");
    ASSERT_EQ(minissd_get_event_arguments(events), nullptr);

    events = minissd_get_next_event(events);
    ASSERT_NE(events, nullptr);
    ASSERT_STREQ(minissd_get_event_name(events), "d");
    ASSERT_EQ(minissd_get_event_arguments(events), nullptr);

    ASSERT_EQ(minissd_get_next_event(events), nullptr);

    Dependency const *dependencies = minissd_get_dependencies(ast);
    ASSERT_NE(dependencies, nullptr);
    ASSERT_STREQ(minissd_get_dependency_path(dependencies), "a::b::c");

    dependencies = minissd_get_next_dependency(dependencies);
    ASSERT_NE(dependencies, nullptr);
    ASSERT_STREQ(minissd_get_dependency_path(dependencies), "d::e::f");

    ASSERT_EQ(minissd_get_next_dependency(dependencies), nullptr);
}

TEST_F(ParserTest, InvalidInput_ServiceNoHandlerAndNoEvent)
{
    const char *source_code = "service MyService {};";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Service must have at least one handler or event at line 1, column 22");
}

TEST_F(ParserTest, InvalidInput_ServiceWithoutName)
{
    const char *source_code = "service  { fn some_function(); };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected service name at line 1, column 11");
}

TEST_F(ParserTest, InvalidInput_ServiceWithoutSemicolon)
{
    const char *source_code = "service MyService { fn some_function(); }";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected ';' after service declaration at line 1, column 42");
}

TEST_F(ParserTest, ValidInput_Import)
{
    const char *source_code = "import my::module;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_IMPORT);
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module");
}

TEST_F(ParserTest, ValidInput_ImportWithSpaceAfter)
{
    const char *source_code = "import my::module ;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_IMPORT);
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module");
}

TEST_F(ParserTest, ValidInput_ImportWithAttribute)
{
    const char *source_code = "#[test] import my::module;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_IMPORT);
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module");
    Attribute const *attr = minissd_get_attributes(ast);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "test");
    ASSERT_EQ(minissd_get_attribute_parameters(attr), nullptr);
    ASSERT_EQ(minissd_get_next_attribute(attr), nullptr);
}

TEST_F(ParserTest, ValidInput_ImportWithAttributes1)
{
    const char *source_code = "#[test, blah] import my::module;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_IMPORT);
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module");
    Attribute const *attr = minissd_get_attributes(ast);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "test");
    ASSERT_EQ(minissd_get_attribute_parameters(attr), nullptr);
    attr = minissd_get_next_attribute(attr);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "blah");
    ASSERT_EQ(minissd_get_attribute_parameters(attr), nullptr);
    ASSERT_EQ(minissd_get_next_attribute(attr), nullptr);
}

TEST_F(ParserTest, ValidInput_ImportWithAttributes2)
{
    const char *source_code = "#[test] #[blah] import my::module;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_IMPORT);
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module");
    Attribute const *attr = minissd_get_attributes(ast);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "test");
    ASSERT_EQ(minissd_get_attribute_parameters(attr), nullptr);
    attr = minissd_get_next_attribute(attr);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "blah");
    ASSERT_EQ(minissd_get_attribute_parameters(attr), nullptr);
    ASSERT_EQ(minissd_get_next_attribute(attr), nullptr);
}

TEST_F(ParserTest, ValidInput_ImportWithAttributeParameter)
{
    const char *source_code = "#[test(a)] import my::module;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_IMPORT);
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module");
    Attribute const *attr = minissd_get_attributes(ast);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "test");
    AttributeParameter const *param = minissd_get_attribute_parameters(attr);
    ASSERT_NE(param, nullptr);
    ASSERT_STREQ(minissd_get_attribute_parameter_name(param), "a");
    ASSERT_EQ(minissd_get_attribute_parameter_value(param), nullptr);
    ASSERT_EQ(minissd_get_next_attribute(attr), nullptr);
}

TEST_F(ParserTest, ValidInput_ImportWithAttributeParameters)
{
    const char *source_code = "#[test(a, b)] import my::module;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_IMPORT);
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module");
    Attribute const *attr = minissd_get_attributes(ast);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "test");
    AttributeParameter const *param = minissd_get_attribute_parameters(attr);
    ASSERT_NE(param, nullptr);
    ASSERT_STREQ(minissd_get_attribute_parameter_name(param), "a");
    ASSERT_EQ(minissd_get_attribute_parameter_value(param), nullptr);
    ASSERT_EQ(minissd_get_next_attribute(attr), nullptr);
    param = minissd_get_next_attribute_parameter(param);
    ASSERT_NE(param, nullptr);
    ASSERT_STREQ(minissd_get_attribute_parameter_name(param), "b");
    ASSERT_EQ(minissd_get_attribute_parameter_value(param), nullptr);
    ASSERT_EQ(minissd_get_next_attribute(attr), nullptr);
}

TEST_F(ParserTest, ValidInput_ImportWithAttributeParameterAndValue)
{
    const char *source_code = "#[test(a = \"asd\")] import my::module;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_IMPORT);
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module");
    Attribute const *attr = minissd_get_attributes(ast);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "test");
    AttributeParameter const *param = minissd_get_attribute_parameters(attr);
    ASSERT_NE(param, nullptr);
    ASSERT_STREQ(minissd_get_attribute_parameter_name(param), "a");
    ASSERT_STREQ(minissd_get_attribute_parameter_value(param), "asd");
    param = minissd_get_next_attribute_parameter(param);
    ASSERT_EQ(param, nullptr);
    ASSERT_EQ(minissd_get_next_attribute(attr), nullptr);
}

TEST_F(ParserTest, ValidInput_ImportWithAttributeMultipleParametersAndValue)
{
    const char *source_code = "#[test(a = \"asd\", b = \"dsa\")] import my::module;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_IMPORT);
    ASSERT_STREQ(minissd_get_import_path(ast), "my::module");
    Attribute const *attr = minissd_get_attributes(ast);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "test");
    AttributeParameter const *param = minissd_get_attribute_parameters(attr);
    ASSERT_NE(param, nullptr);
    ASSERT_STREQ(minissd_get_attribute_parameter_name(param), "a");
    ASSERT_STREQ(minissd_get_attribute_parameter_value(param), "asd");
    param = minissd_get_next_attribute_parameter(param);
    ASSERT_NE(param, nullptr);
    ASSERT_STREQ(minissd_get_attribute_parameter_name(param), "b");
    ASSERT_STREQ(minissd_get_attribute_parameter_value(param), "dsa");
    param = minissd_get_next_attribute_parameter(param);
    ASSERT_EQ(param, nullptr);
    ASSERT_EQ(minissd_get_next_attribute(attr), nullptr);
}

TEST_F(ParserTest, InvalidInput_ImportWithoutPath)
{
    const char *source_code = "import ;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected import path at line 1, column 9");
}

TEST_F(ParserTest, InvalidInput_ImportWithoutSemicolon)
{
    const char *source_code = "import my::module";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected ';' after import declaration at line 1, column 18");
}

TEST_F(ParserTest, InvalidInput_ImportWithSpaceInPath)
{
    const char *source_code = "import my:: module;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected ';' after import declaration at line 1, column 14");
}

TEST_F(ParserTest, ValidInput_Enum)
{
    const char *source_code = "enum Color { Red, };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_ENUM);
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color");

    EnumVariant const *variant = minissd_get_enum_variants(ast);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Red");
    ASSERT_EQ(minissd_get_enum_variant_value(variant, nullptr), 0);

    variant = minissd_get_next_enum_variant(variant);
    ASSERT_EQ(variant, nullptr);
}

TEST_F(ParserTest, ValidInput_EnumNoTrailingComma)
{
    const char *source_code = "enum Color { Red };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_ENUM);
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color");

    EnumVariant const *variant = minissd_get_enum_variants(ast);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Red");
    ASSERT_EQ(minissd_get_enum_variant_value(variant, nullptr), 0);

    variant = minissd_get_next_enum_variant(variant);
    ASSERT_EQ(variant, nullptr);
}

TEST_F(ParserTest, ValidInput_EnumWithValue)
{
    const char *source_code = "enum Color { Red = 1, };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_ENUM);
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color");

    EnumVariant const *variant = minissd_get_enum_variants(ast);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Red");
    ASSERT_EQ(minissd_get_enum_variant_value(variant, nullptr), 1);

    variant = minissd_get_next_enum_variant(variant);
    ASSERT_EQ(variant, nullptr);
}

TEST_F(ParserTest, ValidInput_EnumWithValueNoTrailingComma)
{
    const char *source_code = "enum Color { Red = 1 };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_ENUM);
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color");

    EnumVariant const *variant = minissd_get_enum_variants(ast);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Red");
    ASSERT_EQ(minissd_get_enum_variant_value(variant, nullptr), 1);

    variant = minissd_get_next_enum_variant(variant);
    ASSERT_EQ(variant, nullptr);
}

TEST_F(ParserTest, ValidInput_EnumWithValues)
{
    const char *source_code = "enum Color { Red = 1, Green = 2, };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_ENUM);
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color");

    EnumVariant const *variant = minissd_get_enum_variants(ast);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Red");
    ASSERT_EQ(minissd_get_enum_variant_value(variant, nullptr), 1);

    variant = minissd_get_next_enum_variant(variant);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Green");
    ASSERT_EQ(minissd_get_enum_variant_value(variant, nullptr), 2);

    variant = minissd_get_next_enum_variant(variant);
    ASSERT_EQ(variant, nullptr);
}

TEST_F(ParserTest, ValidInput_EnumWithValuesNoTrailingComma)
{
    const char *source_code = "enum Color { Red = 1, Green = 2 };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_ENUM);
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color");

    EnumVariant const *variant = minissd_get_enum_variants(ast);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Red");
    ASSERT_EQ(minissd_get_enum_variant_value(variant, nullptr), 1);

    variant = minissd_get_next_enum_variant(variant);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Green");
    ASSERT_EQ(minissd_get_enum_variant_value(variant, nullptr), 2);

    variant = minissd_get_next_enum_variant(variant);
    ASSERT_EQ(variant, nullptr);
}

TEST_F(ParserTest, ValidInput_EnumWithSpaceAfter)
{
    const char *source_code = "enum Color { Red } ;";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_ENUM);
    ASSERT_STREQ(minissd_get_enum_name(ast), "Color");

    EnumVariant const *variant = minissd_get_enum_variants(ast);
    ASSERT_NE(variant, nullptr);
    ASSERT_STREQ(minissd_get_enum_variant_name(variant), "Red");
    ASSERT_EQ(minissd_get_enum_variant_value(variant, nullptr), 0);

    variant = minissd_get_next_enum_variant(variant);
    ASSERT_EQ(variant, nullptr);
}

TEST_F(ParserTest, InvalidInput_NoEnumVariants)
{
    const char *source_code = "enum Color {};";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Enum must have at least one variant at line 1, column 15");
}

TEST_F(ParserTest, InvalidInput_NoEnumName)
{
    const char *source_code = "enum  { Red };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected enum name at line 1, column 8");
}

TEST_F(ParserTest, InvalidInput_EnumWithoutSemicolon)
{
    const char *source_code = "enum Color { Red }";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected ';' after enum declaration at line 1, column 19");
}

TEST_F(ParserTest, InvalidInput_MissingType)
{
    const char *source_code = "data Person { name, age: int };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected ':' after property name at line 1, column 20");
}

TEST_F(ParserTest, InvalidInput_MissingBraces)
{
    const char *source_code = "data Person name: string, age: int";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected '{' after data name at line 1, column 14");
}

TEST_F(ParserTest, EmptyInput)
{
    const char *source_code = "";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected at least one node at line 1, column 1");
}

TEST_F(ParserTest, InvalidCharacter)
{
    const char *source_code = "data Person { name: string, age: int }; @";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_EQ(ast, nullptr);
    ASSERT_STREQ(parser->error, "Error: Expected identifier at line 1, column 42");
}

TEST_F(ParserTest, ValidInput_WithAttributes)
{
    const char *source_code = "data Person { #[attr1(name=\"value1\")] name: string, age: int };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_DATA);
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");

    Property const *props = minissd_get_properties(ast);
    ASSERT_NE(props, nullptr);

    Attribute const *attr = minissd_get_property_attributes(props);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr1");

    AttributeParameter const *param = minissd_get_attribute_parameters(attr);
    ASSERT_NE(param, nullptr);
    ASSERT_STREQ(param->key, "name");
    ASSERT_STREQ(param->opt_value, "value1");
}

TEST_F(ParserTest, ValidInput_MultipleAttributes)
{
    const char *source_code = "data Person { #[attr1] #[attr2(name=\"value1\")] name: string, age: int };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_DATA);
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");

    Property const *props = minissd_get_properties(ast);
    ASSERT_NE(props, nullptr);

    Attribute const *attr = minissd_get_property_attributes(props);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr1");

    attr = minissd_get_next_attribute(attr);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr2");

    AttributeParameter const *param = minissd_get_attribute_parameters(attr);
    ASSERT_NE(param, nullptr);
    ASSERT_STREQ(param->key, "name");
    ASSERT_STREQ(param->opt_value, "value1");
}

TEST_F(ParserTest, ValidInput_MultipleAttributes2)
{
    const char *source_code = "data Person { #[attr1, attr2(name=\"value1\")] name: string, age: int };";

    parser = minissd_create_parser(source_code);
    ast = minissd_parse(parser);

    ASSERT_NE(ast, nullptr);
    NodeType const *node_type = minissd_get_node_type(ast);
    ASSERT_NE(node_type, nullptr);
    ASSERT_EQ(*node_type, NODE_DATA);
    ASSERT_STREQ(minissd_get_data_name(ast), "Person");

    Property const *props = minissd_get_properties(ast);
    ASSERT_NE(props, nullptr);

    Attribute const *attr = minissd_get_property_attributes(props);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr1");

    attr = minissd_get_next_attribute(attr);
    ASSERT_NE(attr, nullptr);
    ASSERT_STREQ(minissd_get_attribute_name(attr), "attr2");

    AttributeParameter const *param = minissd_get_attribute_parameters(attr);
    ASSERT_NE(param, nullptr);
    ASSERT_STREQ(param->key, "name");
    ASSERT_STREQ(param->opt_value, "value1");
}