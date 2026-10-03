#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <map>

// Data Model: Submission & Code Quality Issue
struct CodeIssue {
    std::string type;       // "SECURITY", "COMPLEXITY", "SMELL", "STYLE"
    std::string ruleName;
    std::string message;
    int line;
    std::string severity;   // "HIGH", "MEDIUM", "LOW"
};

// Data Model: Static Analysis Metrics
struct QualityMetrics {
    int totalLines;
    int codeLines;
    int commentLines;
    int blankLines;
    int functionCount;
    int cyclomaticComplexity;
    double maintainabilityIndex;
    int securityFlawsCount;
    int styleViolationsCount;
    std::vector<CodeIssue> issues;
};

// Data Model: Empirical DSA Profiling Point
struct ProfileDataPoint {
    int inputSize;
    long long comparisons;
    long long swaps;
    double executionTimeMicroseconds;
};

// Data Model: AlgoPro Profiling Result
struct AlgorithmProfile {
    std::string algorithmName;
    std::string empiricalComplexity; // "O(1)", "O(log N)", "O(N)", "O(N log N)", "O(N^2)"
    double rSquaredFit;              // Regression fit confidence
    std::vector<ProfileDataPoint> dataPoints;
};

// Data Model: Version Control Delta
struct LineDelta {
    char operation; // '+', '-', ' '
    int lineNumber;
    std::string content;
};

struct CommitRecord {
    std::string commitHash;
    std::string author;
    std::string message;
    std::string timestamp;
    std::string parentHash;
    int linesAdded;
    int linesDeleted;
    int rawSizeBytes;
    int deltaSizeBytes;
    double storageSavingsPercent;
    std::vector<LineDelta> deltas;
};

// Data Model: Multi-Dimensional Evaluation Breakdown
struct ScoreBreakdown {
    double functionalityScore; // 30%
    double codeQualityScore;   // 15%
    double performanceScore;   // 10%
    double databaseScore;      // 10%
    double documentationScore; // 10%
    double securityScore;      // 5%
    double innovationScore;    // 10% (Judge)
    double uiUxScore;          // 5%  (Judge)
    double presentationScore;  // 5%  (Judge)
    double compositeTotal;     // 100%
};

// Data Model: Leaderboard Entry for Max-Heap
struct LeaderboardEntry {
    int rank;
    std::string teamId;
    std::string teamName;
    std::string projectName;
    std::string members;
    ScoreBreakdown scores;
    std::string submissionTime;
    std::string latestCommitHash;

    // Operator for Heap ordering (Max-Heap by compositeTotal)
    bool operator<(const LeaderboardEntry& other) const {
        return scores.compositeTotal < other.scores.compositeTotal;
    }
};

#endif // MODELS_H
