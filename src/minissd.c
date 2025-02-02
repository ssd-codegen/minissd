#include "minissd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#define CHECKED_DECLARE(type, obj, func) \
    type obj = func(p);                  \
    if (!obj)                            \
    {                                    \
        return NULL;                     \
    }

static char *
strdup_c99(const char *s)
{
    if (s == NULL)
    {
        return NULL;
    }

    size_t len = strlen(s) + 1;
    char *dup = (char *)malloc(len);

    if (dup)
    {
        memcpy(dup, s, len);
    }

    return dup;
}

// Free functions
static void
free_attribute_arguments(AttributeArgument *args)
{
    AttributeArgument *current = args;
    while (current)
    {
        if (current->key)
        {
            free(current->key);
        }
        if (current->opt_value)
        {
            free(current->opt_value);
        }
        AttributeArgument *next = current->next;
        free(current);
        current = next;
    };
}

static void
free_attributes(Attribute *attrs)
{
    Attribute *current_attr = attrs;
    while (current_attr)
    {
        if (current_attr->name)
        {
            free(current_attr->name);
        }
        free_attribute_arguments(current_attr->opt_ll_arguments);
        Attribute *next_attr = current_attr->next;
        free(current_attr);
        current_attr = next_attr;
    };
}

static void
free_handler_arguments(HandlerArgument *args)
{
    HandlerArgument *current = args;
    while (current)
    {
        if (current->name)
        {
            free(current->name);
        }
        if (current->type)
        {
            free(current->type);
        }
        free_attributes(current->attributes);
        HandlerArgument *next = current->next;
        free(current);
        current = next;
    };
}

static void
free_properties(Property *prop)
{
    Property *current = prop;
    while (current)
    {
        if (current->name)
        {
            free(current->name);
        }
        if (current->type)
        {
            free(current->type);
        }
        free_attributes(current->attributes);
        Property *outer_next = current->next;
        free(current);
        current = outer_next;
    };
}

static void
free_enum_variants(EnumVariant *variants)
{
    EnumVariant *current = variants;
    while (current)
    {
        if (current->name)
        {
            free(current->name);
        }
        if (current->opt_value)
        {
            free(current->opt_value);
        }
        free_attributes(current->attributes);
        EnumVariant *next = current->next;
        free(current);
        current = next;
    };
}

static void
free_dependencies(Dependency *deps)
{
    Dependency *current = deps;
    while (current)
    {
        if (current->path)
        {
            free(current->path);
        }
        free_attributes(current->opt_ll_attributes);
        Dependency *next = current->next;
        free(current);
        current = next;
    };
}
static void
free_handlers(Handler *handlers)
{
    Handler *current = handlers;
    while (current)
    {
        if (current->name)
        {
            free(current->name);
        }
        if (current->opt_return_type)
        {
            free(current->opt_return_type);
        }
        free_attributes(current->opt_ll_attributes);
        free_handler_arguments(current->opt_ll_arguments);
        Handler *next = current->next;
        free(current);
        current = next;
    };
}

