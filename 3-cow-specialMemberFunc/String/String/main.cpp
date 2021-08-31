//
//  main.cpp
//  String
//
//  Created by 最上川 on 2021/8/29.
//

#include <iostream>
#include <stdio.h>
#include "String.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    String s = "abc";
    {
        String a(s);
        printf("%s\n", a.data());
    }
    printf("%s", s.c_str());
    return 0;
}
