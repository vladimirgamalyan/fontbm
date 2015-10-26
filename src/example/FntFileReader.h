#pragma once

class FntFileReader
{
public:
	void loadFromFile(const std::string& fileName);

	int paddingTop;
	int paddingRight;
	int paddingBottom;
	int paddingLeft;

	struct Character
	{
		int x;
		int y;
		int w;
		int h;
		int xoffset;
		int yoffset;
		int xadvance;
		int page;
	};

	std::map<uint32_t, Character> characters;
	std::map<int, std::string> pages;

private:
	static std::string getValueString(const std::string& line, const std::string& key);
	static std::string getQuotedValueString(const std::string& line, const std::string& key);
	static int getValueInt(const std::string& line, const std::string& key, int default = -1);
	static std::vector<int> getCsvInt(const std::string& str);
};
