#pragma once
#include <string>
#include <vector>
#include <functional>
#include <iostream>

class TestCase {
public:
    TestCase(const std::string& name) : _name(name), _passed(false) {}

    virtual void setUp() {}
    virtual void tearDown() {}
    virtual void test() = 0;

    const std::string& getName() const { return _name; }
    bool isPassed() const { return _passed; }
    const std::string& getError() const { return _error; }

    void run() {
        try {
            setUp();
            test();
            _passed = true;
        } catch (const std::exception& e) {
            _error = e.what();
            _passed = false;
        }
        tearDown();
    }

protected:
    void assert_true(bool condition, const std::string& msg) {
        if (!condition) throw std::runtime_error("ASSERT_TRUE failed: " + msg);
    }

    void assert_equal(const std::string& expected, const std::string& actual) {
        if (expected != actual) {
            throw std::runtime_error("ASSERT_EQUAL failed: expected='" + expected + "', actual='" + actual + "'");
        }
    }

    void assert_not_empty(const std::string& str, const std::string& msg) {
        if (str.empty()) throw std::runtime_error("ASSERT_NOT_EMPTY failed: " + msg);
    }

private:
    std::string _name;
    bool _passed;
    std::string _error;
};

class TestSuite {
public:
    static TestSuite& get();

    void addTest(TestCase* test) { _tests.push_back(test); }

    int runAll() {
        int passed = 0, failed = 0;
        std::cout << "\n[测试套件] 运行 " << _tests.size() << " 个测试\n";
        for (auto test : _tests) {
            test->run();
            if (test->isPassed()) {
                std::cout << "  ✓ " << test->getName() << "\n";
                passed++;
            } else {
                std::cout << "  ✗ " << test->getName() << " - " << test->getError() << "\n";
                failed++;
            }
        }
        std::cout << "\n[结果] " << passed << " passed, " << failed << " failed\n";
        return failed;
    }

private:
    TestSuite() = default;
    std::vector<TestCase*> _tests;
};
