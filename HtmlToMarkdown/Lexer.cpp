#include "Lexer.hpp"

const char* LexTypeToString(LexType type)
{
	switch (type)
	{
	case StatementStart:
		return "StatementStart";
	case StatementEnd:
		return "StatementEnd";
	case StatementEarlyEnd:
		return "StatementEarlyEnd";
	case StatementName:
		return "StatementName";
	case StatementAttribute:
		return "StatementAttribute";
	case Comment:
		return "Comment";
	case Slash:
		return "Slash";
	case Equal:
		return "Equal";
	case DocType:
		return "DocType";
	case Identifier:
		return "Identifier";
	case Unknown:
		return "Unknown";
	case StringType:
		return "StringType";
	case StatementBody:
		return "StatementBody";
	case StatementCloser:
		return "StatementCloser";
	case LEXER_END:
		return "LEXER_END";
	default:
		return "UNKNOWN LEXER VALUE";
	}
}

LexRes::LexRes(char val, LexType type) : type(type) { value = val; }
LexRes::LexRes(const char* val, LexType type) : type(type) { value = val; }
LexRes::LexRes(std::string& val, LexType type) : value(val), type(type) {}


char Lexer::Peek(size_t offset)
{
	if (pos + offset < buffer_size)
		return buff[pos + offset];

	return 0;
}

char Lexer::Peek()
{
	return Peek(0);
}

void Lexer::Consume()
{
	pos++;
}

bool Lexer::IsWhitespace()
{
	if (Peek() == ' ' || Peek() == '\r' || Peek() == '\n' || Peek() == '\t')
		return true;

	return false;
}

void Lexer::PruneWhitespace(bool newlines)
{
	while (Peek() == ' ' || (newlines && (Peek() == '\r' || Peek() == '\n' || Peek() == '\t')))
		Consume();
}

char Lexer::PeekWithoutWhitespace()
{
	// Hacky
	size_t temp_pos = pos;
	PruneWhitespace(true);

	char res = Peek();
	pos = temp_pos;

	return res;
}

LexRes Lexer::ReadDocType()
{
	PruneWhitespace(true);

	std::string doc_type;
	while (pos < buffer_size)
	{
		char current = Peek();
		if (current == '>')
		{
			Consume();
			break;
		}

		doc_type += current;
		Consume();
	}

	return { doc_type, DocType };
}

LexRes Lexer::ReadString()
{
	std::string result;
	char end_char = Peek(); // " or '

	Consume();
	while (pos < buffer_size)
	{
		char current = Peek();

		if (current == '\\' && Peek(1) == end_char)
		{
			result += current;
			result += Peek(1);

			Consume();
			Consume();
		}
		else if (current == end_char)
		{
			Consume();
			break;
		}
		else
		{
			result += current;
			Consume();
		}
	}

	return { result, StringType };
}

LexRes Lexer::ReadComment()
{
	PruneWhitespace(true);

	std::string comment;
	std::string temp; // Hold whitespace characters

	while (pos < buffer_size)
	{
		char current = Peek();
		if (current == '-' && Peek(1) == '-' && Peek(2) == '>')
		{
			Consume();
			Consume();
			Consume();

			break;
		}

		if (IsWhitespace())
			temp += current;
		else
		{
			comment += temp;
			temp = "";
			comment += current;
		}

		Consume();
	}

	return { comment, Comment };
}

LexRes Lexer::ReadAlphaNumericAndSome()
{
	std::string chain;
	while (pos < buffer_size)
	{
		char current = Peek();
		if (!isalnum(current) && current != '_')
			break;

		chain += current;
		Consume();
	}

	return { chain, Identifier };
}

LexRes Lexer::ReadStatementHeaderEntry()
{
	PruneWhitespace(true);

	// Values insdie of the header
	if (Peek() == '"' || Peek() == '\'')
		return ReadString();

	if (Peek() == '=')
	{
		Consume();
		return { '=', Equal };
	}

	std::string header_value;
	while (pos < buffer_size)
	{
		char current = Peek();

		// Really only want alpha numeric and _, not sure if thats all that is allowed though
		if (current != '"' && current != '\'' && current != '='
			&& current != '/' && current != '>' && !IsWhitespace())
			header_value += current;
		else
			break;

		Consume();
	}

	if (!header_value.size())
		return { '\0', Unknown };

	return { header_value, Identifier };
}

LexRes Lexer::ReadStatementBody()
{
	// Do not prune spaces
	std::string body;

	while (pos < buffer_size)
	{
		char current = Peek();
		if (current == '<')
			break;

		// However ignore newlines and tabs
		if (!IsWhitespace() || current == ' ')
			body += current;

		Consume();
	}

	if (body.size() == 0)
		return { '\0', Unknown };

	return { body, StatementBody };
}

// Will be //, multiline is handled in the css section
void Lexer::ReadJavascriptComment()
{
	while (pos < buffer_size)
	{
		char current = Peek();
		if (current == '\r' || current == '\n')
		{
			Consume();
			break;
		}

		Consume();
	}
}

// This will handle multiline /* */ style comments
void Lexer::ReadCSSComment()
{
	while (pos < buffer_size)
	{
		char current = Peek();
		if (current == '\\' && Peek(1) == '*')
		{
			Consume();
			Consume();
		}
		else if (current == '*' && Peek(1) == '/')
		{
			Consume();
			Consume();

			break;
		}
		else
			Consume();
	}
}

void Lexer::ReadStyleOrScriptBody()
{

	while (pos < buffer_size)
	{
		char current = Peek();
		if (current == '/' && Peek(1) == '/')
			ReadJavascriptComment();
		else if (current == '/' && Peek(1) == '*')
			ReadCSSComment();
		else if (current == '"' || current == '\'')
			ReadString();
		else
		{
			if (current == '<')
			{
				Consume();
				PruneWhitespace(true);
				if (Peek() == '/')
				{
					Consume();
					PruneWhitespace(true);

					LexRes res = ReadAlphaNumericAndSome();
					if (res.value == "style" || res.value == "script" && Peek() == '>')
					{
						Consume();
						return; // Finally the end
					}
				}
			}
			else
				Consume();
		}
	}
}

LexRes Lexer::ReadNext()
{
	PruneWhitespace(true);

	if (pos >= buffer_size)
		return { '\0', LEXER_END };

	size_t saved_pos = pos;
	char current = Peek();
	switch (current)
	{
	case '<':
	{
		Consume();

		if (Peek() == '!') // Comment or doc type
		{
			Consume();

			if (Peek() == '-' && Peek(1) == '-')
			{
				Consume();
				Consume();

				return ReadComment();
			}
			else
				return ReadDocType();
		}
		else if (PeekWithoutWhitespace() == '/') // End of a statement
		{
			PruneWhitespace(true);
			Consume();

			return { "</", StatementEnd };
		}

		return { current, StatementStart };
	}
	case '>':
	{
		Consume();

		return { current, StatementCloser };
	}
	case '/':
	{
		Consume();

		if (PeekWithoutWhitespace() == '>')
		{
			PruneWhitespace(true);
			Consume();

			return { "/>", StatementEarlyEnd };
		}

		return { current, Slash };
	}
	default:
	{
		Consume();
		return { current, Unknown };
	}
	}
}

Lexer::Lexer(char* buffer, size_t size)
	: buff(buffer)
	, buffer_size(size)
	, pos(0)
{

}