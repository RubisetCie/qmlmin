#include "jsmin.h"

#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>

using namespace std;

static void displayHelp(char *progname)
{
	printf("Usage: %s (options) (input file 1) [(...) (input file n)]\n\nOptions:\n  -o : Output file or output directory depending on the input.\n", progname);
}

constexpr const char *temporaryFile =
#ifdef _WIN32
	".jsmin-temp.js";
#else
	"/tmp/.jsmin-temp.js";
#endif

int main(int argc, char **argv)
{
	if (argc < 2) {
		displayHelp(argv[0]);
		return 0;
	}

	ofstream out;
	string outputBase;
	vector<char> *outBuffer;
	char *inBuffer;
	size_t bufSize = 0;
	bool isDir = false;
	int i = 1;

	/* Handling options */
	if (argv[1][0] == '-') {
		switch (argv[1][1]) {
			case '?':
			case 'h':
				displayHelp(argv[0]);
				return 0;

			case 'o':
				if (argc > 4) {
					/* If multiple input follows, treat the out as a directory */
					isDir = true;
				}
				if (argc <= 3) {
					fprintf(stderr, "Error: Incomplete command.\n");
					displayHelp(argv[0]);
					return 1;
				}
				outputBase = argv[2];
				i = 3;

				/* Check the validity of the path */
#ifdef _WIN32
				if (!isDir && outputBase.back() == '\\') {
#else
				if (!isDir && outputBase.back() == '/') {
#endif
					fprintf(stderr, "Error: The output path must not be a directory.\n");
					return 1;
				}
				break;

			default:
				fprintf(stderr, "Error: Unrecognized option: '%c'\n", argv[1][1]);
				displayHelp(argv[0]);
				return 1;
		}
	}

	/* Processing each input file */
	JsMin jsMin;
	bool same = false;
	for (; i < argc; i++) {
		/* Opening the input file */
		ifstream in(argv[i], ios_base::in | ios_base::binary);

		if (in.fail()) {
			fprintf(stderr, "Error: Can't open the input file: %s\n", argv[i]);
			continue;
		}

		/* When the output file is specified */
		if (!outputBase.empty()) {
			if (isDir) {
				string path(outputBase);
#ifdef _WIN32
				path.append("\\");
#else
				path.append("/");
#endif
				path.append(argv[i]);

				out.open(path, ios_base::out | ios_base::binary);
				if (out.fail()) {
					fprintf(stderr, "Error: Can't create the output file: %s\n", path.c_str());
					in.close();
					continue;
				}
			}
			else {
				out.open(outputBase, ios_base::out | ios_base::binary);
				if (out.fail()) {
					fprintf(stderr, "Error: Can't create the output file: %s\n", outputBase.c_str());
					in.close();
					continue;
				}
			}
		}
		else
		{
			/* When the output file is not specified, open a temporary file instead */
			out.open(temporaryFile, ios_base::out | ios_base::binary);
			if (out.fail()) {
				fprintf(stderr, "Error: Can't create the temporary output file\n");
				in.close();
				continue;
			}
			same = true;
		}

		/* Get length of the input file */
		in.seekg(0, ios::end);
		const size_t length = in.tellg();
		in.seekg(0, ios::beg);

		/* Allocate the input buffer */
		if (length > bufSize) {
			if (bufSize == 0) {
				inBuffer = reinterpret_cast<char*>(::malloc(length));
			}
			else {
				inBuffer = reinterpret_cast<char*>(::realloc(inBuffer, length));
			}
			if (!inBuffer) {
				fprintf(stderr, "Error: Failed to initialize input buffer.\n");
				in.close();
				out.close();
				continue;
			}
			bufSize = length;
		}

		/* Read full file */
		in.read(inBuffer, length);
		const bool fail = in.fail();
		in.close();
		if (fail) {
			fprintf(stderr, "Error: Failed to read from the input file.\n");
			out.close();
			continue;
		}

		/* Minify */
		try {
			jsMin.init();
			outBuffer = jsMin.minify(inBuffer, length);

			/* Write the result to the output */
			out.write(outBuffer->data(), outBuffer->size());
			//out.write(inBuffer, length);
		}
		catch (exception) {}

		/* Close the output file */
		out.close();

		/* If asked to write "in-place", replace the input by the temporary file */
		if (same) {
#ifdef _WIN32
			string relativePath = "./";
#else
			string relativePath = ".\\";
#endif
			relativePath.append(argv[i]);
			if (::remove(relativePath.c_str()) != 0) {
				fprintf(stderr, "Error: Failed to override the input file.\n");
				continue;
			}
			if (::rename(temporaryFile, relativePath.c_str()) != 0) {
				fprintf(stderr, "Error: Failed to override the input file.\n");
			}
		}
	}

	free(inBuffer);

	return 0;
}
