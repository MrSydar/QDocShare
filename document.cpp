#include <document.h>

pair<int, string> getChange(string message){
    pair<int, string> delta;
    int delimiter = message.find(' ');

    delta.first = stoi(message.substr(0, delimiter));
    delta.second = message.substr(delimiter + 1);

    return delta;
}

void changeLocalDocument(pair<int, string> delta, string &document){
    int carette;
    string change, tmp;

    carette = delta.first;
    change = delta.second;

    for(int i = 0; i < (int) change.size(); ++i){
        if(change[i] == '\b') {
            document.erase(carette, 1);
        }
        else {
            tmp = delta.second[i];
            document.insert(carette, tmp);
            carette += 1;
        }
    }
}
