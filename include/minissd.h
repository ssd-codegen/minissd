#ifndef MINISSD_H
#define MINISSD_H

#include <stdbool.h>
#include <stddef.h>

#ifndef MAX_ERROR_SIZE
#define MAX_ERROR_SIZE 512
#endif

#ifndef MAX_TOKEN_SIZE
#define MAX_TOKEN_SIZE 512
#endif

// DLL Export/Import Macros
#ifdef _WIN32
#ifdef MINISSD_SHARED
#ifdef MINISSD_EXPORTS
#define MINISSD_API __declspec(dllexport)
#else
#define MINISSD_API __declspec(dllimport)
#endif
#else
#define MINISSD_API
#endif
#else
#define MINISSD_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct AttributeParameter
    {
        char*                      key;
        char*                      opt_value;  // Nullable
        struct AttributeParameter* next;
    } AttributeParameter;

    typedef struct Attribute
    {
        char*               name;
        AttributeParameter* opt_ll_arguments;
        struct Attribute*   next;
    } Attribute;

    typedef struct Type
    {
        char* name;
        bool  is_list;
        int*  count;  // Nullable
    } Type;

    typedef struct Property
    {
        Attribute*       attributes;
        char*            name;
        Type*            type;
        struct Property* next;
    } Property;

    typedef struct EnumVariant
    {
        Attribute*          attributes;
        char*               name;
        int*                opt_value;  // Nullable
        struct EnumVariant* next;
    } EnumVariant;

    typedef struct Argument
    {
        Attribute*       attributes;
        char*            name;
        Type*            type;
        struct Argument* next;
    } Argument;

    typedef struct Handler
    {
        Attribute*      opt_ll_attributes;
        char*           name;
        Argument*       opt_ll_arguments;
        Type*           opt_return_type;
        struct Handler* next;
    } Handler;

    typedef struct Event
    {
        Attribute*    opt_ll_attributes;
        char*         name;
        Argument*     opt_ll_arguments;
        struct Event* next;
    } Event;

    typedef struct Dependency
    {
        Attribute*         opt_ll_attributes;
        char*              path;
        struct Dependency* next;
    } Dependency;

    typedef struct Import
    {
        char* path;
    } Import;

    typedef struct Data
    {
        char*     name;
        Property* ll_properties;
    } Data;

    typedef struct Enum
    {
        char*        name;
        EnumVariant* ll_variants;
    } Enum;

    typedef struct Service
    {
        char*       name;
        Dependency* opt_ll_dependencies;
        Handler*    opt_ll_handlers;
        Event*      opt_ll_events;
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
        NodeType   type;
        Attribute* opt_ll_attributes;
        union
        {
            Import  import_node;
            Data    data_node;
            Enum    enum_node;
            Service service_node;
        } node;
        struct AstNode* next;
    } AstNode;

    typedef struct
    {
        const char* input;
        size_t      input_length;
        char        error[MAX_ERROR_SIZE];
        char        current;
        size_t      index;
        int         line;
        int         column;
    } Parser;

    // Parser creation and destruction
    MINISSD_API Parser*
    minissd_create_parser(const char* input);
    void
    minissd_free_parser(Parser* p);

    // Parsing function
    MINISSD_API AstNode*
    minissd_parse(Parser* p);
    void
    minissd_free_ast(AstNode* ast);

    // AST Node Accessors
    MINISSD_API NodeType const*
    minissd_get_node_type(AstNode const* node);

    MINISSD_API char const*
    minissd_get_import_path(AstNode const* node);

    MINISSD_API char const*
    minissd_get_data_name(AstNode const* node);

    MINISSD_API char const*
    minissd_get_enum_name(AstNode const* node);

    MINISSD_API char const*
    minissd_get_service_name(AstNode const* node);

    MINISSD_API Property const*
    minissd_get_properties(AstNode const* node);

    MINISSD_API EnumVariant const*
    minissd_get_enum_variants(AstNode const* node);

    MINISSD_API Dependency const*
    minissd_get_dependencies(AstNode const* node);

    MINISSD_API Handler const*
    minissd_get_handlers(AstNode const* node);

    MINISSD_API Event const*
    minissd_get_events(AstNode const* node);

    MINISSD_API Attribute const*
    minissd_get_attributes(AstNode const* node);

    MINISSD_API AstNode const*
    minissd_get_next_node(AstNode const* node);

    // Handler Accessors
    MINISSD_API Attribute const*
    minissd_get_handler_attributes(Handler const* node);

    MINISSD_API char const*
    minissd_get_handler_name(Handler const* node);

    MINISSD_API Type const*
    minissd_get_handler_return_type(Handler const* handler);

    MINISSD_API Argument const*
    minissd_get_handler_arguments(Handler const* node);

    MINISSD_API Handler const*
    minissd_get_next_handler(Handler const* handler);

    // Event Accessors
    MINISSD_API char const*
    minissd_get_event_name(Event const* event);

    MINISSD_API Argument const*
    minissd_get_event_arguments(Event const* event);

    MINISSD_API Event const*
    minissd_get_next_event(Event const* event);

    // Dependency Accessors
    MINISSD_API char const*
    minissd_get_dependency_path(Dependency const* dep);

    MINISSD_API Dependency const*
    minissd_get_next_dependency(Dependency const* dep);

    // Property Accessors
    MINISSD_API char const*
    minissd_get_property_name(Property const* prop);

    MINISSD_API Type const*
    minissd_get_property_type(Property const* prop);

    MINISSD_API Attribute const*
    minissd_get_property_attributes(Property const* prop);

    MINISSD_API Property const*
    minissd_get_next_property(Property const* prop);

    // Type Accessors
    MINISSD_API char const*
    minissd_get_type_name(Type const* type);

    MINISSD_API bool
    minissd_get_type_is_list(Type const* type);

    MINISSD_API int const*
    minissd_get_type_count(Type const* type);

    // Enum Variant Accessors
    MINISSD_API char const*
    minissd_get_enum_variant_name(EnumVariant const* value);

    MINISSD_API int
    minissd_get_enum_variant_value(EnumVariant const* value, bool* has_value);

    MINISSD_API Attribute const*
    minissd_get_enum_variant_attributes(EnumVariant const* value);

    MINISSD_API EnumVariant const*
    minissd_get_next_enum_variant(EnumVariant const* value);

    // Argument Accessors
    MINISSD_API char const*
    minissd_get_argument_name(Argument const* arg);

    MINISSD_API Type const*
    minissd_get_argument_type(Argument const* arg);

    MINISSD_API Attribute const*
    minissd_get_argument_attributes(Argument const* arg);

    MINISSD_API Argument const*
    minissd_get_next_argument(Argument const* arg);

    // Attribute Accessors
    MINISSD_API char const*
    minissd_get_attribute_name(Attribute const* attr);

    MINISSD_API AttributeParameter const*
    minissd_get_attribute_parameters(Attribute const* attr);

    MINISSD_API Attribute const*
    minissd_get_next_attribute(Attribute const* attr);

    // Attribute Parameter Accessors
    MINISSD_API char const*
    minissd_get_attribute_parameter_name(AttributeParameter const* arg);

    MINISSD_API char const*
    minissd_get_attribute_parameter_value(AttributeParameter const* arg);

    MINISSD_API AttributeParameter const*
    minissd_get_next_attribute_parameter(AttributeParameter const* arg);

#ifdef __cplusplus
}
#endif

#endif  // MINISSD_H
