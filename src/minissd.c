#include "minissd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#ifdef _WIN32
#define strdup _strdup
#endif

#define CHECKED_ASSIGN(obj, prop, func) \
    obj->prop = func(p);                \
    if (obj->prop == NULL)              \
    {                                   \
        if (obj)                        \
        {                               \
            free(obj);                  \
            obj = NULL;                 \
        }                               \
        return NULL;                    \
    }

#define CHECKED_ASSIGN_WITH(obj, prop, func, destructor) \
    obj->prop = func(p);                                 \
    if (obj->prop == NULL)                               \
    {                                                    \
        if (obj)                                         \
        {                                                \
            destructor(obj);                             \
            obj = NULL;                                  \
        }                                                \
        return NULL;                                     \
    }

#define CHECKED_ASSIGN_EXT(obj, prop, func, sec) \
    obj->prop = func(p);                         \
    if (obj->prop == NULL)                       \
    {                                            \
        if (sec)                                 \
        {                                        \
            free(sec);                           \
            sec = NULL;                          \
        }                                        \
        if (obj)                                 \
        {                                        \
            free(obj);                           \
            obj = NULL;                          \
        }                                        \
        return NULL;                             \
    }

#define CHECKED_ASSIGN_WITH_EXT(obj, prop, func, destructor, sec) \
    obj->prop = func(p);                                          \
    if (obj->prop == NULL)                                        \
    {                                                             \
        if (sec)                                                  \
        {                                                         \
            free(sec);                                            \
            sec = NULL;                                           \
        }                                                         \
        if (obj)                                                  \
        {                                                         \
            destructor(obj);                                      \
            obj = NULL;                                           \
        }                                                         \
        return NULL;                                              \
    }

#define CHECKED_DECLARE(type, obj, func) \
    type obj = func(p);                  \
    if (obj == NULL)                     \
    {                                    \
        return NULL;                     \
    }

// Free functions
void free_arguments(Argument *args)
{
    Argument *current = args;
    while (current)
    {
        if (current->key)
        {
            free(current->key);
            current->key = NULL;
        }
        if (current->value)
        {
            free(current->value);
            current->value = NULL;
        }
        Argument *next = current->next;
        current->next = NULL;
        free(current);
        current = next;
    };
}

void free_attributes(Attribute *attrs)
{
    Attribute *current_attr = attrs;
    while (current_attr)
    {
        if (current_attr->name)
        {
            free(current_attr->name);
            current_attr->name = NULL;
        }
        if (current_attr->arguments)
        {
            free_arguments(current_attr->arguments);
            current_attr->arguments = NULL;
        }
        Attribute *next_attr = current_attr->next;
        current_attr->next = NULL;
        free(current_attr);
        current_attr = next_attr;
    };
}

void free_properties(Property *prop)
{
    Property *current = prop;
    while (current)
    {
        if (current->name)
        {
            free(current->name);
            current->name = NULL;
        }
        if (current->type)
        {
            free(current->type);
            current->type = NULL;
        }
        if (current->attributes)
        {
            free_attributes(current->attributes);
            current->attributes = NULL;
        }
        Property *outer_next = current->next;
        current->next = NULL;
        free(current);
        current = outer_next;
    };
}

void free_enum_values(EnumValue *values)
{
    EnumValue *current = values;
    while (current)
    {
        if (current->name)
        {
            free(current->name);
            current->name = NULL;
        }
        if (current->value)
        {
            free(current->value);
            current->value = NULL;
        }
        if (current->attributes)
        {
            free_attributes(current->attributes);
            current->attributes = NULL;
        }
        EnumValue *next = current->next;
        current->next = NULL;
        free(current);
        current = next;
    };
}

