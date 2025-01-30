# MiniSSD

Light version of my data description text format [SSD](https://github.com/ssd-codegen/ssd),
provided as a C99 library with no dependencies.

## About
The idea of SSD was to create a simple data format that can be used to write custom
tools for generating code. Wether you just need to generate some serialization code
or you want to generate code based on metadata that you attach to the data definitions,
it should support you writing your own tools to get more out of your time.

## Why another version?
Mostly because I wanted to try to do a hand written parser in C, but also because theres
people who could need something like SSD, but can't or don't want to rely on something
that big.

So this is my attempt at providing a simple and lightweight alternative to the full framework.

## When to use the light version?
If you're already heavily invested in C or C++ and just want to use the data format
or if you believe in minimalism, this is a good alternative for the full SSD framework.

## When to use the full SSD instead?
If you want a more streamlined experience or want to use one of the pre-existing generators
(rhai, handlebars, wasm, etc).