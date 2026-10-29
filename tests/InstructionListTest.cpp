#include "InstructionListTest.hpp"
#include <vector>


void InstructionListTest::SetUp()
{
    m_globalMemory = projectm_eval_memory_buffer_create();
    m_context = projectm_eval_context_create(m_globalMemory, &m_globalRegisters);
}

void InstructionListTest::TearDown()
{
    projectm_eval_context_destroy(m_context);
    projectm_eval_memory_buffer_destroy(m_globalMemory);
    memset(&m_globalRegisters, 0, sizeof(m_globalRegisters));
}

TEST_F(InstructionListTest, TopLevelList)
{
    PRJM_EVAL_F* varX = projectm_eval_context_register_variable(m_context, "x");
    PRJM_EVAL_F* varY = projectm_eval_context_register_variable(m_context, "y");
    auto code = projectm_eval_code_compile(m_context, "x = 1; 2; 3; y = 4; sin(4); gmem[100] = 300; 5");
    auto result = projectm_eval_code_execute(code);
    projectm_eval_code_destroy(code);

    ASSERT_FLOAT_EQ(result, 5.0);
    ASSERT_FLOAT_EQ(*varX, 1.0);
    ASSERT_FLOAT_EQ(*varY, 4.0);
}

TEST_F(InstructionListTest, WhileList)
{
    PRJM_EVAL_F* varX = projectm_eval_context_register_variable(m_context, "x");
    PRJM_EVAL_F* varY = projectm_eval_context_register_variable(m_context, "y");
    auto code = projectm_eval_code_compile(m_context, "x = 1; 2; while(y = 4; sin(5); 0); gmem[100] = 300; 5");
    auto result = projectm_eval_code_execute(code);
    projectm_eval_code_destroy(code);

    ASSERT_FLOAT_EQ(result, 5.0);
    ASSERT_FLOAT_EQ(*varX, 1.0);
    ASSERT_FLOAT_EQ(*varY, 4.0);
}

TEST_F(InstructionListTest, ParenthesedList)
{
    PRJM_EVAL_F* varX = projectm_eval_context_register_variable(m_context, "x");
    PRJM_EVAL_F* varY = projectm_eval_context_register_variable(m_context, "y");
    PRJM_EVAL_F* varZ = projectm_eval_context_register_variable(m_context, "z");
    auto code = projectm_eval_code_compile(m_context, "x = 1; 2; y = (z = 6; sin(5); 4); gmem[100] = 300; 5");
    auto result = projectm_eval_code_execute(code);
    projectm_eval_code_destroy(code);

    ASSERT_FLOAT_EQ(result, 5.0);
    ASSERT_FLOAT_EQ(*varX, 1.0);
    ASSERT_FLOAT_EQ(*varY, 4.0);
    ASSERT_FLOAT_EQ(*varZ, 6.0);
}

TEST_F(InstructionListTest, RandStreamIsPerContext)
{
    auto draw = [](projectm_eval_context* context, int count) {
        auto* code = projectm_eval_code_compile(context, "rand(1000000)");
        std::vector<PRJM_EVAL_F> values;
        for (int i = 0; i < count; i++)
        {
            values.push_back(projectm_eval_code_execute(code));
        }
        projectm_eval_code_destroy(code);
        return values;
    };

    auto* first = projectm_eval_context_create(nullptr, nullptr);
    auto* second = projectm_eval_context_create(nullptr, nullptr);

    // Unseeded contexts each start Milkdrop's default stream; drawing from one doesn't move the other.
    auto firstValues = draw(first, 700);
    EXPECT_EQ(draw(second, 700), firstValues);

    projectm_eval_context_seed_rand(first, 1234);
    projectm_eval_context_seed_rand(second, 1234);
    firstValues = draw(first, 10);
    EXPECT_EQ(draw(second, 10), firstValues);

    projectm_eval_context_seed_rand(second, 5678);
    EXPECT_NE(draw(second, 10), draw(first, 10));

    projectm_eval_context_destroy(first);
    projectm_eval_context_destroy(second);
}