void free_ast(AstNode *ast)
{
    AstNode *current = ast;
    while (current)
    {
        if (current->attributes)
        {
            free_attributes(current->attributes);
            current->attributes = NULL;
        }
        switch (current->type)
        {
        case NODE_IMPORT:
            if (current->node.importNode.path)
            {
                free(current->node.importNode.path);
                current->node.importNode.path = NULL;
            }
            break;
        case NODE_DATA:
            if (current->node.dataNode.name)
            {
                free(current->node.dataNode.name);
                current->node.dataNode.name = NULL;
            }
            if (current->node.dataNode.properties)
            {
                free_properties(current->node.dataNode.properties);
                current->node.dataNode.properties = NULL;
            }
            break;
        case NODE_ENUM:
            if (current->node.enumNode.name)
            {
                free(current->node.enumNode.name);
                current->node.enumNode.name = NULL;
            }
            if (current->node.enumNode.values)
            {
                free_enum_values(current->node.enumNode.values);
                current->node.enumNode.values = NULL;
            }
            break;
        default:
            break;
        }
        AstNode *next = current->next;
        current->next = NULL;
        free(current);
        current = next;
    };
}

void error(Parser *p, const char *message)
{
    snprintf(p->error, MAX_ERROR_SIZE, "Error: %s at line %d, column %d", message, p->line, p->column);
}

void advance(Parser *p)
{
    if (p->input[p->index] == '\0')
    {
        p->current = '\0';
        return;
    }
    p->current = p->input[p->index++];
    if (p->current == '\n')
    {
        p->line++;
        p->column = 1;
    }
    else
    {
        p->column++;
    }
}

void eat_whitespace(Parser *p)
{
    while (isspace(p->current))
    {
        advance(p);
    }
}

int is_alphanumeric(char c)
{
    return isalnum(c) || c == '_';
}

char *parse_path(Parser *p)
{
    char buffer[MAX_TOKEN_SIZE];
    int length = 0;
    eat_whitespace(p);
    while (p->current != '\0' && (is_alphanumeric(p->current) || p->current == '_' || p->current == ':'))
    {
        buffer[length++] = p->current;
        advance(p);
    }
    buffer[length] = '\0';
    if (length == 0)
    {
        error(p, "Expected import path");
        return NULL;
    }
    return strdup(buffer);
}

int *parse_int(Parser *p)
{
    char buffer[MAX_TOKEN_SIZE];
    int length = 0;
    while (isdigit(p->current))
    {
        buffer[length++] = p->current;
        advance(p);
    }
    buffer[length] = '\0';
    if (length == 0)
    {
        error(p, "Expected integer");
        return NULL;
    }
    int *value = (int *)malloc(sizeof(int));
    assert(value);
    *value = atoi(buffer);
    return value;
}

char *parse_string(Parser *p)
{
    if (p->current != '"')
    {
        error(p, "Expected string");
        return NULL;
    }
    advance(p);
    char buffer[MAX_TOKEN_SIZE];
    int length = 0;
    while (p->current != '"' && p->current != '\0')
    {
        buffer[length++] = p->current;
        advance(p);
    }
    if (p->current != '"')
    {
        error(p, "Unterminated string");
        return NULL;
    }
    advance(p);
    buffer[length] = '\0';
    return strdup(buffer);
}

char *parse_ident(Parser *p)
{
    char buffer[MAX_TOKEN_SIZE];
    int length = 0;
    while (is_alphanumeric(p->current))
    {
        buffer[length++] = p->current;
        advance(p);
    }
    buffer[length] = '\0';
    if (length == 0)
    {
        error(p, "Expected identifier");
        return NULL;
    }
    return strdup(buffer);
}

