// main(test).cpp - tests for SearchLogX::LogPriorityQueue
// Uses only the public interface (no private members).

#include "priorityQueue.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace SearchLogX;

static int g_checks   = 0;
static int g_failures = 0;

#define CHECK(cond)                                                      \
    do {                                                                 \
        ++g_checks;                                                      \
        if (!(cond)) {                                                   \
            ++g_failures;                                                \
            std::cout << "    FAILED (line " << __LINE__ << "): "        \
                      << #cond << "\n";                                  \
        }                                                                \
    } while (0)

// Runs one test function and prints PASS/FAIL for it.
static void run(const char* name, void (*fn)()) {
    int before = g_failures;
    fn();
    std::cout << (g_failures == before ? "[PASS] " : "[FAIL] ")
              << name << "\n";
}

// Reads top(), checks its message, then pops. Returns true if all OK.
static bool nextIs(LogPriorityQueue& q, const std::string& expectedMsg) {
    LogEntry e;
    if (!q.top(e)) return false;
    bool ok = (e.message == expectedMsg);
    if (!q.pop()) return false;
    return ok;
}

// ---------------------------------------------------------------------

static void testEmptyQueue() {
    LogPriorityQueue q;
    CHECK(q.empty());
    CHECK(q.size() == 0);

    LogEntry e;
    e.timestamp = "sentinel-ts";
    e.severity  = Severity::notice;
    e.message   = "sentinel-msg";

    CHECK(!q.top(e));                       // must report failure
    CHECK(e.timestamp == "sentinel-ts");    // and leave entry untouched
    CHECK(e.severity == Severity::notice);
    CHECK(e.message == "sentinel-msg");

    CHECK(!q.pop());                        // pop on empty -> false
    CHECK(q.empty());
    CHECK(q.size() == 0);
}

static void testInsertMultiple() {
    LogPriorityQueue q;
    q.push("t1", "info", "a");
    CHECK(q.size() == 1);
    q.push("t2", "error", "b");
    q.push("t3", "debug", "c");
    CHECK(q.size() == 3);
    CHECK(!q.empty());
}

static void testPriorityOrdering() {
    LogPriorityQueue q;
    q.push("t", "info",      "info");
    q.push("t", "error",     "error");
    q.push("t", "emergency", "emergency");
    q.push("t", "debug",     "debug");
    q.push("t", "warning",   "warning");
    q.push("t", "critical",  "critical");
    q.push("t", "alert",     "alert");
    q.push("t", "notice",    "notice");

    CHECK(nextIs(q, "emergency"));
    CHECK(nextIs(q, "alert"));
    CHECK(nextIs(q, "critical"));
    CHECK(nextIs(q, "error"));
    CHECK(nextIs(q, "warning"));
    CHECK(nextIs(q, "notice"));
    CHECK(nextIs(q, "info"));
    CHECK(nextIs(q, "debug"));
    CHECK(q.empty());
}

static void testFifoSameSeverity() {
    LogPriorityQueue q;
    q.push("t1", "error", "first");
    q.push("t2", "error", "second");
    q.push("t3", "error", "third");
    q.push("t4", "error", "fourth");
    q.push("t5", "error", "fifth");

    CHECK(nextIs(q, "first"));
    CHECK(nextIs(q, "second"));
    CHECK(nextIs(q, "third"));
    CHECK(nextIs(q, "fourth"));
    CHECK(nextIs(q, "fifth"));
    CHECK(q.empty());
}

static void testMixedPriorityAndFifo() {
    LogPriorityQueue q;
    q.push("t1", "info",      "info-A");
    q.push("t2", "error",     "error-B");
    q.push("t3", "info",      "info-C");
    q.push("t4", "error",     "error-D");
    q.push("t5", "emergency", "emergency-E");
    q.push("t6", "info",      "info-F");

    CHECK(nextIs(q, "emergency-E"));
    CHECK(nextIs(q, "error-B"));
    CHECK(nextIs(q, "error-D"));
    CHECK(nextIs(q, "info-A"));
    CHECK(nextIs(q, "info-C"));
    CHECK(nextIs(q, "info-F"));
    CHECK(q.empty());
}

