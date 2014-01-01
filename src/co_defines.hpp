
#pragma once


// TIPS:リリース時 std::cout 一網打尽マクロ
#ifdef DEBUG
#define DOUT std::cout
#else
#define DOUT 0 && std::cout
#endif

// TIPS:リリース時 NSLog 一網打尽マクロ
#ifdef DEBUG
#define NSLOG(...) NSLog(__VA_ARGS__)
#else
#define NSLOG(...) 
#endif

typedef unsigned char  u_char;
typedef unsigned int   u_int;
typedef unsigned long  u_long;