static void
free_ast(AstNode *ast)
{
    AstNode *current = ast;
    while (current)
    {
        free_attributes(current->opt_ll_attributes);
        switch (current->type)
        {
        case NODE_IMPORT:
            if (current->node.import_node.path)
            {
                free(current->node.import_node.path);
            }
            break;
        case NODE_DATA:
            if (current->node.data_node.name)
            {
                free(current->node.data_node.name);
            }
            free_properties(current->node.data_node.ll_properties);
            break;
        case NODE_ENUM:
            if (current->node.enum_node.name)
            {
                free(current->node.enum_node.name);
            }
            free_enum_variants(current->node.enum_node.ll_variants);
            break;
        case NODE_SERVICE:
            if (current->node.service_node.name)
            {
                free(current->node.service_node.name);
            }
            free_dependencies(current->node.service_node.opt_ll_dependencies);
            free_handlers(current->node.service_node.ll_handlers);
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

static void
error(Parser *p, const char *message)
{
    snprintf(p->error, MAX_ERROR_SIZE, "Error: %s at line %d, column %d", message, p->line, p->column);
}

static char
peek(const Parser *p)
{
    if (p->index < p->input_length)
    {
        return p->input[p->index];
    }
    return '\0';
}

void debug(const Parser *p)
{
    printf("Current: %c\n", p->current);
    printf("Next: %c\n", peek(p));
    printf("Index: %d\n", p->index);
    printf("Line: %d\n", p->line);
    printf("Column: %d\n", p->column);
}

static void
advance(Parser *p)
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

static void
eat_whitespaces_and_comments(Parser *p)
{
    bool comment = true;
    while (comment)
    {
        comment = false;
        while (isspace(p->current))
        {
            advance(p);
        }
        if (p->current == '/' && peek(p) == '/')
        {
            comment = true;
            while (p->current != '\n' && p->current != '\0')
            {
                advance(p);
            }
        }
    }
}

static int
is_alphanumeric(char c)
{
    return isalnum(c) || c == '_';
}

static char *
parse_path(Parser *p)
{
    char buffer[MAX_TOKEN_SIZE + 1];
    int length = 0;
    eat_whitespaces_and_comments(p);
    while (length <= MAX_TOKEN_SIZE && p->current != '\0' && (is_alphanumeric(p->current) || p->current == ':'))
    {
        if (length == MAX_TOKEN_SIZE)
        {
            error(p, "Path length exceeds maximum token size");
            return NULL;
        }

        buffer[length++] = p->current;
        advance(p);
    }
    buffer[length] = '\0';
    if (length == 0)
    {
        error(p, "Expected import path");
        return NULL;
    }
    return strdup_c99(buffer);
}

static int *
parse_int(Parser *p)
{
    char buffer[MAX_TOKEN_SIZE + 1];
    int length = 0;
    while (length <= MAX_TOKEN_SIZE && isdigit(p->current))
    {
        if (length == MAX_TOKEN_SIZE)
        {
            error(p, "Integer length exceeds maximum token size");
            return NULL;
        }
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

static char *
parse_string(Parser *p)
{
    if (p->current != '"')
    {
        error(p, "Expected string");
        return NULL;
    }
    advance(p);
    char buffer[MAX_TOKEN_SIZE + 1];
    int length = 0;
    while (length <= MAX_TOKEN_SIZE && p->current != '"' && p->current != '\0')
    {
        if (length == MAX_TOKEN_SIZE)
        {
            error(p, "String length exceeds maximum token size");
            return NULL;
        }
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
    return strdup_c99(buffer);
}

static char *
parse_identifier(Parser *p)
{
    char buffer[MAX_TOKEN_SIZE + 1];
    int length = 0;
    while (length <= MAX_TOKEN_SIZE && is_alphanumeric(p->current))
    {
        if (length == MAX_TOKEN_SIZE)
        {
            error(p, "Identifier length exceeds maximum token size");
            return NULL;
        }
        buffer[length++] = p->current;
        advance(p);
    }
    buffer[length] = '\0';
    if (length == 0)
    {
        error(p, "Expected identifier");
        return NULL;
    }
    return strdup_c99(buffer);
}

static Attribute *
parse_attributes(Parser *p)
{
    Attribute *head = NULL, *tail = NULL;
    eat_whitespaces_and_comments(p);
    while (p->current == '#')
    {
        advance(p);
        eat_whitespaces_and_comments(p);
        if (p->current != '[')
        {
            free_attributes(head);
            error(p, "Expected '[' after attribute");
            return NULL;
        }
        advance(p);
        eat_whitespaces_and_comments(p);
        while (p->current != ']')
        {
            Attribute *attr = (Attribute *)calloc(1, sizeof(Attribute));
            assert(attr);

            eat_whitespaces_and_comments(p);
            attr->name = parse_identifier(p);
            if (!attr->name)
            {
                free_attributes(attr);
                free_attributes(head);
                return NULL;
            };

            eat_whitespaces_and_comments(p);
            if (p->current == '(')
            {
                advance(p);
                AttributeArgument *arg_head = NULL, *arg_tail = NULL;

                eat_whitespaces_and_comments(p);
                while (p->current != ')')
                {
                    AttributeArgument *arg = (AttributeArgument *)calloc(1, sizeof(AttributeArgument));
                    assert(arg);

                    eat_whitespaces_and_comments(p);
                    arg->key = parse_identifier(p);
                    if (!arg->key)
                    {
                        free_attribute_arguments(arg);
                        free_attribute_arguments(arg_head);
                        free_attributes(attr);
                        free_attributes(head);
                        return NULL;
                    };

                    eat_whitespaces_and_comments(p);
                    if (p->current == '=')
                    {
                        advance(p);
                        eat_whitespaces_and_comments(p);
                        arg->opt_value = parse_string(p);
                        if (!arg->opt_value)
                        {
                            free_attribute_arguments(arg);
                            free_attribute_arguments(arg_head);
                            free_attributes(attr);
                            free_attributes(head);
                            return NULL;
                        };
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

                    eat_whitespaces_and_comments(p);
                    if (p->current != ',')
                    {
                        break;
                    }
                    advance(p);
                }

                eat_whitespaces_and_comments(p);
                if (p->current != ')')
                {
                    error(p, "Expected ')' after attribute argument");
                    free_attribute_arguments(arg_head);
                    free_attributes(attr);
                    free_attributes(head);

                    return NULL;
                }
                advance(p);
                attr->opt_ll_arguments = arg_head;
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

            eat_whitespaces_and_comments(p);
            if (p->current != ',')
            {
                break;
            }
            advance(p);
        }

        eat_whitespaces_and_comments(p);
        if (p->current != ']')
        {
            free_attributes(head);
            error(p, "Expected ',' after attribute");
            return NULL;
        }
        advance(p);
        eat_whitespaces_and_comments(p);
    }
    return head;
}

static EnumVariant *
parse_enum_variants(Parser *p)
{
    if (p->current != '{')
    {
        error(p, "Expected '{' after enum name");
        return NULL;
    }
    advance(p);

    EnumVariant *head = NULL, *tail = NULL;

    eat_whitespaces_and_comments(p);
    while (p->current != '}')
    {

        EnumVariant *ev = (EnumVariant *)calloc(1, sizeof(EnumVariant));
        assert(ev);

        eat_whitespaces_and_comments(p);
        ev->attributes = parse_attributes(p);

        eat_whitespaces_and_comments(p);
        ev->name = parse_identifier(p);
        if (!ev->name)
        {
            free_enum_variants(ev);
            free_enum_variants(head);
            return NULL;
        };

        eat_whitespaces_and_comments(p);
        if (p->current == '=')
        {
            advance(p);

            eat_whitespaces_and_comments(p);
            ev->opt_value = parse_int(p);
            if (!ev->opt_value)
            {
                free_enum_variants(ev);
                free_enum_variants(head);
                return NULL;
            };
        }

        if (!head)
        {
            head = ev;
        }
        else
        {
            tail->next = ev;
        }
        tail = ev;

        eat_whitespaces_and_comments(p);
        if (p->current != ',')
        {
            break;
        }
        advance(p);
        eat_whitespaces_and_comments(p);
    }

    eat_whitespaces_and_comments(p);
    if (p->current != '}')
    {
        error(p, "Expected ',' after enum value");
        free_enum_variants(head);
        return NULL;
    }
    advance(p);
    if (!head)
    {
        error(p, "Enum must have at least one variant");
        return NULL;
    }
    return head;
}

static Property *
parse_properties(Parser *p)
{
    if (p->current != '{')
    {
        error(p, "Expected '{' after data name");
        return NULL;
    }
    advance(p);

    Property *head = NULL, *tail = NULL;

    eat_whitespaces_and_comments(p);
    while (p->current != '}')
    {

        Property *prop = (Property *)calloc(1, sizeof(Property));
        assert(prop);

        eat_whitespaces_and_comments(p);
        prop->attributes = parse_attributes(p);

        eat_whitespaces_and_comments(p);
        prop->name = parse_identifier(p);
        if (!prop->name)
        {
            free_properties(prop);
            free_properties(head);
            return NULL;
        };

        eat_whitespaces_and_comments(p);
        if (p->current != ':')
        {
            error(p, "Expected ':' after property name");
            free_properties(prop);
            free_properties(head);
            return NULL;
        }
        advance(p);

        eat_whitespaces_and_comments(p);
        prop->type = parse_identifier(p);
        if (!prop->type)
        {
            free_properties(prop);
            free_properties(head);
            return NULL;
        };

        if (!head)
        {
            head = prop;
        }
        else
        {
            tail->next = prop;
        }
        tail = prop;

        eat_whitespaces_and_comments(p);
        if (p->current != ',')
        {
            break;
        }
        advance(p);
        eat_whitespaces_and_comments(p);
    }

    eat_whitespaces_and_comments(p);
    if (p->current != '}')
    {
        error(p, "Expected ',' after property");
        free_properties(head);
        return NULL;
    }
    advance(p);
    return head;
}

static HandlerArgument *
parse_handler_arguments(Parser *p)
{
    HandlerArgument *head = NULL, *tail = NULL;
    while (p->current != ')')
    {
        HandlerArgument *arg = (HandlerArgument *)calloc(1, sizeof(HandlerArgument));
        assert(arg);
        eat_whitespaces_and_comments(p);
        arg->name = parse_identifier(p);
        eat_whitespaces_and_comments(p);
        if (!arg->name)
        {
            error(p, "Expected argument name");
            free_handler_arguments(arg);
            free_handler_arguments(head);
            return NULL;
        };
        if (p->current != ':')
        {
            error(p, "Expected ':' after argument name");
            free_handler_arguments(arg);
            free_handler_arguments(head);
            return NULL;
        }
        advance(p);
        eat_whitespaces_and_comments(p);
        arg->type = parse_identifier(p);
        eat_whitespaces_and_comments(p);
        if (!arg->type)
        {
            error(p, "Expected argument type");
            free_handler_arguments(arg);
            free_handler_arguments(head);
            return NULL;
        };

        if (!head)
        {
            head = arg;
        }
        else
        {
            tail->next = arg;
        }

        if (p->current != ',')
        {
            break;
        }
        advance(p);
        eat_whitespaces_and_comments(p);
    }
    return head;
}

typedef struct ServiceComponents
{
    Handler *ll_handlers;
    Dependency *opt_ll_dependencies;
} ServiceComponents;

static void
free_service_components(ServiceComponents *sc)
{
    free_handlers(sc->ll_handlers);
    free_dependencies(sc->opt_ll_dependencies);
    free(sc);
}

static ServiceComponents *
parse_service(Parser *p)
{
    if (p->current != '{')
    {
        error(p, "Expected '{' after service name");
        return NULL;
    }
    advance(p);

    ServiceComponents *sc = (ServiceComponents *)calloc(1, sizeof(ServiceComponents));
    Handler *handler_head = NULL, *handler_tail = NULL;
    Dependency *dep_head = NULL, *dep_tail = NULL;

    while (p->current != '}')
    {
        eat_whitespaces_and_comments(p);
        Attribute *attributes = parse_attributes(p);
        eat_whitespaces_and_comments(p);
        char *ident = parse_identifier(p);
        if (!ident)
        {
            error(p, "Expected 'depends' or 'handle' keyword");
            free(ident);
            free_dependencies(dep_head);
            free_handlers(handler_head);
            free_service_components(sc);
            return NULL;
        };

        if (strcmp(ident, "depends") == 0)
        {
            Dependency *dep = (Dependency *)calloc(1, sizeof(Dependency));
            assert(dep);

            dep->opt_ll_attributes = attributes;
            dep->path = parse_path(p);
            if (!dep->path)
            {
                error(p, "Expected dependency path");
                free(ident);
                free_dependencies(dep);
                free_dependencies(dep_head);
                free_handlers(handler_head);
                free_service_components(sc);
                return NULL;
            };

            if (!dep_head)
            {
                dep_head = dep;
            }
            else
            {
                dep_tail->next = dep;
            }
            dep_tail = dep;
        }
        else if (strcmp(ident, "handle") == 0)
        {
            Handler *handler = (Handler *)calloc(1, sizeof(Handler));
            assert(handler);

            handler->opt_ll_attributes = attributes;

            eat_whitespaces_and_comments(p);
            handler->name = parse_identifier(p);

            if (!handler->name)
            {
                error(p, "Expected handler name");
                free(ident);
                free_handlers(handler);
                free_handlers(handler_head);
                free_dependencies(dep_head);
                free_service_components(sc);
                return NULL;
            };

            eat_whitespaces_and_comments(p);
            if (p->current != '(')
            {
                error(p, "Expected '(' after handler name");
                free(ident);
                free_handlers(handler);
                free_handlers(handler_head);
                free_dependencies(dep_head);
                free_service_components(sc);
                return NULL;
            }
            advance(p);

            eat_whitespaces_and_comments(p);
            handler->opt_ll_arguments = parse_handler_arguments(p);
            eat_whitespaces_and_comments(p);

            if (p->current != ')')
            {
                error(p, "Expected ')' after handler arguments");
                free(ident);
                free_handlers(handler);
                free_handlers(handler_head);
                free_dependencies(dep_head);
                free_service_components(sc);
                return NULL;
            }
            advance(p);

            eat_whitespaces_and_comments(p);
            if (p->current == ':')
            {
                advance(p);
                eat_whitespaces_and_comments(p);
                handler->opt_return_type = parse_identifier(p);
                eat_whitespaces_and_comments(p);
                if (!handler->opt_return_type)
                {
                    error(p, "Expected return type after ':'");
                    free(ident);
                    free_handlers(handler);
                    free_handlers(handler_head);
                    free_dependencies(dep_head);
                    free_service_components(sc);
                    return NULL;
                };
            }

            if (!handler_head)
            {
                handler_head = handler;
            }
            else
            {
                handler_tail->next = handler;
            }
            handler_tail = handler;
        }
        else
        {
            error(p, "Expected 'depends' or 'handle' keyword");
            free(ident);
            free_dependencies(dep_head);
            free_handlers(handler_head);
            free_service_components(sc);
            return NULL;
        }

        if (p->current != ';')
        {
            error(p, "Expected ';' after service component");
            free(ident);
            free_dependencies(dep_head);
            free_handlers(handler_head);
            free_service_components(sc);
            return NULL;
        }
        advance(p);

        eat_whitespaces_and_comments(p);
    }
    advance(p);
    sc->ll_handlers = handler_head;
    sc->opt_ll_dependencies = dep_head;
    return sc;
}

static AstNode *
parse_node(Parser *p)
{
    eat_whitespaces_and_comments(p);
    Attribute *attributes = parse_attributes(p);

    eat_whitespaces_and_comments(p);
    char *ident = parse_identifier(p);
    if (!ident)
    {
        free_attributes(attributes);
        return NULL;
    };
    AstNode *node = (AstNode *)calloc(1, sizeof(AstNode));
    assert(node);

    eat_whitespaces_and_comments(p);
    node->opt_ll_attributes = attributes;
    if (strcmp(ident, "import") == 0)
    {
        node->type = NODE_IMPORT;
        node->node.import_node.path = parse_path(p);
        if (!node->node.import_node.path)
        {
            error(p, "Expected import path");
            free(ident);
            free_ast(node);
            return NULL;
        };
    }
    else if (strcmp(ident, "data") == 0)
    {
        node->type = NODE_DATA;
        node->node.data_node.name = parse_identifier(p);
        if (!node->node.data_node.name)
        {
            error(p, "Expected data name");
            free(ident);
            free_ast(node);
            return NULL;
        };
        eat_whitespaces_and_comments(p);
        node->node.data_node.ll_properties = parse_properties(p);
        if (!node->node.data_node.ll_properties)
        {
            free(ident);
            free_ast(node);
            return NULL;
        };
    }
    else if (strcmp(ident, "enum") == 0)
    {
        node->type = NODE_ENUM;
        node->node.enum_node.name = parse_identifier(p);
        if (!node->node.enum_node.name)
        {
            error(p, "Expected enum name");
            free(ident);
            free_ast(node);
            return NULL;
        };
        eat_whitespaces_and_comments(p);
        node->node.enum_node.ll_variants = parse_enum_variants(p);
        if (!node->node.enum_node.ll_variants)
        {
            free(ident);
            free_ast(node);
            return NULL;
        };
    }
    else if (strcmp(ident, "service") == 0)
    {
        node->type = NODE_SERVICE;
        node->node.service_node.name = parse_identifier(p);
        if (!node->node.service_node.name)
        {
            error(p, "Expected service name");
            free(ident);
            free_ast(node);
            return NULL;
        };
        eat_whitespaces_and_comments(p);
        ServiceComponents *sc = parse_service(p);
        if (!sc)
        {
            free(ident);
            free_ast(node);
            return NULL;
        };
        if (!sc->ll_handlers)
        {
            error(p, "Service must have at least one handler");
            free_service_components(sc);
            free(ident);
            free_ast(node);
            return NULL;
        }
        node->node.service_node.ll_handlers = sc->ll_handlers;
        node->node.service_node.opt_ll_dependencies = sc->opt_ll_dependencies;
        free(sc);
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
        eat_whitespaces_and_comments(p);
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
            case NODE_SERVICE:
                error(p, "Expected ';' after service declaration");
                break;
            }
            free_ast(node);
            return NULL;
        }
        advance(p);
    }
    return node;
}

static AstNode *
parse(Parser *p)
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

static Parser *
create_parser(const char *input)
{
    assert(input);
    Parser *p = (Parser *)calloc(1, sizeof(Parser));
    assert(p);
    p->input = input;
    p->line = 1;
    p->column = 1;
    return p;
}

// Parser functions
Parser *
minissd_create_parser(const char *input)
{
    return create_parser(input);
}

void minissd_free_parser(Parser *p)
{
    free(p);
}

// Parsing
AstNode *
minissd_parse(Parser *p)
{
    return parse(p);
}

void minissd_free_ast(AstNode *ast)
{
    free_ast(ast);
}

// AST Node accessors
NodeType
minissd_get_node_type(const AstNode *node)
{
    return node ? node->type : -1;
}

const char *
minissd_get_import_path(const AstNode *node)
{
    return (node && node->type == NODE_IMPORT) ? node->node.import_node.path : NULL;
}

const char *
minissd_get_data_name(const AstNode *node)
{
    return (node && node->type == NODE_DATA) ? node->node.data_node.name : NULL;
}

const char *
minissd_get_enum_name(const AstNode *node)
{
    return (node && node->type == NODE_ENUM) ? node->node.enum_node.name : NULL;
}

const char *
minissd_get_handler_name(const AstNode *node)
{
    return (node && node->type == NODE_SERVICE) ? node->node.service_node.name : NULL;
}

// Attribute accessors
Attribute *
minissd_get_attributes(const AstNode *node)
{
    return node ? node->opt_ll_attributes : NULL;
}

const char *
minissd_get_attribute_name(const Attribute *attr)
{
    return attr ? attr->name : NULL;
}

AttributeArgument *
minissd_get_attribute_arguments(const Attribute *attr)
{
    return attr ? attr->opt_ll_arguments : NULL;
}

// Property accessors
Property *
minissd_get_properties(const AstNode *node)
{
    return (node && node->type == NODE_DATA) ? node->node.data_node.ll_properties : NULL;
}

const char *
minissd_get_property_name(const Property *prop)
{
    return prop ? prop->name : NULL;
}

Attribute *
minissd_get_property_attributes(const Property *prop)
{
    return prop ? prop->attributes : NULL;
}

const char *
minissd_get_property_type(const Property *prop)
{
    return prop ? prop->type : NULL;
}

Dependency *
minissd_get_dependencies(const AstNode *node)
{
    return (node && node->type == NODE_SERVICE) ? node->node.service_node.opt_ll_dependencies : NULL;
}

Handler *
minissd_get_handlers(const AstNode *node)
{
    return (node && node->type == NODE_SERVICE) ? node->node.service_node.ll_handlers : NULL;
}

// Enum Value accessors
EnumVariant *
minissd_get_enum_variants(const AstNode *node)
{
    return (node && node->type == NODE_ENUM) ? node->node.enum_node.ll_variants : NULL;
}

const char *
minissd_get_enum_variant_name(const EnumVariant *value)
{
    return value ? value->name : NULL;
}

Attribute *
minissd_get_enum_variant_attributes(const EnumVariant *value)
{
    return value ? value->attributes : NULL;
}

int minissd_get_enum_variant(const EnumVariant *value, bool *has_value)
{
    if (!value || !value->opt_value)
    {
        if (has_value)
        {
            *has_value = false;
        }
        return 0;
    }
    if (has_value)
    {
        *has_value = true;
    }
    return *(value->opt_value);
}

HandlerArgument *
minissd_get_handler_arguments(const Handler *handler)
{
    return (handler) ? handler->opt_ll_arguments : NULL;
}
const char *
minissd_get_argument_name(const HandlerArgument *arg)
{
    return arg ? arg->name : NULL;
}
Attribute *
minissd_get_argument_attributes(const HandlerArgument *arg)
{
    return arg ? arg->attributes : NULL;
}
const char *
minissd_get_argument_type(const HandlerArgument *arg)
{
    return arg ? arg->type : NULL;
}

// Traversal functions
AstNode *
minissd_get_next_node(const AstNode *node)
{
    return node ? node->next : NULL;
}

Property *
minissd_get_next_property(const Property *prop)
{
    return prop ? prop->next : NULL;
}

EnumVariant *
minissd_get_next_enum_value(const EnumVariant *value)
{
    return value ? value->next : NULL;
}

Attribute *
minissd_get_next_attribute(const Attribute *attr)
{
    return attr ? attr->next : NULL;
}

AttributeArgument *
minissd_get_next_attribute_argument(const AttributeArgument *arg)
{
    return arg ? arg->next : NULL;
}

Dependency *
minissd_get_next_dependency(const Dependency *dep)
{
    return dep ? dep->next : NULL;
}

Handler *
minissd_get_next_handler(const Handler *handler)
{
    return handler ? handler->next : NULL;
}

HandlerArgument *
minissd_get_next_handler_argument(const HandlerArgument *arg)
{
    return arg ? arg->next : NULL;
}

const char *minissd_get_dependency_path(const Dependency *dep)
{
    return dep ? dep->path : NULL;
}

const char *minissd_get_service_name(const AstNode *node)
{
    return (node && node->type == NODE_SERVICE) ? node->node.service_node.name : NULL;
}