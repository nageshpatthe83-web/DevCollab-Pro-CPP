#include "../include/CodeSentry.h"
#include <sstream>
#include <regex>
#include <algorithm>
#include <cmath>

// Strategy 1: Cyclomatic Complexity Rule
void CyclomaticComplexityRule::evaluate(const std::string& code, const std::vector<std::string>& lines, 
                                       const MetricsCollectorVisitor& metrics, std::vector<CodeIssue>& issues) {
    int complexity = metrics.decisionPoints + 1;
    if (complexity > 10) {
        CodeIssue issue;
        issue.type = "COMPLEXITY";
        issue.ruleName = getRuleName();
        issue.severity = (complexity > 15) ? "HIGH" : "MEDIUM";
        issue.line = 1;
        issue.message = "High Cyclomatic Complexity (" + std::to_string(complexity) + 
                        "). Consider refactoring complex conditionals and breaking logic into helper functions.";
        issues.push_back(issue);
    }

    if (metrics.maxNestingDepth >= 3) {
        CodeIssue issue;
        issue.type = "COMPLEXITY";
        issue.ruleName = getRuleName();
        issue.severity = "MEDIUM";
        issue.line = 1;
        issue.message = "Excessive control flow nesting (depth " + std::to_string(metrics.maxNestingDepth) + 
                        "). Deep nesting degrades readability and increases bug probability.";
        issues.push_back(issue);
    }
}

// Strategy 2: Security Audit Rule
void SecurityAuditRule::evaluate(const std::string& code, const std::vector<std::string>& lines, 
                                const MetricsCollectorVisitor& metrics, std::vector<CodeIssue>& issues) {
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& l = lines[i];

        // SQL Injection pattern check
        if ((l.find("SELECT") != std::string::npos || l.find("INSERT") != std::string::npos || l.find("UPDATE") != std::string::npos) &&
            (l.find("+") != std::string::npos || l.find("%s") != std::string::npos || l.find("$") != std::string::npos)) {
            CodeIssue issue;
            issue.type = "SECURITY";
            issue.ruleName = getRuleName();
            issue.severity = "HIGH";
            issue.line = static_cast<int>(i + 1);
            issue.message = "Potential SQL Injection: Dynamic SQL concatenation detected. Use prepared statements and parameterized queries.";
            issues.push_back(issue);
        }

        // Buffer Overflow / Unsafe C-string functions
        if (l.find("strcpy(") != std::string::npos || l.find("gets(") != std::string::npos || l.find("sprintf(") != std::string::npos) {
            CodeIssue issue;
            issue.type = "SECURITY";
            issue.ruleName = getRuleName();
            issue.severity = "HIGH";
            issue.line = static_cast<int>(i + 1);
            issue.message = "Unsafe C-standard function detected (strcpy/gets/sprintf). Vulnerable to buffer overflows. Use std::string or strncpy_s/snprintf.";
            issues.push_back(issue);
        }

        // Raw Pointer Memory Leak risk
        if (l.find("new ") != std::string::npos && code.find("delete ") == std::string::npos && code.find("unique_ptr") == std::string::npos) {
            CodeIssue issue;
            issue.type = "SECURITY";
            issue.ruleName = getRuleName();
            issue.severity = "MEDIUM";
            issue.line = static_cast<int>(i + 1);
            issue.message = "Raw pointer allocation ('new') without corresponding 'delete' or smart pointer RAII (std::unique_ptr). Risk of memory leak.";
            issues.push_back(issue);
        }
    }
}

