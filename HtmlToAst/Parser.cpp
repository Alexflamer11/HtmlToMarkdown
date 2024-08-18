#include "Parser.hpp"

HTMLElement::HTMLElement()
	: is_body(false)
	, name("")
	, attributes({})
	, children({})
{}

HTMLElement::~HTMLElement()
{
	for (auto* element : children)
		delete element;
}

void ParseHTML::AddElementToStack(HTMLElement* element)
{
	if (stack_objects.size())
		stack_objects.top()->children.push_back(element);
	else
		root->children.push_back(element);
}

bool ParseHTML::ReadStatement()
{
	// return true = no end needed
	auto* element = new HTMLElement();
	AddElementToStack(element);

	// Get the name of the object if it exists
	LexRes result = lexer.ReadStatementHeaderEntry();
	if (result.type == Unknown)
		return true;

	element->name = result.value;

	// Check for attributes, unknown = / or >
	result = lexer.ReadStatementHeaderEntry();

	// Attributes
	std::string attribute_name; // Cache name for equals
	bool had_equals = false;
	while (result.type != Unknown)
	{
		if (result.type == Equal)
			had_equals = true;
		else
		{
			if (!had_equals)
			{
				// If a new name is found and equals is not present, push as is
				if (attribute_name.size())
					element->attributes.push_back({ attribute_name, "" });

				attribute_name = result.value;
			}
			else
			{
				element->attributes.push_back({ attribute_name, result.value });
				attribute_name = "";
			}

			had_equals = false;
		}

		result = lexer.ReadStatementHeaderEntry();
	}

	//printf("Element: %s\n", element->name.c_str());
	//for (auto& attribute : element->attributes)
	//{
	//	printf("  -> %s = %s\n", attribute.name.c_str(), attribute.value.c_str());
	//}

	result = lexer.ReadNext(); // Read end

	//printf("statement end lex result: %s - %s\n", LexTypeToString(result.type), result.value.c_str());

	// Handle statmenets that do not nest (Statement end can be ignored as its properly handled)
	if (result.type == StatementEarlyEnd || result.type == StatementCloser)
	{
		if (element->name == "br" || element->name == "img")
			return true;
	}

	// Add to the stack
	if (result.type != StatementEarlyEnd)
		stack_objects.push(element);

	return result.type == StatementEarlyEnd;
}

void ParseHTML::ReadStatementEnd()
{
	// Read statement name, unknown means it ends in / or > with no name
	LexRes result = lexer.ReadStatementHeaderEntry();
	if (result.type == Unknown)
		return;

	std::string statement_end_name = result.value;

	lexer.ReadNext(); // Removes >
	//printf("Statement closed: %s\n", statement_end_name.c_str());

	// Try to remove the element
	while (stack_objects.size())
	{
		if (stack_objects.top()->name == statement_end_name)
		{
			//printf("Popping element: %s\n", statement_end_name.c_str());
			stack_objects.pop();
			break;
		}

		stack_objects.pop();
	}
}

bool ParseHTML::ParseStatement()
{
	if (ReadStatement())
		return true;

	if (stack_objects.size()
		&& (stack_objects.top()->name == "style" || stack_objects.top()->name == "script"))
	{
		// Read until </style> or </script> is found, ignoring comments of /* */
		lexer.ReadStyleOrScriptBody();

		return true;
	}

	while (true)
	{
		// Read statements
		LexRes body = lexer.ReadStatementBody();
		if (body.type == Unknown)
		{
			LexRes result = lexer.ReadNext();
			if (result.type == LEXER_END)
				return false;

			switch (result.type)
			{
			case StatementStart:
			{
				ParseStatement();
				break;
			}
			case StatementEnd:
			{
				ReadStatementEnd();
				return true;
			}
			case Comment:
			{
				// Lazy fix to save memory
				auto* element = new HTMLElement();
				element->name = "Comment";

				auto* child = new HTMLElement();
				child->is_body = true;
				child->name = result.value;

				element->children.push_back(child);

				AddElementToStack(element);

				break;
			}
			default:
				break;
			}
		}
		else
		{
			auto* element = new HTMLElement();
			element->is_body = true;
			element->name = body.value;
			printf("Body value: %s\n", body.value.c_str());

			AddElementToStack(element);
		}
	}

	return true;
}

void ParseHTML::Parse()
{
	while (true)
	{
		// Read statements but top level
		LexRes result = lexer.ReadNext();
		if (result.type == LEXER_END)
			break;

		if (result.type == StatementStart)
		{
			if (!ParseStatement())
				break;
		}
		else if (result.type == Comment)
		{
			// Lazy fix to save memory
			auto* element = new HTMLElement();
			element->name = "Comment";

			auto* child = new HTMLElement();
			child->is_body = true;
			child->name = result.value;

			element->children.push_back(child);

			AddElementToStack(element);
		}
	}
}

std::shared_ptr<HTMLElement> ParseHTML::GetRoot()
{
	return root;
}

ParseHTML::ParseHTML(std::string& buffer)
	: buff(buffer.data())
	, buffer_size(buffer.size())
	, pos(0)
	, root(std::make_shared<HTMLElement>())
	, stack_objects()
	, lexer(buff, buffer_size)
{
}