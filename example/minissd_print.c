#include "./minissd.h"

#include <stdlib.h>

#include <stdio.h>
#include <assert.h>

void print_attributes(Attribute *attr)
{
    while (attr)
    {
        printf("  Attribute: %s\n", minissd_get_attribute_name(attr));

        for (AttributeArgument *arg = minissd_get_attribute_arguments(attr); arg; arg = minissd_get_next_attribute_argument(arg))
        {
            printf("    Argument: %s", arg->key);
            if (arg->opt_value)
            {
                printf(" = %s", arg->opt_value);
            }
            printf("\n");
        }

        attr = minissd_get_next_attribute(attr);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "rb");
    if (!f)
    {
        printf("Failed to open file: %s\n", argv[1]);
        return 2;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); /* same as rewind(f); */

    char *source = malloc(fsize + 1);
    assert(source != NULL);

    fread(source, fsize, 1, f);
    fclose(f);

    source[fsize] = 0;

    Parser *parser = minissd_create_parser(source);
    AstNode *ast = minissd_parse(parser);

    if (!ast)
    {
        printf("Parsing failed: %s\n", parser->error);
        minissd_free_parser(parser);
        free(source);
        return 1;
    }

    for (AstNode *node = ast; node; node = minissd_get_next_node(node))
    {
        printf("Node Type: ");
        switch (minissd_get_node_type(node))
        {
        case NODE_IMPORT:
            printf("Import\n");
            printf("  Path: %s\n", minissd_get_import_path(node));
            print_attributes(minissd_get_attributes(node));
            break;

        case NODE_DATA:
            printf("Data\n");
            printf("  Name: %s\n", minissd_get_data_name(node));
            print_attributes(minissd_get_attributes(node));

            for (Property *prop = minissd_get_properties(node); prop; prop = minissd_get_next_property(prop))
            {
                printf("  Property: %s : %s\n", minissd_get_property_name(prop), minissd_get_property_type(prop));
                print_attributes(prop->attributes);
            }
            break;

        case NODE_ENUM:
            printf("Enum\n");
            printf("  Name: %s\n", minissd_get_enum_name(node));
            print_attributes(minissd_get_attributes(node));

            for (EnumVariant *value = minissd_get_enum_variants(node); value; value = minissd_get_next_enum_value(value))
            {
                bool has_value;
                int val = minissd_get_enum_variant(value, &has_value);
                printf("  Enum Value: %s", minissd_get_enum_variant_name(value));
                if (has_value)
                {
                    printf(" = %d", val);
                }
                printf("\n");
                print_attributes(value->attributes);
            }
            break;

        case NODE_SERVICE:
            printf("Service\n");
            printf("  Name: %s\n", minissd_get_service_name(node));
            print_attributes(minissd_get_attributes(node));

            for (Dependency *dep = minissd_get_dependencies(node); dep; dep = minissd_get_next_dependency(dep))
            {
                printf("  Depends: %s\n", minissd_get_dependency_path(dep));
                print_attributes(dep->opt_ll_attributes);
            }

            for (Handler *handler = minissd_get_handlers(node); handler; handler = minissd_get_next_handler(handler))
            {
                printf("  Handler: %s\n", handler->name);
                print_attributes(handler->opt_ll_attributes);
                if (handler->opt_return_type)
                {
                    printf("    Return Type: %s\n", handler->opt_return_type);
                }
                for (HandlerArgument *arg = minissd_get_handler_arguments(handler); arg; arg = minissd_get_next_handler_argument(arg))
                {
                    printf("    Argument: %s : %s\n", minissd_get_argument_name(arg), minissd_get_argument_type(arg));
                    print_attributes(minissd_get_argument_attributes(arg));
                }
            }
            break;

        default:
            printf("Unknown Node Type\n");
        }
    }

    minissd_free_ast(ast);
    minissd_free_parser(parser);

    free(source);
    return 0;
}
