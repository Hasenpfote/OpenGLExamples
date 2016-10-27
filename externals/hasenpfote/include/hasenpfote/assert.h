/*!
* @file assert.h
* @brief Defines the assert macro and related functionality.
* @author Hasenpfote
* @date 2016/06/22
*/
#pragma once
#include <iostream>
#include <sstream>

#undef HASENPFOTE_ASSERT_MSG
#undef HASENPFOTE_ASSERT

#ifdef NDEBUG
// マクロ内のトリックは以下を達成するためのものである.
// (1) C4101 を除去する
// (2) message に stream 命令も併用できるようにする
// (3) Release mode で一命令も生成させない(msvs2015 では確認済み)
#if 1
 #define HASENPFOTE_ASSERT_MSG(expression, message)\
    do{\
        static_cast<void>(true ? static_cast<void>(0) : static_cast<void>((expression)));\
        static_cast<void>(true ? static_cast<void>(0) : static_cast<void>((std::ostringstream() << message)));\
    }while(false)
#else
#define HASENPFOTE_ASSERT_MSG(expression, message)\
    do{\
        (void)(true ? (void)0 : ((void)(expression)));\
        (void)(true ? (void)0 : ((void)(std::ostringstream() << message)));\
    }while(false)
#endif
 #define HASENPFOTE_ASSERT(expression)\
    HASENPFOTE_ASSERT_MSG(expression, "")
#else
 #define HASENPFOTE_ASSERT_MSG(expression, message)\
    do{\
        if(!(expression)){\
            std::cerr << "Assertion `" #expression "` failed in " << __func__ << "() " << __FILE__ << "(" << __LINE__ << "): " << message << std::endl;\
            std::terminate();\
        }\
    }while(false)
 #define HASENPFOTE_ASSERT(expression)\
    HASENPFOTE_ASSERT_MSG(expression, "")
#endif