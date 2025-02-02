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
    typedef struct AttributeArgument
    {
        char *key;
        char *opt_value; // Nullable
        struct AttributeArgument *next;
    } AttributeArgument;

    typedef struct Attribute
    {
        char *name;
        AttributeArgument *opt_ll_arguments;
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

    typedef struct HandlerArgument
    {
        Attribute *attributes;
        char *name;
        char *type;
        struct HandlerArgument *next;
    } HandlerArgument;

    typedef struct Handler
    {
        Attribute *opt_ll_attributes;
        char *name;
        HandlerArgument *opt_ll_arguments;
        char *opt_return_type;
        struct Handler *next;
    } Handler;

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
        Handler *ll_handlers;
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
        int input_length;
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
    NodeType minissd_get_node_type(const AstNode *node);
    const char *minissd_get_import_path(const AstNode *node);
    const char *minissd_get_data_name(const AstNode *node);
    const char *minissd_get_enum_name(const AstNode *node);
    const char *minissd_get_handler_name(const AstNode *node);

    // Attribute accessors
    Attribute *minissd_get_attributes(const AstNode *node);
    const char *minissd_get_attribute_name(const Attribute *attr);
    AttributeArgument *minissd_get_attribute_arguments(const Attribute *attr);

    // Property and EnumVariant accessors
    Property *minissd_get_properties(const AstNode *node);
    const char *minissd_get_property_name(const Property *prop);
    Attribute *minissd_get_property_attributes(const Property *prop);
    const char *minissd_get_property_type(const Property *prop);

    EnumVariant *minissd_get_enum_variants(const AstNode *node);
    const char *minissd_get_enum_variant_name(const EnumVariant *value);
    Attribute *minissd_get_enum_variant_attributes(const EnumVariant *value);
    int minissd_get_enum_variant(const EnumVariant *value, bool *has_value);

    const char *minissd_get_service_name(const AstNode *node);
    HandlerArgument *minissd_get_handler_arguments(const Handler *node);
    const char *minissd_get_argument_name(const HandlerArgument *prop);
    Attribute *minissd_get_argument_attributes(const HandlerArgument *prop);
    const char *minissd_get_argument_type(const HandlerArgument *prop);

    Dependency *minissd_get_dependencies(const AstNode *node);
    Handler *minissd_get_handlers(const AstNode *node);

    Dependency *minissd_get_next_dependency(const Dependency *dep);
    Handler *minissd_get_next_handler(const Handler *handler);
    const char *minissd_get_dependency_path(const Dependency *dep);

    // Traversal functions
    AstNode *minissd_get_next_node(const AstNode *node);
    Property *minissd_get_next_property(const Property *prop);
    EnumVariant *minissd_get_next_enum_value(const EnumVariant *value);
    Attribute *minissd_get_next_attribute(const Attribute *attr);
    AttributeArgument *minissd_get_next_attribute_argument(const AttributeArgument *arg);
    HandlerArgument *minissd_get_next_handler_argument(const HandlerArgument *arg);

#ifdef __cplusplus
}
#endif

#endif // MINISSD_H