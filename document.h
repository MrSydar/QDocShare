#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>

using namespace std;

pair<int, string> getChange(string message);

void changeLocalDocument(pair<int, string> delta, string &document);

#endif // DOCUMENT_H
