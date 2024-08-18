#include <string>

// Lexer types
enum LexType
{
	StatementStart,
	StatementEnd,
	StatementEarlyEnd,
	StatementName,
	StatementAttribute,
	Comment,
	Slash,
	Equal,
	DocType,
	Identifier,
	Unknown,
	StringType,
	StatementBody,
	StatementCloser,

	LEXER_END
};

// Convert a lexer type to a const char*
const char* LexTypeToString(LexType type);

// Lexer return value
struct LexRes
{
	std::string value; // Contents
	LexType type; // Lexer type

	LexRes(char val, LexType type);
	LexRes(const char* val, LexType type);
	LexRes(std::string& val, LexType type);
};

class Lexer
{
	char* buff; // Read buffer
	size_t buffer_size; // Size of read buffer

	size_t pos; // Position inside the buffer

public:

	char Peek(size_t offset); // Get value at offset
	char Peek(); // Get current value

	// Eat the current character and move forwards
	void Consume();

	// Is the current character whitespace
	bool IsWhitespace();

	// Consume all whitespace until the next character
	void PruneWhitespace(bool newlines);

	// Get the next character, ignoring whitespace, not moving the position
	char PeekWithoutWhitespace();

	// Reads a DocType lexer value
	LexRes ReadDocType();

	// Reads a String lexer value
	LexRes ReadString();

	// Reads a Comment lexer value
	LexRes ReadComment();

	// Reads alpha numberic with _
	LexRes ReadAlphaNumericAndSome();
	
	// Reads the key/equal/value in <name key=value>
	LexRes ReadStatementHeaderEntry();

	// Reads the contents after <>
	LexRes ReadStatementBody();

	// Will be //, multiline is handled in the css section
	void ReadJavascriptComment();

	// This will handle multiline /* */ style comments
	void ReadCSSComment();

	// Reads the contents of a css or javascript body
	void ReadStyleOrScriptBody();

	// Find the next character entry and get its type
	LexRes ReadNext();

	Lexer(char* buffer, size_t size);
};