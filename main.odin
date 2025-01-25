package minissd

import "core:fmt"
import "core:strconv"
import "core:strings"

Ast :: [dynamic]AstNode

Import :: struct {
	attributes: [dynamic]Attribute,
	path:       string,
}

Data :: struct {
	attributes: [dynamic]Attribute,
	name:       string,
	properties: [dynamic]Property,
}
Enum :: struct {
	attributes: [dynamic]Attribute,
	name:       string,
	values:     [dynamic]EnumValue,
}

AstNode :: union {
	Ast,
	Import,
	Data,
	Enum,
}

Argument :: struct {
	key:   string,
	value: Maybe(string),
}

Attribute :: struct {
	name:      string,
	arguments: [dynamic]Argument,
}

Property :: struct {
	attributes: [dynamic]Attribute,
	name:       string,
	type:       string,
}

EnumValue :: struct {
	attributes: [dynamic]Attribute,
	name:       string,
	value:      Maybe(int),
}

ParseError :: struct {
	message: string,
	line:    int,
	column:  int,
}

Parser :: struct {
	input:   string,
	current: u8,
	index:   int,
	line:    int,
	column:  int,
}

advance :: proc(p: ^Parser) {
	if p.index >= len(p.input) {
		p.current = 0
		return
	}

	p.current = p.input[p.index]
	p.index += 1
	if p.current == '\n' {
		p.line += 1
		p.column = 1
	} else {
		p.column += 1
	}
}

is_whitespace :: proc(c: u8) -> bool {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r'
}

eat_whitespace :: proc(p: ^Parser) {
	for p.current != 0 && is_whitespace(p.current) {
		advance(p)
	}
}

error :: proc(p: ^Parser, message: string) -> ParseError {
	return ParseError{message = message, line = p.line, column = p.column}
}


is_numeric :: proc(c: u8) -> bool {
	return c >= '0' && c <= '9'
}

is_alphanumeric :: proc(c: u8) -> bool {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
}

parse_int :: proc(p: ^Parser) -> (num: int, err: Maybe(ParseError)) {
	ident := ""
	for p.current != 0 && is_numeric(p.current) {
		ident = strings.concatenate([]string{ident, strings.string_from_ptr(&p.current, 1)})
		advance(p)
	}
	if len(ident) > 0 {
		num = strconv.atoi(ident)
		return
	}
	err = error(p, "Expected integer")
	return
}

parse_ident :: proc(p: ^Parser) -> (ident: string, err: Maybe(ParseError)) {
	for p.current != 0 && (is_alphanumeric(p.current) || p.current == '_') {

		ident = strings.concatenate([]string{ident, strings.string_from_ptr(&p.current, 1)})
		advance(p)
	}
	if len(ident) > 0 {
		return
	}
	err = error(p, "Expected identifier")
	return
}

parse_path :: proc(p: ^Parser) -> (path: string, err: Maybe(ParseError)) {
	for p.current != 0 && (is_alphanumeric(p.current) || p.current == '_' || p.current == ':') {
		path = strings.concatenate([]string{path, strings.string_from_ptr(&p.current, 1)})
		advance(p)
	}
	if len(path) > 0 {
		return
	}
	err = error(p, "Expected path")
	return
}

parse_attributes :: proc(p: ^Parser) -> (attributes: [dynamic]Attribute, err: Maybe(ParseError)) {
	eat_whitespace(p)
	for p.current == '#' {
		advance(p) // Consume '#'
		if p.current == '[' {
			advance(p) // Consume '['
			eat_whitespace(p)
			attribute := parse_attribute(p) or_return
			append(&attributes, attribute)
			eat_whitespace(p)
			if p.current != ']' {
				err = error(p, "Expected ']' after attribute")
			}
			advance(p) // Consume ']'
			eat_whitespace(p)
		}
	}
	return
}

parse_attribute :: proc(p: ^Parser) -> (attribute: Attribute, err: Maybe(ParseError)) {
	eat_whitespace(p)
	name := parse_ident(p) or_return
	arguments: [dynamic]Argument
	eat_whitespace(p)
	if p.current == '(' {
		advance(p) // Consume '('
		eat_whitespace(p)
		for {
			key := parse_ident(p) or_return
			value: Maybe(string)
			eat_whitespace(p)
			if p.current == '=' {
				advance(p) // Consume '='
				eat_whitespace(p)
				value = parse_string(p) or_return
			}
			append(&arguments, Argument{key, value})
			eat_whitespace(p)
			if p.current != ',' {
				break
			}
			advance(p) // Consume ','
			eat_whitespace(p)
		}
		if p.current != ')' {
			err := error(p, "Expected ')' after attribute arguments")
			return
		}
		advance(p) // Consume ')'
	}
	attribute = Attribute {
		name      = name,
		arguments = arguments,
	}
	return
}

