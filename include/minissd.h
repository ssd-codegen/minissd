#ifndef MINISSD_H
#define MINISSD_H

#include <stdbool.h>

#ifndef MAX_ERROR_SIZE
#define MAX_ERROR_SIZE 512
#endif

#ifndef MAX_TOKEN_SIZE
#define MAX_TOKEN_SIZE 512
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct AttributeParameter
    {
        char *key;
        char *opt_value; // Nullable
        struct AttributeParameter *next;
    } AttributeParameter;

    typedef struct Attribute
    {
        char *name;
        AttributeParameter *opt_ll_arguments;
        struct Attribute *next;
    } Attribute;

    typedef struct Property
    {
        Attribute *attributes;
        char *name;
        char *type;
        struct Property *next;
    } Property;

    typedef struct EnumVariant
    {
        Attribute *attributes;
        char *name;
        int *opt_value; // Nullable
        struct EnumVariant *next;
    } EnumVariant;

    typedef struct Argument
    {
        Attribute *attributes;
        char *name;
        char *type;
        struct Argument *next;
    } Argument;

    typedef struct Handler
    {
        Attribute *opt_ll_attributes;
        char *name;
        Argument *opt_ll_arguments;
        char *opt_return_type;
        struct Handler *next;
    } Handler;

    typedef struct Event
    {
        Attribute *opt_ll_attributes;
        char *name;
        Argument *opt_ll_arguments;
        struct Event *next;
    } Event;

    typedef struct Dependency
    {
        Attribute *opt_ll_attributes;
        char *path;
        struct Dependency *next;
    } Dependency;

    typedef struct Import
    {
        char *path;
    } Import;

    typedef struct Data
    {
        char *name;
        Property *ll_properties;
    } Data;

    typedef struct Enum
    {
        char *name;
        EnumVariant *ll_variants;
    } Enum;

    typedef struct Service
    {
        char *name;
        Dependency *opt_ll_dependencies;
        Handler *opt_ll_handlers;
        Event *opt_ll_events;
    } Service;

    typedef enum
    {
        NODE_IMPORT,
        NODE_DATA,
        NODE_ENUM,
        NODE_SERVICE
    } NodeType;

    typedef struct AstNode
    {
        NodeType type;
        Attribute *opt_ll_attributes;
        union
        {
            Import import_node;
            Data data_node;
            Enum enum_node;
            Service service_node;
        } node;
        struct AstNode *next;
    } AstNode;

    typedef struct
    {
        const char *input;
        size_t input_length;
        char error[MAX_ERROR_SIZE];
        char current;
        int index;
        int line;
        int column;
    } Parser;

    // Parser creation and destruction
    Parser *minissd_create_parser(const char *input);
    void minissd_free_parser(Parser *p);

    // Parsing function
    AstNode *minissd_parse(Parser *p);
    void minissd_free_ast(AstNode *ast);

    // AST Node accessors
    NodeType const *
    minissd_get_node_type(AstNode const *node);

    char const *
    minissd_get_import_path(AstNode const *node);

    char const *
    minissd_get_data_name(AstNode const *node);

    char const *
    minissd_get_enum_name(AstNode const *node);

    char const *
    minissd_get_handler_name(Handler const *node);

    char const *
    minissd_get_handler_return_type(Handler const *handler);

    char const *
    minissd_get_event_name(Event const *event);

    // Attribute accessors
    Attribute const *
    minissd_get_attributes(AstNode const *node);

    char const *
    minissd_get_attribute_name(Attribute const *attr);

    AttributeParameter const *
    minissd_get_attribute_parameters(Attribute const *attr);

    // Property and EnumVariant accessors
    Property const *
    minissd_get_properties(AstNode const *node);

    char const *
    minissd_get_property_name(Property const *prop);

    Attribute const *
    minissd_get_property_attributes(Property const *prop);

    char const *
    minissd_get_property_type(Property const *prop);

    EnumVariant const *
    minissd_get_enum_variants(AstNode const *node);

    char const *
    minissd_get_enum_variant_name(EnumVariant const *value);

    Attribute const *
    minissd_get_enum_variant_attributes(EnumVariant const *value);

    int minissd_get_enum_variant(EnumVariant const *value, bool *has_value);

    char const *
    minissd_get_service_name(AstNode const *node);

    Argument const *
    minissd_get_handler_arguments(Handler const *node);

    Argument const *
    minissd_get_event_arguments(Event const *event);

    char const *
    minissd_get_argument_name(Argument const *prop);

    Attribute const *
    minissd_get_argument_attributes(Argument const *prop);

    char const *
    minissd_get_argument_type(Argument const *prop);

    Dependency const *
    minissd_get_dependencies(AstNode const *node);
    Handler const *
    minissd_get_handlers(AstNode const *node);

    Event const *
    minissd_get_events(AstNode const *node);

    Dependency const *
    minissd_get_next_dependency(Dependency const *dep);

    Handler const *
    minissd_get_next_handler(Handler const *handler);

    Event const *
    minissd_get_next_event(Event const *event);

    char const *minissd_get_dependency_path(Dependency const *dep);

    // Traversal functions
    AstNode const *
    minissd_get_next_node(AstNode const *node);

    Property const *
    minissd_get_next_property(Property const *prop);

    EnumVariant const *
    minissd_get_next_enum_variant(EnumVariant const *value);

    Attribute const *
    minissd_get_next_attribute(Attribute const *attr);

    AttributeParameter const *
    minissd_get_next_attribute_parameter(AttributeParameter const *arg);

    char const *
    minissd_get_attribute_parameter_name(AttributeParameter const *arg);

    char const *
    minissd_get_attribute_parameter_value(AttributeParameter const *arg);

    Argument const *
    minissd_get_next_argument(Argument const *arg);

#ifdef __cplusplus
}
#endif

#endif // MINISSD_H