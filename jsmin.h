#ifndef JSMIN_H
#define JSMIN_H

#include <vector>

/*
 * 1   Output A. Copy B to A. Get the next B.
 * 2   Copy B to A. Get the next B. (Delete A).
 * 3   Get the next B. (Delete B).
 */
enum ACTION
{
	OUTPUT = 1,
	COPY,
	GET
};

class JsMin
{
public:
	void init();
	std::vector<char>* minify(const char *input, int len);

private:
	void jsmin();
	void action(ACTION d);
	int next();
	char get() noexcept;
	int peek() noexcept;
	char getDataChar() noexcept;
	void putDataChar(char dataChar) noexcept;

private:
	std::vector<char> m_output;
	const char *m_data;
	int m_dataLen;
	int m_theA;
	int m_theB;
	int m_theLookahead;
	int m_charPos;
};

#endif // JSMIN_H
