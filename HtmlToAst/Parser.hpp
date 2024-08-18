#pragma once

#include "Lexer.hpp"

#include <stack>
#include <memory>
#include <string>
#include <vector>

struct Attribute
{
	std::string name; // key
	std::string value;
};

struct HTMLElement
{
	bool is_body; // Is the statment <> or is it the content after
	std::string name; // Element name such as h1, p, span, div
	std::vector<Attribute> attributes; // Attributes suck as key=value
	std::vector<HTMLElement*> children; // Anything contained inside the element

	HTMLElement();
	~HTMLElement();
};

class ParseHTML
{
	char* buff;
	size_t buffer_size;

	size_t pos;

	std::shared_ptr<HTMLElement> root; // Root element
	std::stack<HTMLElement*> stack_objects; // Save a stack of currently used objects

	Lexer lexer;

	void AddElementToStack(HTMLElement* element);

public:

	bool ReadStatement(); // Read <name key=val>

	void ReadStatementEnd(); // Read a </name> block

	bool ParseStatement(); // Parse a <name key=value>stuff</name>

	void Parse(); // Parses the html buffer into a format usable by the markdown formatter

	std::shared_ptr<HTMLElement> GetRoot(); // Root for the html formatter

	ParseHTML(std::string& buffer);
};
