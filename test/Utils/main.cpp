#include <Arduino.h>
#include <unity.h>

#include "RMath.h"

void Vector_Default()
{
    Vec3 vector;

    TEST_ASSERT_EQUAL(0.0f, vector.x);
    TEST_ASSERT_EQUAL(0.0f, vector.y);
    TEST_ASSERT_EQUAL(0.0f, vector.z);
}

void Vector_Assign()
{
    Vec3 vector;
    vector.x = 1.0f;
    vector.y = 2.0f;
    vector.z = 3.0f;

    TEST_ASSERT_EQUAL(1.0f, vector.x);
    TEST_ASSERT_EQUAL(2.0f, vector.y);
    TEST_ASSERT_EQUAL(3.0f, vector.z);
    
}

void Vector_Length()
{
    Vec3 vector;
    vector.x = 50.0f;
    TEST_ASSERT_EQUAL(50.0f, Length(vector));
}

void setup()
{
    UNITY_BEGIN();
    {
        delay(500);
        RUN_TEST(Vector_Default);

        delay(500);
        RUN_TEST(Vector_Assign);

        delay(500);
        RUN_TEST(Vector_Length);
    }
    UNITY_END();
}

void loop() { }