static void testCaseInsensitive() {
    CHECK(severityFromString("ERROR") == Severity::error);
    CHECK(severityFromString("error") == Severity::error);
    CHECK(severityFromString("ErRoR") == Severity::error);
    CHECK(severityFromString("eMeRgEnCy") == Severity::emergency);
    CHECK(severityFromString("Debug") == Severity::debug);

    // Through push(): all three are stored as the same severity,
    // so they must come out in insertion order.
    LogPriorityQueue q;
    q.push("t1", "ERROR", "upper");
    q.push("t2", "error", "lower");
    q.push("t3", "Error", "mixed");

    LogEntry e;
    CHECK(q.top(e));
    CHECK(e.severity == Severity::error);

    CHECK(nextIs(q, "upper"));
    CHECK(nextIs(q, "lower"));
    CHECK(nextIs(q, "mixed"));
}

static void testUnknownSeverity() {
    CHECK(severityFromString("banana") == Severity::unknown);
    CHECK(severityFromString("") == Severity::unknown);
    CHECK(severityFromString("7") == Severity::unknown);
    CHECK(severityFromString("errorr") == Severity::unknown);
    CHECK(severityFromString(" error") == Severity::unknown); // no trimming

    // Unknown entries are stored and have the LOWEST priority.
    LogPriorityQueue q;
    q.push("t1", "banana", "unknown-1");
    q.push("t2", "debug",  "debug");
    q.push("t3", "???",    "unknown-2");
    q.push("t4", "info",   "info");

    CHECK(q.size() == 4);
    CHECK(nextIs(q, "info"));
    CHECK(nextIs(q, "debug"));

    LogEntry e;
    CHECK(q.top(e));
    CHECK(e.severity == Severity::unknown);
    CHECK(nextIs(q, "unknown-1"));   // FIFO among unknowns
    CHECK(nextIs(q, "unknown-2"));
    CHECK(q.empty());
}

static void testTopDoesNotRemove() {
    LogPriorityQueue q;
    q.push("t1", "warning", "w");
    q.push("t2", "alert",   "a");

    LogEntry e1, e2;
    CHECK(q.top(e1));
    CHECK(q.top(e2));
    CHECK(q.size() == 2);                 // unchanged
    CHECK(e1.message == "a");
    CHECK(e2.message == "a");
    CHECK(e1.timestamp == e2.timestamp);
    CHECK(e1.severity == e2.severity);
}

static void testPopRemoves() {
    LogPriorityQueue q;
    q.push("t1", "warning", "w");
    q.push("t2", "alert",   "a");
    q.push("t3", "debug",   "d");

    CHECK(q.size() == 3);
    CHECK(q.pop());
    CHECK(q.size() == 2);

    LogEntry e;
    CHECK(q.top(e));
    CHECK(e.message == "w");              // "a" is gone

    CHECK(q.pop());
    CHECK(q.size() == 1);
    CHECK(q.pop());
    CHECK(q.size() == 0);
    CHECK(q.empty());
    CHECK(!q.pop());                      // extra pop is safe
}

static void testPreservesData() {
    LogPriorityQueue q;
    q.push("2026-10-22T08:15:30Z", "WARNING",
           "Disk almost full: /dev/sda1 (95%)");

    LogEntry e;
    CHECK(q.top(e));
    CHECK(e.timestamp == "2026-10-22T08:15:30Z");
    CHECK(e.severity == Severity::warning);
    CHECK(e.message == "Disk almost full: /dev/sda1 (95%)");
    CHECK(q.pop());

    // Empty message and multi-line / special-character message.
    q.push("ts-empty", "info", "");
    q.push("ts-special", "info", "line1\nline2\t\"quoted\"");

    CHECK(q.top(e));
    CHECK(e.timestamp == "ts-empty");
    CHECK(e.message == "");
    CHECK(q.pop());

    CHECK(q.top(e));
    CHECK(e.timestamp == "ts-special");
    CHECK(e.message == "line1\nline2\t\"quoted\"");
}

