#include <cstdint>
#include <queue>
#include <string>
#include <vector>

namespace SearchLogX {
enum class Severity : int {
    emergency = 0,
    alert     = 1,
    critical  = 2,
    error     = 3,
    warning   = 4,
    notice    = 5,
    info      = 6,
    debug     = 7,
    unknown   = 8
};

// Represents a single log entry containing its severity level and message.
// Severity defaults to Unknown if not specified, which has the lowest
// priority in the priority queue.
struct LogEntry {
    std::string timestamp = "";
    Severity severity = Severity::unknown;
    std::string message = "";
};


// Helper functions (readable names <-> enum).
std::string severityToString(Severity s);                 // e.g. Severity::Error -> "ERROR"
Severity    severityFromString(const std::string& name);  // case-insensitive; bad input -> Unknown
Severity    severityFromInt(int value);                   // outside 0..7 -> Unknown

class LogPriorityQueue {
    struct Item {
        LogEntry entry;
        std::uint64_t seq;
    };

    // Returns true when 'a' has LOWER priority than 'b'
    // (std::priority_queue keeps the "largest" element on top).
    struct LowerPriority {
        bool operator()(const Item& a, const Item& b) const;
    };

    std::priority_queue<Item, std::vector<Item>, LowerPriority> queue_;
    std::uint64_t nextSeq_ = 0;
        
    public:
        void push(const std::string& timestamp, const std::string& level,  const std::string& message);

        // Returns false if empty; otherwise copies the highest-priority entry.
        bool top(LogEntry& entry) const;

        // Remove the highest-priority entry. Returns false if the queue was empty.
        bool pop();

        // Checks whether the priority queue is empty.
        bool empty() const;

        size_t size() const;
    
};
}