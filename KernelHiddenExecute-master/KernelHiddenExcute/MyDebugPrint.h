#pragma once

#include "Head.h"



#define _DBG_PRINT
#define PRINT_NAME "XYLab"


#define _Title "["##PRINT_NAME##"] "
#define _Func "["##__FUNCTION__##"] "
#define _TitleAndFunc "["##PRINT_NAME##"] "##"["##__FUNCTION__##"] " 

#ifdef _DBG_PRINT
//#define MyPrint(_x_) DbgPrint _x_
#define MyPrint(...) DbgPrint(__VA_ARGS__)
#else
//#define MyPrint(_x_)
#define MyPrint(...)
#endif