parse_string :: proc(p: ^Parser) -> (value: string, err: Maybe(ParseError)) {
	value = ""
	if p.current != '"' {
		err = error(p, "Expected string")
		return
	}
	advance(p) // Consume '"'
	for p.current != 0 && p.current != '"' {
		value = strings.concatenate([]string{value, strings.string_from_ptr(&p.current, 1)})
		advance(p)
	}
	if p.current != '"' {
		err = error(p, "Unterminated string")
		return
	}
	advance(p) // Consume '"'
	return
}

parse_import :: proc(
	p: ^Parser,
	attributes: [dynamic]Attribute,
) -> (
	result: Import,
	err: Maybe(ParseError),
) {
	eat_whitespace(p)
	path := parse_path(p) or_return
	eat_whitespace(p)
	result = Import{attributes, path}
	return
}

parse_property :: proc(p: ^Parser) -> (result: Property, err: Maybe(ParseError)) {
	eat_whitespace(p)
	attributes := parse_attributes(p) or_return
	name := parse_ident(p) or_return
	eat_whitespace(p)
	if p.current == ':' {
		advance(p)
		eat_whitespace(p)
		typ := parse_ident(p) or_return
		eat_whitespace(p)
		if p.current == ',' {
			advance(p)
			result = Property{attributes, name, typ}
		} else {
			err = error(p, "Expected ',' after property")
		}
	} else {
		error(p, "Expected ':' after property name")
	}
	return
}

parse_data :: proc(
	p: ^Parser,
	attributes: [dynamic]Attribute,
) -> (
	result: Data,
	err: Maybe(ParseError),
) {
	eat_whitespace(p)
	name := parse_ident(p) or_return
	eat_whitespace(p)
	if p.current == '{' {
		advance(p)
		properties: [dynamic]Property
		eat_whitespace(p)
		for p.current != '}' {
			property := parse_property(p) or_return
			append(&properties, property)
			eat_whitespace(p)
		}
		advance(p) // consume '}'
		eat_whitespace(p)
		result = Data{attributes, name, properties}
	}

	return
}

parse_enum :: proc(
	p: ^Parser,
	attributes: [dynamic]Attribute,
) -> (
	result: Enum,
	err: Maybe(ParseError),
) {
	eat_whitespace(p)
	name := parse_ident(p) or_return
	eat_whitespace(p)
	if p.current == '{' {
		advance(p)
		eat_whitespace(p)
		values: [dynamic]EnumValue
		for p.current != '}' {
			attributes := parse_attributes(p) or_return
			name := parse_ident(p) or_return
			eat_whitespace(p)
			if p.current == '=' {
				advance(p)
				eat_whitespace(p)
				num := parse_int(p) or_return
				ev := EnumValue{attributes, name, num}
				append(&values, ev)
			} else {
				append(&values, EnumValue{attributes, name, nil})
			}
			if p.current == ',' {
				advance(p)
			} else {
				err = error(p, "Expected ',' after enum value")
			}
			eat_whitespace(p)
		}
		advance(p) // consume '}'
		eat_whitespace(p)
		result = Enum{attributes, name, values}
	} else {
		err = error(p, "Expected '{' after enum name")
	}
	return
}

parse_node :: proc(p: ^Parser) -> (result: AstNode, err: Maybe(ParseError)) {
	eat_whitespace(p)
	attributes := parse_attributes(p) or_return
	ident := parse_ident(p) or_return
	switch (ident) {
	case "import":
		result = parse_import(p, attributes) or_return
	case "data":
		result = parse_data(p, attributes) or_return
	case "enum":
		result = parse_enum(p, attributes) or_return
	case:
		err = error(p, "Unknown node type")
	}
	return
}

parse_file :: proc(p: ^Parser) -> (file: Ast, err: Maybe(ParseError)) {
	for p.current != 0 {
		eat_whitespace(p)
		if p.current != 0 {
			node := parse_node(p) or_return
			append(&file, node)
		}
	}

	return
}

parse :: proc(input: string) -> (file: Ast, err: Maybe(ParseError)) {
	parser := Parser {
		input   = input,
		current = 0,
		index   = 0,
		line    = 1,
		column  = 1,
	}
	advance(&parser)

	return parse_file(&parser)
}

main :: proc() {
	input := `#[derive(Debug)]
import std::path::Path

#[table]
data MyData {
    #[column(name="field1", type="string")]
    field1: string,
    #[column(name="field2", type="int")]
    field2: int,
}

#[repr(C)]
enum MyEnum {
    Value1,
    Value2 = 42,
}`


	ast, err := parse(input)
	if err != nil {
		fmt.println("Error:", err.?.message, "at line", err.?.line, "column", err.?.column)
	} else {
		fmt.printfln("%#v", ast)
	}
}
