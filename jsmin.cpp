/* jsmin.c
   2008-08-03
   Ported to C++ by Adrien Chey <adrien@betterinbox.com>

Copyright (c) 2002 Douglas Crockford  (www.crockford.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "jsmin.h"

#include <cstdlib>
#include <cstdio>

using namespace std;

/* isAlphanum -- return true if the character is a letter, digit, underscore,
		dollar sign, or non-ASCII character.
*/
static bool isAlphanum(int c) noexcept
{
	return ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
		(c >= 'A' && c <= 'Z') || c == '_' || c == '$' || c == '\\' ||
		c > 126);
}

void JsMin::init()
{
	m_theLookahead = EOF;
	m_charPos = 0;
}

std::vector<char>* JsMin::minify(const char *input, int len)
{
	m_data = input;
	m_dataLen = len;
	jsmin();
	return &m_output;
}

char JsMin::getDataChar() noexcept
{
	if (m_charPos >= m_dataLen) {
		return EOF;
	}
	const char res = m_data[m_charPos];
	m_charPos++;
	return res;
}

void JsMin::putDataChar(char dataChar) noexcept
{
	if (dataChar != 0) {
		m_output.emplace_back(dataChar);
	}
}

/* get -- return the next character from stdin. Watch out for lookahead. If
		the character is a control character, translate it to a space or
		linefeed.
*/
char JsMin::get() noexcept
{
	int c = m_theLookahead;
	m_theLookahead = EOF;
	if (c == EOF) {
		c = getDataChar();
	}
	if (c == '\r') {
		return '\n';
	}
	return c;
}

/* peek -- get the next character without getting it.
*/
int JsMin::peek() noexcept
{
	m_theLookahead = get();
	return m_theLookahead;
}

/* next -- get the next character, excluding comments. peek() is used to see
		if a '/' is followed by a '/' or '*'.
*/
int JsMin::next()
{
	int c = get();
	if (c == '/') {
		switch (peek()) {
			case '/':
				for (;;) {
					c = get();
					if (c <= '\n') {
						return c;
					}
				}
			case '*':
				get();
				for (;;) {
					switch (get()) {
						case '*':
							if (peek() == '/') {
								get();
								return ' ';
							}
							break;
						case EOF:
							fprintf(stderr, "Error: Unterminated comment in file.\n");
							throw exception();
					}
				}
			default:
				return c;
		}
	}
	return c;
}

/* action -- do something! What you do is determined by the argument.
   action treats a string as a single character. Wow!
   action recognizes a regular expression if it is preceded by ( or , or =.
*/
void JsMin::action(ACTION d)
{
	switch (d) {
		case OUTPUT:
			putDataChar(m_theA);
		case COPY:
			m_theA =  m_theB;
			if (m_theA == '\'' || m_theA == '"') {
				for (;;) {
					putDataChar(m_theA);
					m_theA = get();
					if (m_theA == m_theB) {
						break;
					}
					if (m_theA == '\\') {
						putDataChar(m_theA);
						m_theA = get();
					}
					if (m_theA == EOF) {
						fprintf(stderr, "Error: Unterminated string literal in file.\n");
						throw exception();
					}
				}
			}
		case GET:
			m_theB = next();
			if (m_theB == '/' && (m_theA == '(' || m_theA == ',' || m_theA == '=' ||
								m_theA == ':' || m_theA == '[' || m_theA == '!' ||
								m_theA == '&' || m_theA == '|' || m_theA == '?' ||
								m_theA == '{' || m_theA == '}' || m_theA == ';' ||
								m_theA == '\n')) {
				putDataChar(m_theA);
				putDataChar(m_theB);
				for (;;) {
					m_theA = get();
					if (m_theA == '/') {
						break;
					}
					if (m_theA =='\\') {
						putDataChar(m_theA);
						m_theA = get();
					}
					if (m_theA == EOF) {
						fprintf(stderr, "Error: Unterminated Regular Expression literal in file.\n");
						throw exception();
					}
					putDataChar(m_theA);
				}
				m_theB = next();
			}
	}
}

/* jsmin -- Copy the input to the output, deleting the characters which are
		insignificant to JavaScript. Comments will be removed. Tabs will be
		replaced with spaces. Carriage returns will be replaced with linefeeds.
		Most spaces and linefeeds will be removed.
*/
void JsMin::jsmin()
{
	m_theA = 0;
	action(GET);
	while (m_theA != EOF) {
		switch (m_theA) {
			case ' ':
				if (isAlphanum(m_theB)) {
					action(OUTPUT);
				} else {
					action(COPY);
				}
				break;
			case '\n':
				switch (m_theB) {
					case '{':
					case '[':
					case '(':
					case '+':
					case '-':
						action(OUTPUT);
						break;
					case ' ':
						action(GET);
						break;
					default:
						if (isAlphanum(m_theB)) {
							action(OUTPUT);
						} else {
							action(COPY);
						}
				}
				break;
			default:
				switch (m_theB) {
					case ' ':
						if (isAlphanum(m_theA)) {
							action(OUTPUT);
							break;
						}
						action(GET);
						break;
					case '\n':
						switch (m_theA) {
							case '}':
							case ']':
							case ')':
							case '+':
							case '-':
							case '"':
							case '\'':
								action(OUTPUT);
								break;
							default:
								if (isAlphanum( m_theA)) {
									action(OUTPUT);
								} else {
									action(GET);
								}
							}
						break;
					default:
						action(OUTPUT);
						break;
				}
		}
	}
}
