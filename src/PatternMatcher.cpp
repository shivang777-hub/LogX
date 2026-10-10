#include "PatternMatcher.h"
#include <cctype>

static std::string toLowerCopy(const std::string& text) {
    std::string result = text;
    for (int i = 0; i < (int)result.size(); i++) {
        result[i] = (char)std::tolower((unsigned char)result[i]);
    }
    return result;
}

std::vector<int> buildLps(const std::string& pattern) {
    int m = (int)pattern.size();
    std::vector<int> lps(m, 0);
    int len = 0;
    int i = 1;
    while (i < m) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else if (len > 0) {
            len = lps[len - 1];
        } else {
            lps[i] = 0;
            i++;
        }
    }
    return lps;
}

std::vector<int> kmpSearch(const std::string& text, const std::string& pattern) {
    std::vector<int> result;
    std::string t = toLowerCopy(text);
    std::string p = toLowerCopy(pattern);
    int n = (int)t.size();
    int m = (int)p.size();
    if (m == 0 || m > n) {
        return result;
    }

    std::vector<int> lps = buildLps(p);
    int i = 0;
    int j = 0;
    while (i < n) {
        if (t[i] == p[j]) {
            i++;
            j++;
            if (j == m) {
                result.push_back(i - m);
                j = lps[j - 1];
            }
        } else if (j > 0) {
            j = lps[j - 1];
        } else {
            i++;
        }
    }
    return result;
}

std::vector<int> rabinKarpSearch(const std::string& text, const std::string& pattern) {
    std::vector<int> result;
    std::string t = toLowerCopy(text);
    std::string p = toLowerCopy(pattern);
    int n = (int)t.size();
    int m = (int)p.size();
    if (m == 0 || m > n) {
        return result;
    }

    long long base = 256;
    long long prime = 1000000007;

    long long highPower = 1;
    for (int i = 0; i < m - 1; i++) {
        highPower = (highPower * base) % prime;
    }

    long long patternHash = 0;
    long long windowHash = 0;
    for (int i = 0; i < m; i++) {
        patternHash = (patternHash * base + (unsigned char)p[i]) % prime;
        windowHash = (windowHash * base + (unsigned char)t[i]) % prime;
    }

    for (int i = 0; i + m <= n; i++) {
        if (patternHash == windowHash) {
            bool same = true;
            for (int j = 0; j < m; j++) {
                if (t[i + j] != p[j]) {
                    same = false;
                    break;
                }
            }
            if (same) {
                result.push_back(i);
            }
        }
        if (i + m < n) {
            long long removed = ((unsigned char)t[i] * highPower) % prime;
            windowHash = (windowHash - removed + prime) % prime;
            windowHash = (windowHash * base + (unsigned char)t[i + m]) % prime;
        }
    }
    return result;
}

std::vector<LogEntry> searchLogs(const std::vector<LogEntry>& logs, const std::string& pattern, bool useKmp) {
    std::vector<LogEntry> result;
    if (pattern.empty()) {
        return result;
    }
    for (int i = 0; i < (int)logs.size(); i++) {
        std::vector<int> found;
        if (useKmp) {
            found = kmpSearch(logs[i].message, pattern);
        } else {
            found = rabinKarpSearch(logs[i].message, pattern);
        }
        if (!found.empty()) {
            result.push_back(logs[i]);
        }
    }
    return result;
}
