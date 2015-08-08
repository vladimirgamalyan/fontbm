#pragma once
#include <boost/iostreams/filtering_stream.hpp>

class JsonCommentsInputFilter : public boost::iostreams::input_filter
{
public:
	JsonCommentsInputFilter() : currentChar(UNDEFINED), prevChar0(UNDEFINED),
		prevChar1(UNDEFINED), prevChar2(UNDEFINED), prevChar3(UNDEFINED),
		insideString(false), insideComment(NO_COMMENT)
	{ 
	}

	template<typename Source>
	int get(Source& src)
	{
		int nextChar;
		for (;;)
		{
			if (currentChar == EOF)
				return EOF;

			nextChar = boost::iostreams::get(src);
			if (nextChar == boost::iostreams::WOULD_BLOCK)
				return boost::iostreams::WOULD_BLOCK;

			prevChar3 = prevChar2;
			prevChar2 = prevChar1;
			prevChar1 = prevChar0;
			prevChar0 = nextChar;

			if (currentChar == UNDEFINED)
			{
				currentChar = nextChar;
				continue;
			}

			if (insideComment == NO_COMMENT && currentChar == '\"')
			{
				bool escaped = prevChar2 == '\\' && prevChar3 != '\\';
				if (!escaped)
					insideString = !insideString;
			}

			if (insideString)
				break;

			if (insideComment == NO_COMMENT && currentChar == '/' && nextChar == '/')
			{
				insideComment = SINGLE_COMMENT;
				currentChar = UNDEFINED;
				continue;
			}
			else if (insideComment == SINGLE_COMMENT && (currentChar == '\n' || (currentChar == '\r' && nextChar == '\n')))
			{
				insideComment = NO_COMMENT;
				break;
			}
			else if (insideComment == NO_COMMENT && currentChar == '/' && nextChar == '*')
			{
				insideComment = MULTI_COMMENT;
				currentChar = UNDEFINED;
				continue;
			}
			else if (insideComment == MULTI_COMMENT && currentChar == '*' && nextChar == '/')
			{
				insideComment = NO_COMMENT;
				currentChar = UNDEFINED;
				continue;
			}

			if (!insideComment)
				break;

			currentChar = nextChar;
		}

		int ret = currentChar;
		currentChar = nextChar;
		return ret;
	}

	template<typename Source>
	void close(Source&)
	{
		currentChar = UNDEFINED;
		prevChar0 = UNDEFINED;
		prevChar1 = UNDEFINED;
        prevChar2 = UNDEFINED;
        prevChar3 = UNDEFINED;
		insideString = false;
		insideComment = NO_COMMENT;
	}
private:
	enum CommentType
	{
		NO_COMMENT,
		SINGLE_COMMENT,
		MULTI_COMMENT
	};
	static const int UNDEFINED = (int)(EOF - 2);

	int currentChar;
	int prevChar0;
	int prevChar1;
    int prevChar2;
    int prevChar3;
	bool insideString;
	int insideComment;
};