// Strategy 3: Code Smell & Clean Code Rule
void CodeSmellRule::evaluate(const std::string& code, const std::vector<std::string>& lines, 
                            const MetricsCollectorVisitor& metrics, std::vector<CodeIssue>& issues) {
    if (metrics.highParamFunctions > 0) {
        CodeIssue issue;
        issue.type = "SMELL";
        issue.ruleName = getRuleName();
        issue.severity = "MEDIUM";
        issue.line = 1;
        issue.message = "Method signature has more than 4 parameters. Consider encapsulating parameters into a configuration struct or class.";
        issues.push_back(issue);
    }

    // Check for hardcoded magic numbers
    std::regex magicNumRegex("(?!(?:0|1|2|10|100)\\b)\\b\\d{3,}\\b");
    for (size_t i = 0; i < lines.size(); ++i) {
        if (lines[i].find("//") != std::string::npos || lines[i].find("#define") != std::string::npos || lines[i].find("const") != std::string::npos) {
            continue;
        }
        if (std::regex_search(lines[i], magicNumRegex)) {
            CodeIssue issue;
            issue.type = "SMELL";
            issue.ruleName = getRuleName();
            issue.severity = "LOW";
            issue.line = static_cast<int>(i + 1);
            issue.message = "Hardcoded magic number literal detected. Extract into a named 'constexpr' or 'const' variable.";
            issues.push_back(issue);
            break; // limit to one notice
        }
    }
}

// Strategy 4: Style & Naming Rule
void StyleNamingRule::evaluate(const std::string& code, const std::vector<std::string>& lines, 
                              const MetricsCollectorVisitor& metrics, std::vector<CodeIssue>& issues) {
    if (lines.size() > 200) {
        CodeIssue issue;
        issue.type = "STYLE";
        issue.ruleName = getRuleName();
        issue.severity = "LOW";
        issue.line = 1;
        issue.message = "Large file length (" + std::to_string(lines.size()) + " lines). Consider decomposing into smaller modular header and source units.";
        issues.push_back(issue);
    }
}

// Base Analysis logic
static QualityMetrics runAnalysisPipeline(const std::string& sourceCode, const std::vector<std::shared_ptr<IAnalysisRule>>& rules) {
    QualityMetrics metrics;
    metrics.totalLines = 0;
    metrics.codeLines = 0;
    metrics.commentLines = 0;
    metrics.blankLines = 0;
    metrics.functionCount = 0;
    metrics.securityFlawsCount = 0;
    metrics.styleViolationsCount = 0;

    std::vector<std::string> lines;
    std::stringstream ss(sourceCode);
    std::string line;

    bool inMultiComment = false;
    while (std::getline(ss, line)) {
        metrics.totalLines++;
        // Trim whitespace
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
        trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

        if (trimmed.empty()) {
            metrics.blankLines++;
            lines.push_back(line);
            continue;
        }

        if (inMultiComment) {
            metrics.commentLines++;
            if (trimmed.find("*/") != std::string::npos) {
                inMultiComment = false;
            }
            lines.push_back(line);
            continue;
        }

        if (trimmed.rfind("/*", 0) == 0) {
            metrics.commentLines++;
            if (trimmed.find("*/") == std::string::npos) {
                inMultiComment = true;
            }
            lines.push_back(line);
            continue;
        }

        if (trimmed.rfind("//", 0) == 0) {
            metrics.commentLines++;
            lines.push_back(line);
            continue;
        }

        metrics.codeLines++;
        lines.push_back(line);
    }

    // Build Mock AST and run MetricsCollectorVisitor (Visitor Pattern)
    MetricsCollectorVisitor visitor;
    int currentNesting = 0;

    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& l = lines[i];

        if (l.find("for (") != std::string::npos || l.find("for(") != std::string::npos ||
            l.find("while (") != std::string::npos || l.find("while(") != std::string::npos) {
            currentNesting++;
            LoopNode loopNode("loop", static_cast<int>(i + 1), currentNesting);
            loopNode.accept(visitor);
        }

        if (l.find("if (") != std::string::npos || l.find("if(") != std::string::npos ||
            l.find("else if") != std::string::npos || l.find("case ") != std::string::npos) {
            BranchNode branchNode("branch", static_cast<int>(i + 1));
            branchNode.accept(visitor);
        }

        if ((l.find("void ") != std::string::npos || l.find("int ") != std::string::npos || 
             l.find("bool ") != std::string::npos || l.find("double ") != std::string::npos) &&
            l.find("(") != std::string::npos && l.find(")") != std::string::npos && l.find(";") == std::string::npos) {
            // Count commas as parameter estimate
            size_t pstart = l.find("(");
            size_t pend = l.find(")");
            int params = 0;
            if (pend > pstart + 1) {
                std::string argStr = l.substr(pstart + 1, pend - pstart - 1);
                params = 1 + static_cast<int>(std::count(argStr.begin(), argStr.end(), ','));
            }
            FunctionNode fnNode("function", static_cast<int>(i + 1), static_cast<int>(i + 1), params);
            fnNode.accept(visitor);
        }

        if (l.find("}") != std::string::npos && currentNesting > 0) {
            currentNesting--;
        }
    }

    metrics.functionCount = std::max(1, visitor.totalFunctions);
    metrics.cyclomaticComplexity = visitor.decisionPoints + 1;

    // Evaluate Strategy Rules
    for (const auto& rule : rules) {
        rule->evaluate(sourceCode, lines, visitor, metrics.issues);
    }

    for (const auto& issue : metrics.issues) {
        if (issue.type == "SECURITY") metrics.securityFlawsCount++;
        if (issue.type == "STYLE") metrics.styleViolationsCount++;
    }

    // Maintainability Index (MI formula approximation)
    // MI = 171 - 5.2 * ln(Halstead V) - 0.23 * CC - 16.2 * ln(LOC) + 50 * sin(sqrt(2.4 * perCM))
    double volume = std::max(10.0, static_cast<double>(metrics.codeLines * 8.0));
    double loc = std::max(5.0, static_cast<double>(metrics.codeLines));
    double cc = static_cast<double>(metrics.cyclomaticComplexity);
    double commentRatio = (metrics.totalLines > 0) ? (static_cast<double>(metrics.commentLines) / metrics.totalLines) : 0.0;

    double mi = 171.0 - 5.2 * std::log(volume) - 0.23 * cc - 16.2 * std::log(loc) + (commentRatio * 30.0);
    if (mi < 0.0) mi = 0.0;
    if (mi > 100.0) mi = 100.0;
    metrics.maintainabilityIndex = std::round(mi * 10.0) / 10.0;

    return metrics;
}

