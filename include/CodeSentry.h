#ifndef CODE_SENTRY_H
#define CODE_SENTRY_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "Models.h"

// Forward declarations
class FunctionNode;
class LoopNode;
class BranchNode;
class StatementNode;
class IASTVisitor;

// Base AST Node
class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual void accept(IASTVisitor& visitor) = 0;
};

// Visitor Pattern: AST Visitor Interface
class IASTVisitor {
public:
    virtual ~IASTVisitor() {}
    virtual void visit(FunctionNode& node) = 0;
    virtual void visit(LoopNode& node) = 0;
    virtual void visit(BranchNode& node) = 0;
    virtual void visit(StatementNode& node) = 0;
};

// Concrete AST Nodes
class FunctionNode : public ASTNode {
public:
    std::string name;
    int lineStart;
    int lineEnd;
    int parameterCount;
    std::vector<std::shared_ptr<ASTNode>> children;

    FunctionNode(const std::string& n, int s, int e, int params = 0)
        : name(n), lineStart(s), lineEnd(e), parameterCount(params) {}

    void accept(IASTVisitor& visitor) override {
        visitor.visit(*this);
        for (auto& child : children) child->accept(visitor);
    }
};

class LoopNode : public ASTNode {
public:
    std::string loopType; // "for", "while", "do"
    int line;
    int nestingLevel;
    std::vector<std::shared_ptr<ASTNode>> children;

    LoopNode(const std::string& t, int l, int nest = 1)
        : loopType(t), line(l), nestingLevel(nest) {}

    void accept(IASTVisitor& visitor) override {
        visitor.visit(*this);
        for (auto& child : children) child->accept(visitor);
    }
};

class BranchNode : public ASTNode {
public:
    std::string branchType; // "if", "else if", "switch", "case"
    int line;

    BranchNode(const std::string& t, int l) : branchType(t), line(l) {}

    void accept(IASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

class StatementNode : public ASTNode {
public:
    std::string content;
    int line;

    StatementNode(const std::string& c, int l) : content(c), line(l) {}

    void accept(IASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

// Concrete Visitor: Computes AST metrics
class MetricsCollectorVisitor : public IASTVisitor {
public:
    int totalStatements = 0;
    int totalFunctions = 0;
    int decisionPoints = 0; // if, for, while, case, etc.
    int maxNestingDepth = 0;
    int highParamFunctions = 0;

    void visit(FunctionNode& node) override {
        totalFunctions++;
        if (node.parameterCount > 4) highParamFunctions++;
    }

    void visit(LoopNode& node) override {
        decisionPoints++;
        if (node.nestingLevel > maxNestingDepth) {
            maxNestingDepth = node.nestingLevel;
        }
    }

    void visit(BranchNode& node) override {
        decisionPoints++;
    }

    void visit(StatementNode& node) override {
        totalStatements++;
    }
};

// Strategy Pattern: Rule Interface
class IAnalysisRule {
public:
    virtual ~IAnalysisRule() {}
    virtual std::string getRuleName() const = 0;
    virtual void evaluate(const std::string& code, const std::vector<std::string>& lines, 
                          const MetricsCollectorVisitor& metrics, std::vector<CodeIssue>& issues) = 0;
};

// Strategy 1: Cyclomatic Complexity Rule
class CyclomaticComplexityRule : public IAnalysisRule {
public:
    std::string getRuleName() const override { return "CyclomaticComplexityRule"; }
    void evaluate(const std::string& code, const std::vector<std::string>& lines, 
                  const MetricsCollectorVisitor& metrics, std::vector<CodeIssue>& issues) override;
};

// Strategy 2: Security Audit Rule (SQL Injection, buffer overflow, raw pointers)
class SecurityAuditRule : public IAnalysisRule {
public:
    std::string getRuleName() const override { return "SecurityAuditRule"; }
    void evaluate(const std::string& code, const std::vector<std::string>& lines, 
                  const MetricsCollectorVisitor& metrics, std::vector<CodeIssue>& issues) override;
};

// Strategy 3: Code Smell & Clean Code Rule (Nesting depth, long functions, magic numbers)
class CodeSmellRule : public IAnalysisRule {
public:
    std::string getRuleName() const override { return "CodeSmellRule"; }
    void evaluate(const std::string& code, const std::vector<std::string>& lines, 
                  const MetricsCollectorVisitor& metrics, std::vector<CodeIssue>& issues) override;
};

// Strategy 4: Style & Naming Rule
class StyleNamingRule : public IAnalysisRule {
public:
    std::string getRuleName() const override { return "StyleNamingRule"; }
    void evaluate(const std::string& code, const std::vector<std::string>& lines, 
                  const MetricsCollectorVisitor& metrics, std::vector<CodeIssue>& issues) override;
};

// Base Abstract Analyzer
class ICodeAnalyzer {
protected:
    std::vector<std::shared_ptr<IAnalysisRule>> rules;

public:
    virtual ~ICodeAnalyzer() {}
    virtual std::string getLanguage() const = 0;
    virtual QualityMetrics analyze(const std::string& sourceCode) = 0;

    void addRule(std::shared_ptr<IAnalysisRule> rule) {
        rules.push_back(rule);
    }
};

// Concrete C++ Analyzer
class CppCodeAnalyzer : public ICodeAnalyzer {
public:
    CppCodeAnalyzer();
    std::string getLanguage() const override { return "C++"; }
    QualityMetrics analyze(const std::string& sourceCode) override;
};

// Concrete Java Analyzer
class JavaCodeAnalyzer : public ICodeAnalyzer {
public:
    JavaCodeAnalyzer();
    std::string getLanguage() const override { return "Java"; }
    QualityMetrics analyze(const std::string& sourceCode) override;
};

// Concrete C Analyzer
class CCodeAnalyzer : public ICodeAnalyzer {
public:
    CCodeAnalyzer();
    std::string getLanguage() const override { return "C"; }
    QualityMetrics analyze(const std::string& sourceCode) override;
};

// Factory Pattern: Analyzer Factory
class CodeAnalyzerFactory {
public:
    static std::unique_ptr<ICodeAnalyzer> createAnalyzer(const std::string& language);
};

#endif // CODE_SENTRY_H
