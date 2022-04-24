#ifndef LOG_H
#define LOG_H

namespace Log {
    void Init(void);
    void Error(const char *data, ...);
    void Exit(void);
}

#endif