Attribute *parse_attributes(Parser *p)
{
    Attribute *head = NULL, *tail = NULL;
    eat_whitespace(p);
    while (p->current == '#')
    {
        advance(p);
        eat_whitespace(p);
        if (p->current != '[')
        {
            error(p, "Expected '[' after attribute");
            return NULL;
        }
        advance(p);
        eat_whitespace(p);
        while (p->current != ']')
        {
            Attribute *attr = (Attribute *)malloc(sizeof(Attribute));
            assert(attr);
            attr->name = NULL;
            attr->arguments = NULL;
            attr->next = NULL;

            eat_whitespace(p);
            CHECKED_ASSIGN(attr, name, parse_ident);

            eat_whitespace(p);
            if (p->current == '(')
            {
                advance(p);
                Argument *arg_head = NULL, *arg_tail = NULL;

                eat_whitespace(p);
                while (p->current != ')')
                {
                    Argument *arg = (Argument *)malloc(sizeof(Argument));
                    assert(arg);
                    arg->key = NULL;
                    arg->next = NULL;
                    arg->value = NULL;

                    eat_whitespace(p);
                    CHECKED_ASSIGN(arg, key, parse_ident);

                    eat_whitespace(p);
                    if (p->current == '=')
                    {
                        advance(p);
                        eat_whitespace(p);
                        CHECKED_ASSIGN(arg, value, parse_string);
                    }

                    if (!arg_head)
                    {
                        arg_head = arg;
                    }
                    else
                    {
                        arg_tail->next = arg;
                    }
                    arg_tail = arg;

                    eat_whitespace(p);
                    if (p->current != ',')
                    {
                        break;
                    }
                    advance(p);
                }

                eat_whitespace(p);
                if (p->current != ')')
                {
                    error(p, "Expected ')' after attribute argument");
                    free_arguments(arg_head);
                    free_attributes(attr);
                    free_attributes(head);

                    return NULL;
                }
                advance(p);
                attr->arguments = arg_head;
            }

            if (!head)
            {
                head = attr;
            }
            else
            {
                tail->next = attr;
            }
            tail = attr;

            eat_whitespace(p);
            if (p->current != ',')
            {
                break;
            }
            advance(p);
        }

        eat_whitespace(p);
        if (p->current != ']')
        {
            free_attributes(head);
            error(p, "Expected ',' after attribute");
            return NULL;
        }
        advance(p);
        eat_whitespace(p);
    }
    return head;
}

EnumValue *parse_enum_values(Parser *p)
{
    if (p->current != '{')
    {
        error(p, "Expected '{' after enum name");
        return NULL;
    }
    advance(p);

    EnumValue *head = NULL, *tail = NULL;

    eat_whitespace(p);
    while (p->current != '}')
    {

        EnumValue *prop = (EnumValue *)malloc(sizeof(EnumValue));
        assert(prop);
        prop->next = NULL;
        prop->value = NULL;
        prop->attributes = NULL;
        prop->name = NULL;

        eat_whitespace(p);
        prop->attributes = parse_attributes(p);

        eat_whitespace(p);
        CHECKED_ASSIGN(prop, name, parse_ident);

        eat_whitespace(p);
        if (p->current == '=')
        {
            advance(p);

            eat_whitespace(p);
            CHECKED_ASSIGN(prop, value, parse_int);
        }

        if (!head)
        {
            head = prop;
        }
        else
        {
            tail->next = prop;
        }
        tail = prop;

        eat_whitespace(p);
        if (p->current != ',')
        {
            break;
        }
        advance(p);
        eat_whitespace(p);
    }

    eat_whitespace(p);
    if (p->current != '}')
    {
        error(p, "Expected ',' after enum value");
        free_enum_values(head);
        return NULL;
    }
    advance(p);
    if (!head)
    {
        error(p, "Enum must have at least one value");
        return NULL;
    }
    return head;
}

