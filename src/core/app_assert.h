#pragma once

#include <filesystem>

#define APP_DEBUGBREAK() __debugbreak()
#define APP_EXPAND_MACRO(x) x
#define APP_STRINGIFY_MACRO(x) #x

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define APP_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { APP##type##ERROR(msg, __VA_ARGS__); APP_DEBUGBREAK(); } }
#define APP_INTERNAL_ASSERT_WITH_MSG(type, check, ...) APP_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define APP_INTERNAL_ASSERT_NO_MSG(type, check) APP_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", APP_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define APP_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define APP_INTERNAL_ASSERT_GET_MACRO(...) APP_EXPAND_MACRO( APP_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, APP_INTERNAL_ASSERT_WITH_MSG, APP_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define APP_ASSERT(...) APP_EXPAND_MACRO( APP_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )

