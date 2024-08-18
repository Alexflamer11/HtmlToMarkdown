#include "Parser.hpp"

void PrintHTML(HTMLElement* element, size_t indent_count)
{
	std::string tabs(indent_count, '\t');

	if (indent_count)
		printf("%s", tabs.c_str());

	if (element->is_body)
	{
		printf("Body: %s\n", element->name.c_str());
	}
	else
	{
		printf("Element: %s\n", element->name.c_str());

		for (auto& attribute : element->attributes)
		{
			printf("%s -> ", tabs.c_str());

			if (attribute.value.size())
				printf("%s = %s\n", attribute.name.c_str(), attribute.value.c_str());
			else
				printf("%s\n", attribute.name.c_str());
		}

		for (auto* child : element->children)
			PrintHTML(child, indent_count + 1);
	}
}

bool CharIsNum(char num)
{
	if (num >= '0' && num <= '9')
		return true;
}

// Would be better as AST but thats ok
class HTMLToMarkdownFormatter
{
	std::string markdown;

	void TraverseTree(HTMLElement* element, size_t indent_count)
	{
		if (element->is_body)
		{
			// No newlines or anything, this is raw text
			markdown += element->name.c_str();
		}
		else
		{
			std::string& name = element->name;
			if (name == "Comment")
			{
				// Maybe?, must have to not print anyways though due to lazy handling
				return;
			}
			// Heading
			else if (name.size() == 2 && name[0] == 'h' && CharIsNum(name[1]))
			{
				for (auto* child : element->children)
				{
					markdown += "\n\n"; // Extra newline for spacing

					for (size_t i = 0; i < name[1] - '0'; i++)
						markdown += '#';

					markdown += " ";
					TraverseTree(child, indent_count + 1);
					markdown += "\n";

					return;
				}

			}
			else if (name == "p")
			{
				markdown += "\n";
			}
			else if (name == "img")
			{
				std::string source;
				std::string alt;

				for (auto& attribute : element->attributes)
				{
					printf("name: %s\n", attribute.name.c_str());
					if (attribute.name == "src")
						source = attribute.value;
					else if (attribute.name == "alt")
						alt = attribute.value;
				}

				if (source.size())
					markdown += "Img: " + source;

				if (source.size() && alt.size())
					markdown += "; ";

				if (alt.size())
					markdown += "Alt: " + alt;

				printf("\n");
			}
			else if (name == "br")
				markdown += "\n";
			else if (name == "li")
			{
				for (auto* child : element->children)
				{
					markdown += "- ";
					TraverseTree(child, indent_count + 1);
				}

				printf("\n");

				return;
			}

			size_t previous_size = markdown.size();
			for (auto* child : element->children)
				TraverseTree(child, indent_count + 1);

			if (markdown.size() && markdown.size() != previous_size && markdown.back() != '\n')
				markdown += "\n";
		}
	}

public:
	std::string Parse(std::shared_ptr<HTMLElement> root)
	{
		for (auto* element : root->children)
			TraverseTree(element, 0);

		return markdown;
	}
};

std::string HTMLToMarkdown(std::string& input)
{
	ParseHTML html_parser(input);
	html_parser.Parse();

	std::shared_ptr<HTMLElement> root = html_parser.GetRoot();

	HTMLToMarkdownFormatter markdown;
	return markdown.Parse(root);
}