#include <filesystem>
#include <fstream>
#include <string>

#include "HTMLToMarkdown.hpp"

int main()
{
	std::string path = "C:\\Users\\Alex\\Downloads\\About Me.html";
	size_t file_size = std::filesystem::file_size(path);

	std::string file_buffer(file_size, '\0');
	std::ifstream file(path, std::ios::binary);
	file.read(&file_buffer[0], file_size);
	file.close();

	std::string markdown = HTMLToMarkdown(file_buffer);

	printf("Markdown: \n%s\n", markdown.c_str());

	return 0;
}