static void testHelpers() {
    CHECK(severityToString(Severity::emergency) == "EMERGENCY");
    CHECK(severityToString(Severity::alert)     == "ALERT");
    CHECK(severityToString(Severity::critical)  == "CRITICAL");
    CHECK(severityToString(Severity::error)     == "ERROR");
    CHECK(severityToString(Severity::warning)   == "WARNING");
    CHECK(severityToString(Severity::notice)    == "NOTICE");
    CHECK(severityToString(Severity::info)      == "INFO");
    CHECK(severityToString(Severity::debug)     == "DEBUG");
    CHECK(severityToString(Severity::unknown)   == "UNKNOWN");

    // Round trip for every valid level.
    for (int i = 0; i <= 7; ++i) {
        Severity s = severityFromInt(i);
        CHECK(static_cast<int>(s) == i);
        CHECK(severityFromString(severityToString(s)) == s);
    }

    CHECK(severityFromInt(-1)  == Severity::unknown);
    CHECK(severityFromInt(8)   == Severity::unknown);
    CHECK(severityFromInt(100) == Severity::unknown);
    CHECK(severityFromString("UNKNOWN") == Severity::unknown);
}

static void testStress() {
    const char* names[] = {"emergency", "alert", "critical", "error",
                           "warning", "notice", "info", "debug"};
    struct Rec { int sev; std::string msg; };

    std::vector<Rec> recs;
    LogPriorityQueue q;
    std::uint32_t x = 12345;

    for (int i = 0; i < 1000; ++i) {
        x = x * 1103515245u + 12345u;            // simple pseudo-random
        int sev = static_cast<int>((x >> 16) % 8);
        std::string msg = "m" + std::to_string(i);
        recs.push_back({sev, msg});
        q.push("t" + std::to_string(i), names[sev], msg);
    }
    CHECK(q.size() == 1000);

    // stable_sort keeps insertion order for equal severity = expected result.
    std::stable_sort(recs.begin(), recs.end(),
                     [](const Rec& a, const Rec& b) { return a.sev < b.sev; });

    bool ok = true;
    for (const Rec& r : recs) {
        LogEntry e;
        if (!q.top(e) || e.message != r.msg ||
            static_cast<int>(e.severity) != r.sev) {
            ok = false;
            break;
        }
        q.pop();
    }
    CHECK(ok);
    CHECK(q.empty());
}

static void testDrainThenReuse() {
    LogPriorityQueue q;
    q.push("t1", "info", "x");
    CHECK(q.pop());
    CHECK(q.empty());
    CHECK(!q.pop());

    LogEntry e;
    CHECK(!q.top(e));

    // Queue still works after being emptied.
    q.push("t2", "alert", "y");
    CHECK(q.size() == 1);
    CHECK(q.top(e));
    CHECK(e.message == "y");
}

// ---------------------------------------------------------------------

int main() {
    run("Empty queue behavior",                 testEmptyQueue);
    run("Insert multiple entries",              testInsertMultiple);
    run("Priority ordering",                    testPriorityOrdering);
    run("FIFO for equal severity",              testFifoSameSeverity);
    run("Mixed priority + FIFO",                testMixedPriorityAndFifo);
    run("Case-insensitive severity",            testCaseInsensitive);
    run("Unknown / invalid severity strings",   testUnknownSeverity);
    run("top() does not remove",                testTopDoesNotRemove);
    run("pop() removes",                        testPopRemoves);
    run("Timestamps and messages preserved",    testPreservesData);
    run("Helper functions",                     testHelpers);
    run("Stress test (1000 entries)",           testStress);
    run("Drain then reuse",                     testDrainThenReuse);

    std::cout << "\n" << (g_checks - g_failures) << " / " << g_checks
              << " checks passed.\n";
    if (g_failures == 0) {
        std::cout << "ALL TESTS PASSED\n";
        return 0;
    }
    std::cout << g_failures << " CHECK(S) FAILED\n";
    return 1;
}