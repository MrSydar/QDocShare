#ifndef QUITINFO_H
#define QUITINFO_H

#include <mutex>

class quitCode {
    private:
    enum codes {NOQUIT, ERROR, QUIT};
    codes code;
    std::mutex m, w;

    public:
    void setNoQuit();

    void setError();

    void setQuit();

    bool isNoQuit();

    bool isQuit();

    bool isError();

    void wait();

    quitCode();
};


#endif // QUITINFO_H
