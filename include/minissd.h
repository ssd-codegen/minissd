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
    typedef struct Argument
    {
        char *key;
        char *value; // Nullable
        struct Argument *next;
    } Argument;

    typedef struct Attribute
    {
        char *name;
        Argument *arguments;
        struct Attribute *next;
    } Attribute;

    typedef struct Property
    {
        Attribute *attributes;
        char *name;
        char *type;
        struct Property *next;
    } Property;

    typedef struct EnumValue
    {
        Attribute *attributes;
        char *name;
        int *value; // Nullable
        struct EnumValue *next;
    } EnumValue;

    typedef struct Import
    {
        char *path;
    } Import;

    typedef struct Data
    {
        char *name;
        Property *properties;
    } Data;

    typedef struct Enum
    {
        char *name;
        EnumValue *values;
    } Enum;

    typedef enum
    {
        NODE_IMPORT,
        NODE_DATA,
        NODE_ENUM
    } NodeType;

    typedef struct AstNode
    {
        NodeType type;
        Attribute *attributes;
        union
        {
            Import importNode;
            Data dataNode;
            Enum enumNode;
        } node;
        struct AstNode *next;
    } AstNode;

    typedef struct
    {
        const char *input;
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

    // Attribute accessors
    Attribute *minissd_get_attributes(const AstNode *node);
    const char *minissd_get_attribute_name(const Attribute *attr);
    Argument *minissd_get_attribute_arguments(const Attribute *attr);

    // Property and EnumValue accessors
    Property *minissd_get_properties(const AstNode *node);
    const char *minissd_get_property_name(const Property *prop);
    Attribute *minissd_get_property_attributes(const Property *prop);
    const char *minissd_get_property_type(const Property *prop);

    EnumValue *minissd_get_enum_values(const AstNode *node);
    const char *minissd_get_enum_value_name(const EnumValue *value);
    Attribute *minissd_get_enum_value_attributes(const EnumValue *value);
    int minissd_get_enum_value(const EnumValue *value, bool *has_value);

    // Traversal functions
    AstNode *minissd_get_next_node(const AstNode *node);
    Property *minissd_get_next_property(const Property *prop);
    EnumValue *minissd_get_next_enum_value(const EnumValue *value);
    Attribute *minissd_get_next_attribute(const Attribute *attr);
    Argument *minissd_get_next_argument(const Argument *arg);

#ifdef __cplusplus
}
#endif

#endif // MINISSD_H