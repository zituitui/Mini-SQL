#include "Interpreter.h"
#include "API.h"
#include "RecordManager.h"
#include <time.h>
#pragma warning( disable : 4996 )
BufferManager bf;
int main() {
	InterpretManager iter;
	int run = 1;
	time_t rawtime;
	struct tm* timeinfo;
	char b[128];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(b, sizeof(b), "%Y/%m/%d %H:%M:%S", timeinfo);
	cout << "Mini SQL system         [TIME]:" << b << endl;
	while (run) {
		try {
			cout << "[Message]Please input query:";
			iter.InputQuery();
			run = iter.Interpret_Fetch();
		}
		catch (TableException te) {
			cout << te.what() << endl;
		}
		catch (QueryException qe) {
			cout << qe.what() << endl;
		}
	}
	return 0;
}
