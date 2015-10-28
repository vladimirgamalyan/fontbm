#include "FntInfo.h"
#include <fstream>
#include <sstream>

FntInfo FntInfo::loadFromFile(const std::string& fileName)
{
	std::ifstream f( fileName );
	if ( !f.is_open() )
		throw std::runtime_error( "error while opening file" );

	FntInfo info;

	for ( std::string line; std::getline( f, line ); )
	{
		std::string tag;
		std::istringstream( line ) >> tag;

		if ( tag == "page" )
		{
			const int page = getValueInt(line, "id");
			info.pages[page] = getQuotedValueString(line, "file");
		}
		else if ( tag == "char" )
		{
			uint32_t id = static_cast<uint32_t>(getValueInt(line, "id"));

			Character character;
			character.x = getValueInt(line, "x");
			character.y = getValueInt(line, "y");
			character.w = getValueInt(line, "width");
			character.h = getValueInt(line, "height");
			character.xoffset = getValueInt(line, "xoffset");
			character.yoffset = getValueInt(line, "yoffset");
			character.xadvance = getValueInt(line, "xadvance");
			character.page = getValueInt(line, "page");

			info.characters[id] = character;
		}
	}

	if ( f.bad() )
		throw std::runtime_error( "error while reading fnt file" );

	return info;
}

std::string FntInfo::getValueString(const std::string& line, const std::string& key)
{
	std::string k = " " + key + "=";

	size_t begin = line.find( k );
	if ( begin == std::string::npos )
		return "";

	begin += k.length();
	return line.substr( begin, line.find( " ", begin ) - begin );
}

std::string FntInfo::getQuotedValueString(const std::string& line, const std::string& key)
{
	std::string key_ = " " + key + "=\"";

	size_t begin = line.find( key_ );
	if ( begin == std::string::npos )
		return "";

	begin += key_.length();

	size_t end = line.find( "\"", begin + 1 );
	if ( end == std::string::npos )
		return "";

	return line.substr( begin, end - begin );
}

int FntInfo::getValueInt(const std::string& line, const std::string& key, int defaultVal)
{
	std::istringstream(getValueString(line, key)) >> defaultVal;
	return defaultVal;
}
