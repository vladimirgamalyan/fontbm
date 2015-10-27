#include "FntFileReader.h"
#include <fstream>
#include <sstream>

FntFileReader::Info FntFileReader::loadFromFile(const std::string& fileName)
{
	std::ifstream f( fileName );
	if ( !f.is_open() )
		throw std::runtime_error( "error while opening file" );

	FntFileReader::Info info;

	for ( std::string line; std::getline( f, line ); )
	{
		std::string tag;
		std::istringstream( line ) >> tag;

		if ( tag == "info" )
		{
			const std::string paddingString = getValueString(line, "padding");
			const std::vector<int> values = getCsvInt(paddingString);
			if (values.size() == 4)
			{
				info.padding.top = values[0];
				info.padding.right = values[1];
				info.padding.bottom = values[2];
				info.padding.left = values[3];
			}
		}
		else if ( tag == "page" )
		{
			const int page = getValueInt(line, "id");
			info.pages[page] = getQuotedValueString(line, "file");
		}
		else if ( tag == "char" )
		{
			uint32_t id = static_cast<uint32_t>(getValueInt(line, "id"));

			Info::Character character;
			character.x = getValueInt(line, "x");
			character.y = getValueInt(line, "y");
			character.w = getValueInt(line, "w");
			character.h = getValueInt(line, "h");
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

std::string FntFileReader::getValueString(const std::string& line, const std::string& key)
{
	std::string k = " " + key + "=";

	size_t begin = line.find( k );
	if ( begin == std::string::npos )
		return std::string();

	begin += k.length();
	return line.substr( begin, line.find( " ", begin ) - begin );
}

std::string FntFileReader::getQuotedValueString(const std::string& line, const std::string& key)
{
	std::string key_ = " " + key + "=\"";

	size_t begin = line.find( key_ );
	if ( begin == std::string::npos )
		return std::string();

	begin += key_.length();

	size_t end = line.find( "\"", begin + 1 );
	if ( end == std::string::npos )
		return std::string();

	return line.substr( begin, end - begin );
}

int FntFileReader::getValueInt(const std::string& line, const std::string& key, int defaultVal)
{
	std::istringstream(getValueString(line, key)) >> defaultVal;
	return defaultVal;
}

std::vector<int> FntFileReader::getCsvInt( const std::string& str )
{
	std::istringstream iss( str );
	std::string s;
	int n;
	std::vector<int> values;
	while ( std::getline( iss, s, ',' ) && std::istringstream( s ) >> n )
		values.push_back( n );
	return values;
}