Property *parse_properties(Parser *p)
{
    if (p->current != '{')
    {
        error(p, "Expected '{' after data name");
        return NULL;
    }
    advance(p);

    Property *head = NULL, *tail = NULL;

    eat_whitespace(p);
    while (p->current != '}')
    {

        Property *prop = (Property *)malloc(sizeof(Property));
        assert(prop);
        prop->next = NULL;
        prop->attributes = NULL;
        prop->name = NULL;
        prop->type = NULL;

        eat_whitespace(p);
        prop->attributes = parse_attributes(p);

        eat_whitespace(p);
        CHECKED_ASSIGN(prop, name, parse_ident);

        eat_whitespace(p);
        if (p->current != ':')
        {
            error(p, "Expected ':' after property name");
            free_properties(prop);
            free_properties(head);
            return NULL;
        }
        advance(p);

        eat_whitespace(p);
        CHECKED_ASSIGN(prop, type, parse_ident);

        if (!head)
        {
            head = prop;
        }
        else
        {
            tail->next = prop;
        }
        tail = prop;

        eat_whitespace(p);
        if (p->current != ',')
        {
            break;
        }
        advance(p);
        eat_whitespace(p);
    }

    eat_whitespace(p);
    if (p->current != '}')
    {
        error(p, "Expected ',' after property");
        free_properties(head);
        return NULL;
    }
    advance(p);
    return head;
}

AstNode *parse_node(Parser *p)
{
    eat_whitespace(p);
    Attribute *attributes = parse_attributes(p);

    eat_whitespace(p);
    CHECKED_DECLARE(char *, ident, parse_ident);
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    assert(node);
    node->next = NULL;
    node->attributes = NULL;

    eat_whitespace(p);
    node->attributes = attributes;
    if (strcmp(ident, "import") == 0)
    {
        node->type = NODE_IMPORT;
        node->node.importNode.path = parse_path(p);
        if (node->node.importNode.path == NULL)
        {
            if (ident)
            {
                free(ident);
                ident = NULL;
            }
            if (node)
            {
                free_ast(node);
                node = NULL;
            }
            return NULL;
        };
    }
    else if (strcmp(ident, "data") == 0)
    {
        node->type = NODE_DATA;
        node->node.dataNode.name = parse_ident(p);
        if (node->node.dataNode.name == NULL)
        {
            if (ident)
            {
                free(ident);
                ident = NULL;
            }
            if (node)
            {
                free_ast(node);
                node = NULL;
            }
            return NULL;
        };
        eat_whitespace(p);
        node->node.dataNode.properties = parse_properties(p);
        if (node->node.dataNode.properties == NULL)
        {
            if (ident)
            {
                free(ident);
                ident = NULL;
            }
            if (node)
            {
                free_ast(node);
                node = NULL;
            }
            return NULL;
        };
    }
    else if (strcmp(ident, "enum") == 0)
    {
        node->type = NODE_ENUM;
        node->node.enumNode.name = parse_ident(p);
        if (node->node.enumNode.name == NULL)
        {
            if (ident)
            {
                free(ident);
                ident = NULL;
            }
            if (node)
            {
                free_ast(node);
                node = NULL;
            }
            return NULL;
        };
        eat_whitespace(p);
        node->node.enumNode.values = parse_enum_values(p);
        if (node->node.enumNode.values == NULL)
        {
            if (ident)
            {
                free(ident);
                ident = NULL;
            }
            if (node)
            {
                free_ast(node);
                node = NULL;
            }
            return NULL;
        };
    }
    else
    {
        error(p, "Unknown node type");
        free_ast(node);
        node = NULL;
    }
    free(ident);
    if (node)
    {
        eat_whitespace(p);
        if (p->current != ';')
        {
            switch (node->type)
            {
            case NODE_IMPORT:
                error(p, "Expected ';' after import declaration");
                break;
            case NODE_DATA:
                error(p, "Expected ';' after data declaration");
                break;
            case NODE_ENUM:
                error(p, "Expected ';' after enum declaration");
                break;
            }
            free_ast(node);
            return NULL;
        }
        advance(p);
    }
    return node;
}

AstNode *parse(Parser *p)
{
    advance(p);
    AstNode *ast = NULL, *last = NULL;
    while (p->current != '\0')
    {
        AstNode *node = parse_node(p);
        if (!node)
        {
            free_ast(ast);
            return NULL;
        }
        if (!ast)
        {
            ast = node;
        }
        else
        {
            last->next = node;
        }
        last = node;
    }
    if (!ast)
    {
        error(p, "Expected at least one node");
    }
    return ast;
}

