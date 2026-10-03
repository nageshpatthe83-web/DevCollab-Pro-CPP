#include "../include/CodeVault.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

CodeVault::CodeVault() : currentBranch("main"), latestCodeContent("") {
    // Initial sample seed commit
    commit("System", "Initial repository setup with sample C++ evaluation template", 
           "#include <iostream>\n\nint main() {\n    std::cout << \"DevCollab Pro Initialized\" << std::endl;\n    return 0;\n}\n");
}

std::string CodeVault::computeHash(const std::string& input) {
    // 64-bit FNV-1a hash formatted as 16-char hex string
    unsigned long long hash = 14695981039346656037ULL;
    for (char c : input) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ULL;
    }
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return ss.str();
}

std::vector<std::string> CodeVault::splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}

std::vector<LineDelta> CodeVault::computeDelta(const std::string& oldText, const std::string& newText) {
    std::vector<std::string> oldLines = splitLines(oldText);
    std::vector<std::string> newLines = splitLines(newText);
    std::vector<LineDelta> deltas;

    size_t i = 0, j = 0;
    int lineNum = 1;

    // Line-by-line diff algorithm
    while (i < oldLines.size() || j < newLines.size()) {
        if (i < oldLines.size() && j < newLines.size()) {
            if (oldLines[i] == newLines[j]) {
                deltas.push_back({' ', lineNum++, oldLines[i]});
                i++;
                j++;
            } else {
                // Check if old line was deleted
                bool foundNewInOld = false;
                for (size_t lookahead = j + 1; lookahead < std::min(newLines.size(), j + 5); ++lookahead) {
                    if (oldLines[i] == newLines[lookahead]) {
                        foundNewInOld = true;
                        break;
                    }
                }

                if (foundNewInOld) {
                    // Lines were added in new
                    deltas.push_back({'+', lineNum++, newLines[j]});
                    j++;
                } else {
                    // Old line was deleted or replaced
                    deltas.push_back({'-', lineNum++, oldLines[i]});
                    i++;
                    if (j < newLines.size()) {
                        deltas.push_back({'+', lineNum++, newLines[j]});
                        j++;
                    }
                }
            }
        } else if (i < oldLines.size()) {
            // Remaining old lines were deleted
            deltas.push_back({'-', lineNum++, oldLines[i++]});
        } else {
            // Remaining new lines were added
            deltas.push_back({'+', lineNum++, newLines[j++]});
        }
    }

    return deltas;
}

CommitRecord CodeVault::commit(const std::string& author, const std::string& message, const std::string& newContent) {
    CommitRecord rec;
    rec.author = author;
    rec.message = message;
    rec.parentHash = commitOrder.empty() ? "" : commitOrder.back();

    // Get current ISO-like timestamp
    std::time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    rec.timestamp = std::string(buf);

    rec.commitHash = computeHash(author + message + rec.timestamp + newContent);
    rec.rawSizeBytes = static_cast<int>(newContent.size());

    rec.linesAdded = 0;
    rec.linesDeleted = 0;

    if (latestCodeContent.empty()) {
        // First commit
        std::vector<std::string> lines = splitLines(newContent);
        int lnum = 1;
        for (const auto& l : lines) {
            rec.deltas.push_back({'+', lnum++, l});
            rec.linesAdded++;
        }
        rec.deltaSizeBytes = rec.rawSizeBytes;
        rec.storageSavingsPercent = 0.0;
    } else {
        rec.deltas = computeDelta(latestCodeContent, newContent);
        int deltaSize = 0;
        for (const auto& d : rec.deltas) {
            if (d.operation == '+') {
                rec.linesAdded++;
                deltaSize += static_cast<int>(d.content.size()) + 4;
            } else if (d.operation == '-') {
                rec.linesDeleted++;
                deltaSize += static_cast<int>(d.content.size()) + 4;
            }
        }
        rec.deltaSizeBytes = std::max(deltaSize, 12);
        if (rec.rawSizeBytes > 0) {
            rec.storageSavingsPercent = (1.0 - (static_cast<double>(rec.deltaSizeBytes) / rec.rawSizeBytes)) * 100.0;
            if (rec.storageSavingsPercent < 0) rec.storageSavingsPercent = 0;
        } else {
            rec.storageSavingsPercent = 0.0;
        }
    }

    latestCodeContent = newContent;
    commitDatabase[rec.commitHash] = rec;
    commitOrder.push_back(rec.commitHash);
    branchHeads[currentBranch] = rec.commitHash;

    return rec;
}

bool CodeVault::detectThreeWayMergeConflict(const std::string& ancestor, const std::string& local, const std::string& remote, std::string& mergedResult) {
    std::vector<std::string> ancLines = splitLines(ancestor);
    std::vector<std::string> locLines = splitLines(local);
    std::vector<std::string> remLines = splitLines(remote);

    bool conflictFound = false;
    std::stringstream out;

    size_t maxLen = std::max({ancLines.size(), locLines.size(), remLines.size()});
    for (size_t i = 0; i < maxLen; ++i) {
        std::string a = (i < ancLines.size()) ? ancLines[i] : "";
        std::string l = (i < locLines.size()) ? locLines[i] : "";
        std::string r = (i < remLines.size()) ? remLines[i] : "";

        if (l == r) {
            out << l << "\n";
        } else if (l == a) {
            // Local didn't change, remote changed
            out << r << "\n";
        } else if (r == a) {
            // Remote didn't change, local changed
            out << l << "\n";
        } else {
            // Both local and remote changed differently -> MERGE CONFLICT!
            conflictFound = true;
            out << "<<<<<<< HEAD (Local)\n" << l << "\n=======\n" << r << "\n>>>>>>> Remote\n";
        }
    }

    mergedResult = out.str();
    return conflictFound;
}

std::vector<CommitRecord> CodeVault::getHistory() const {
    std::vector<CommitRecord> list;
    for (auto it = commitOrder.rbegin(); it != commitOrder.rend(); ++it) {
        auto mapIt = commitDatabase.find(*it);
        if (mapIt != commitDatabase.end()) {
            list.push_back(mapIt->second);
        }
    }
    return list;
}

bool CodeVault::getCommit(const std::string& hash, CommitRecord& outCommit) const {
    auto it = commitDatabase.find(hash);
    if (it != commitDatabase.end()) {
        outCommit = it->second;
        return true;
    }
    return false;
}

void CodeVault::getStorageStats(long long& totalRawBytes, long long& totalDeltaBytes, double& savingsPercent) const {
    totalRawBytes = 0;
    totalDeltaBytes = 0;
    for (const auto& pair : commitDatabase) {
        totalRawBytes += pair.second.rawSizeBytes;
        totalDeltaBytes += pair.second.deltaSizeBytes;
    }
    if (totalRawBytes > 0) {
        savingsPercent = (1.0 - (static_cast<double>(totalDeltaBytes) / totalRawBytes)) * 100.0;
    } else {
        savingsPercent = 0.0;
    }
}
