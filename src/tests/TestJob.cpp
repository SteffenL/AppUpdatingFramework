#include "core/job/Job.h"

#include <UnitTest++.h>

#include <cstdio>
#include <iostream>

using namespace aufw::job;

struct DummyException {};

//
// Execution failure
//

class Step1ExecutionFailure : public Step {
public:
    bool HasExecuted;
    bool HasRolledBack;

    Step1ExecutionFailure() : HasExecuted(false), HasRolledBack(false) {}

    virtual void Execute() {
        HasExecuted = true;
    }

    virtual void Rollback() {
        HasRolledBack = true;
    }
};

class Step2ExecutionFailure : public Step {
public:
    bool HasExecuted;
    bool HasRolledBack;

    Step2ExecutionFailure() : HasExecuted(false), HasRolledBack(false) {}

    virtual void Execute() {
        throw DummyException();
        HasExecuted = true;
    }

    virtual void Rollback() {
        HasRolledBack = true;
    }
};

int g_executeTryCount = 1;

void testExecutionFailure() {
    Job job;
    job.OnExecutionFailed = [](JobFailureArg& arg) {
        // Test retries
        arg.ShouldRetry = (g_executeTryCount <= 1);
        if (arg.ShouldRetry) {
            ++g_executeTryCount;
        }
    };

    job.OnRollbackFailed = [](JobFailureArg& arg) {
        arg.ShouldRetry = false;
    };

    auto step1 = new Step1ExecutionFailure;
    job.AddStep(step1);
    auto step2 = new Step2ExecutionFailure;
    job.AddStep(step2);

    job.Execute();
    CHECK_EQUAL(2, g_executeTryCount);
    CHECK(step1->HasExecuted && !step2->HasExecuted);
    CHECK(step1->HasRolledBack && !step2->HasRolledBack);
}

//
// Rollback failure
//

class Step1RollbackFailure : public Step {
public:
    bool HasExecuted;
    bool HasRolledBack;

    Step1RollbackFailure() : HasExecuted(false), HasRolledBack(false) {}

    virtual void Execute() {
        HasExecuted = true;
    }

    virtual void Rollback() {
        throw DummyException();
        HasRolledBack = true;
    }
};

class Step2RollbackFailure : public Step {
public:
    bool HasExecuted;
    bool HasRolledBack;

    Step2RollbackFailure() : HasExecuted(false), HasRolledBack(false) {}

    virtual void Execute() {
        throw DummyException();
        HasExecuted = true;
    }

    virtual void Rollback() {
        HasRolledBack = true;
    }
};

int g_rollbackTryCount = 1;

void testRollbackFailure() {
    Job job;
    job.OnExecutionFailed = [](JobFailureArg& arg) {
        arg.ShouldRetry = false;
    };

    job.OnRollbackFailed = [](JobFailureArg& arg) {
        // Test retries
        arg.ShouldRetry = (g_rollbackTryCount <= 1);
        if (arg.ShouldRetry) {
            ++g_rollbackTryCount;
        }
    };

    auto step1 = new Step1RollbackFailure;
    job.AddStep(step1);
    auto step2 = new Step2RollbackFailure;
    job.AddStep(step2);

    job.Execute();
    CHECK_EQUAL(2, g_rollbackTryCount);
    CHECK(step1->HasExecuted && !step2->HasExecuted);
    CHECK(!step1->HasRolledBack && !step2->HasRolledBack);
}

TEST(Job) {
    testExecutionFailure();
    testRollbackFailure();
}