Parser *create_parser(const char *input)
{
    assert(input);
    Parser *p = (Parser *)malloc(sizeof(Parser));
    assert(p);
    p->input = input;
    memset(p->error, 0, MAX_ERROR_SIZE);
    p->current = 0;
    p->index = 0;
    p->line = 1;
    p->column = 1;
    return p;
}

void print_ast(AstNode *ast)
{
    while (ast)
    {
        switch (ast->type)
        {
        case NODE_IMPORT:
            printf("Import: %s\n", ast->node.importNode.path);
            break;
        case NODE_DATA:
            printf("Data: %s\n", ast->node.dataNode.name);
            break;
        case NODE_ENUM:
            printf("Enum: %s\n", ast->node.enumNode.name);
            break;
        }
        ast = ast->next;
    }
}

// Parser functions
Parser *minissd_create_parser(const char *input)
{
    return create_parser(input);
}

void minissd_free_parser(Parser *p)
{
    free(p);
}

// Parsing
AstNode *minissd_parse(Parser *p)
{
    return parse(p);
}

void minissd_free_ast(AstNode *ast)
{
    free_ast(ast);
}

// AST Node accessors
NodeType minissd_get_node_type(const AstNode *node)
{
    return node ? node->type : -1;
}

const char *minissd_get_import_path(const AstNode *node)
{
    return (node && node->type == NODE_IMPORT) ? node->node.importNode.path : NULL;
}

const char *minissd_get_data_name(const AstNode *node)
{
    return (node && node->type == NODE_DATA) ? node->node.dataNode.name : NULL;
}

const char *minissd_get_enum_name(const AstNode *node)
{
    return (node && node->type == NODE_ENUM) ? node->node.enumNode.name : NULL;
}

// Attribute accessors
Attribute *minissd_get_attributes(const AstNode *node)
{
    return node ? node->attributes : NULL;
}

const char *minissd_get_attribute_name(const Attribute *attr)
{
    return attr ? attr->name : NULL;
}

Argument *minissd_get_attribute_arguments(const Attribute *attr)
{
    return attr ? attr->arguments : NULL;
}

// Property accessors
Property *minissd_get_properties(const AstNode *node)
{
    return (node && node->type == NODE_DATA) ? node->node.dataNode.properties : NULL;
}

const char *minissd_get_property_name(const Property *prop)
{
    return prop ? prop->name : NULL;
}

Attribute *minissd_get_property_attributes(const Property *prop)
{
    return prop ? prop->attributes : NULL;
}

const char *minissd_get_property_type(const Property *prop)
{
    return prop ? prop->type : NULL;
}

// Enum Value accessors
EnumValue *minissd_get_enum_values(const AstNode *node)
{
    return (node && node->type == NODE_ENUM) ? node->node.enumNode.values : NULL;
}

const char *minissd_get_enum_value_name(const EnumValue *value)
{
    return value ? value->name : NULL;
}

Attribute *minissd_get_enum_value_attributes(const EnumValue *value)
{
    return value ? value->attributes : NULL;
}

int minissd_get_enum_value(const EnumValue *value, bool *has_value)
{
    if (!value || !value->value)
    {
        if (has_value)
            *has_value = false;
        return 0;
    }
    if (has_value)
        *has_value = true;
    return *(value->value);
}

// Traversal functions
AstNode *minissd_get_next_node(const AstNode *node)
{
    return node ? node->next : NULL;
}

Property *minissd_get_next_property(const Property *prop)
{
    return prop ? prop->next : NULL;
}

EnumValue *minissd_get_next_enum_value(const EnumValue *value)
{
    return value ? value->next : NULL;
}

Attribute *minissd_get_next_attribute(const Attribute *attr)
{
    return attr ? attr->next : NULL;
}

Argument *minissd_get_next_argument(const Argument *arg)
{
    return arg ? arg->next : NULL;
}
