#ifndef CODE_VAULT_H
#define CODE_VAULT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Models.h"

// CodeVault: Version Control & Delta Storage Engine
class CodeVault {
private:
    std::map<std::string, CommitRecord> commitDatabase;
    std::vector<std::string> commitOrder;
    std::map<std::string, std::string> branchHeads; // branch name -> commitHash
    std::string currentBranch;
    std::string latestCodeContent;

    std::string computeHash(const std::string& input);
    std::vector<std::string> splitLines(const std::string& text);

public:
    CodeVault();

    // Create a new commit with delta storage against previous commit
    CommitRecord commit(const std::string& author, const std::string& message, const std::string& newContent);

    // Compute diff/delta between two arbitrary texts (Myers/Line-by-line diff)
    std::vector<LineDelta> computeDelta(const std::string& oldText, const std::string& newText);

    // Detect 3-way merge conflict between common ancestor, branch A, and branch B
    bool detectThreeWayMergeConflict(const std::string& ancestor, const std::string& local, const std::string& remote, std::string& mergedResult);

    // Get complete commit history
    std::vector<CommitRecord> getHistory() const;

    // Get specific commit details
    bool getCommit(const std::string& hash, CommitRecord& outCommit) const;

    // Calculate total repository storage stats (raw vs delta)
    void getStorageStats(long long& totalRawBytes, long long& totalDeltaBytes, double& savingsPercent) const;

    std::string getLatestCode() const { return latestCodeContent; }
};

#endif // CODE_VAULT_H