// CppCodeAnalyzer
CppCodeAnalyzer::CppCodeAnalyzer() {
    addRule(std::make_shared<CyclomaticComplexityRule>());
    addRule(std::make_shared<SecurityAuditRule>());
    addRule(std::make_shared<CodeSmellRule>());
    addRule(std::make_shared<StyleNamingRule>());
}

QualityMetrics CppCodeAnalyzer::analyze(const std::string& sourceCode) {
    return runAnalysisPipeline(sourceCode, rules);
}

// JavaCodeAnalyzer
JavaCodeAnalyzer::JavaCodeAnalyzer() {
    addRule(std::make_shared<CyclomaticComplexityRule>());
    addRule(std::make_shared<SecurityAuditRule>());
    addRule(std::make_shared<CodeSmellRule>());
    addRule(std::make_shared<StyleNamingRule>());
}

QualityMetrics JavaCodeAnalyzer::analyze(const std::string& sourceCode) {
    return runAnalysisPipeline(sourceCode, rules);
}

// CCodeAnalyzer
CCodeAnalyzer::CCodeAnalyzer() {
    addRule(std::make_shared<CyclomaticComplexityRule>());
    addRule(std::make_shared<SecurityAuditRule>());
    addRule(std::make_shared<CodeSmellRule>());
    addRule(std::make_shared<StyleNamingRule>());
}

QualityMetrics CCodeAnalyzer::analyze(const std::string& sourceCode) {
    return runAnalysisPipeline(sourceCode, rules);
}

// Factory Pattern: Analyzer Factory
std::unique_ptr<ICodeAnalyzer> CodeAnalyzerFactory::createAnalyzer(const std::string& language) {
    std::string lowerLang = language;
    std::transform(lowerLang.begin(), lowerLang.end(), lowerLang.begin(), ::tolower);

    if (lowerLang == "java") {
        return std::unique_ptr<ICodeAnalyzer>(new JavaCodeAnalyzer());
    } else if (lowerLang == "c") {
        return std::unique_ptr<ICodeAnalyzer>(new CCodeAnalyzer());
    } else {
        // Default to C++
        return std::unique_ptr<ICodeAnalyzer>(new CppCodeAnalyzer());
    }
}
